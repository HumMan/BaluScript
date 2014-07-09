#include "SExpression.h"

#include "../Syntax/Expression.h"
#include "FormalParam.h"
#include "../semanticAnalyzer.h"
#include "SStatements.h"
#include "FormalParam.h"
#include "SLocalVar.h"

class TSemanticTreeBuilder :public TExpressionTreeVisitor
{
	TSExpression::TOperation* return_new_operation;
	TExpression* syntax_node;
	TSClass* owner;
	TSMethod* method;
	TSExpression* parent;
public:
	TSExpression::TOperation* GetResult()
	{
		return return_new_operation;
	}
	TSemanticTreeBuilder(TExpression* syntax_node, TSClass* owner, TSMethod* method, TSExpression* parent)
	{
		this->syntax_node = syntax_node;
		this->owner = owner;
		this->method = method;
		this->parent = parent;
	}
	void Visit(TExpression::TBinOp* operation_node)
	{
		std::vector<TSExpression::TOperation*> param_expressions;

		TSExpression::TOperation *left,*right;
		operation_node->left->Accept(this);
		left = return_new_operation;
		operation_node->right->Accept(this);
		right = return_new_operation;

		param_expressions.push_back(left);
		param_expressions.push_back(right);

		int left_conv_count = -1, right_conv_count = -1;

		TSMethod *bin_operator = NULL, *bin_operator2 = NULL;

		std::vector<TFormalParam> param;
		std::vector<TMethod*> bin_operators;

		param.resize(2);
		param[0] = left->GetFormalParameter();
		param[1] = right->GetFormalParameter();

		if (param[0].IsVoid())
			syntax_node->Error("К левому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");
		if (param[1].IsVoid())
			syntax_node->Error("К правому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");

		std::vector<TSMethod*> operators;

		param[0].GetClass()->GetOperators(operators, operation_node->op);
		bin_operator = FindMethod(syntax_node, operators, param, left_conv_count);
		if (operation_node->op >= TOperator::Assign&&operation_node->op <= TOperator::OrA && !param[0].IsRef())
			syntax_node->Error("Для присваиваниия требуется ссылка, а не значение!");

		if (!(operation_node->op >= TOperator::Assign&&operation_node->op <= TOperator::OrA))
		{
			operators.resize(0);
			param[1].GetClass()->GetOperators(operators, operation_node->op);
			bin_operator2 = FindMethod(syntax_node, operators, param, right_conv_count);
		}

		TSMethod* invoke=NULL;
		if (bin_operator != NULL && (bin_operator2 == NULL || left_conv_count<right_conv_count))
		{
			ValidateAccess(syntax_node, owner, bin_operator);
			invoke = bin_operator;
		}
		else if (bin_operator2 != NULL)
		{
			ValidateAccess(syntax_node, owner, bin_operator2);
			invoke = bin_operator2;
		}
		else syntax_node->Error("Бинарного оператора для данных типов не существует!");

		TSExpression::TMethodCall* method_call = new TSExpression::TMethodCall();
		method_call->Build(param_expressions,param, invoke);
		return_new_operation = method_call;
	}
	void Visit(TExpression::TUnaryOp* operation_node)
	{
		std::vector<TSExpression::TOperation*> param_expressions;
		TSExpression::TOperation *left;
		operation_node->left->Accept(this);
		left = return_new_operation;

		param_expressions.push_back(left);

		std::vector<TFormalParam> param;

		param.resize(1);
		param[0] = left->GetFormalParameter();

		int conv_count;
		TSMethod *unary_operator = NULL;

		if (param[0].GetClass() == NULL)
			syntax_node->Error("К данному операнду нельзя применить унарный оператор (нужен тип отличающийся от void)!");

		std::vector<TSMethod*> operators;
		param[0].GetClass()->GetOperators(operators, operation_node->op);

		unary_operator = FindMethod(syntax_node, operators, param, conv_count);

		if (unary_operator != NULL)
		{
			ValidateAccess(syntax_node, owner, unary_operator);

			TSExpression::TMethodCall* method_call = new TSExpression::TMethodCall();
			method_call->Build(param_expressions,param, unary_operator);
			return_new_operation = method_call;
		}
		else syntax_node->Error("Унарного оператора для данного типа не существует!");
	}
	void Visit(TExpression::TCallParamsOp* operation_node)
	{
		std::vector<TSExpression::TOperation*> param_expressions;
		TSExpression::TOperation *left;
		operation_node->left->Accept(this);
		left = return_new_operation;

		TFormalParam left_result = left->GetFormalParameter();
		std::vector<TFormalParam> params_result;

		for (int i = 0; i < operation_node->param.size(); i++)
		{
			operation_node->param[i]->Accept(this);
			param_expressions.push_back(return_new_operation);
			params_result.push_back(return_new_operation->GetFormalParameter());
		}
		int conv_need = 0;
		TSMethod* invoke=NULL;
		if (left_result.IsMethods())
		{
			//вызов метода
			if (operation_node->is_bracket)
				assert(false);//при вызове метода используются круглые скобки
			TSMethod* method = FindMethod(syntax_node, left_result.GetMethods(), params_result, conv_need);
			if (method != NULL)
			{
				ValidateAccess(syntax_node, owner, method);
				invoke = method;
			}
			else 
				syntax_node->Error("Метода с такими параметрами не существует");
		}
		else if (left_result.IsType())
		{
			//if(left_result.GetType()->GetType()==TYPE_ENUM)
			//	Error("Для перечислений нельзя использовать оператор вызова параметров!");
			int conv_need = -1;
			std::vector<TSMethod*> constructors;
			TSClass* constr_class = left_result.GetType();
			constr_class->GetConstructors(constructors);
			TSMethod* constructor = FindMethod(syntax_node, constructors, params_result, conv_need);
			if (constructor != NULL)
			{
				ValidateAccess(syntax_node, owner, constructor);
				invoke = method;
			}
			else if (params_result.size()>0)
				syntax_node->Error("Конструктора с такими параметрами не существует!");
		}
		else
		{
			params_result.insert(params_result.begin(), left_result);
			std::vector<TSMethod*> operators;
			left_result.GetClass()->GetOperators(operators, operation_node->is_bracket ? (TOperator::GetArrayElement) : (TOperator::ParamsCall));
			TSMethod* method = FindMethod(syntax_node, operators, params_result, conv_need);
			if (method != NULL)
			{
				ValidateAccess(syntax_node, owner, method);
				invoke = method;
			}
			else
				syntax_node->Error("Оператора с такими параметрами не существует!");
		}
		
		TSExpression::TMethodCall* method_call = new TSExpression::TMethodCall();
		method_call->Build(param_expressions, params_result, invoke);
		return_new_operation = method_call;
	}
	
	void Visit(TExpression::TGetMemberOp* operation_node)
	{
		TSExpression::TOperation *left;
		operation_node->left->Accept(this);
		left = return_new_operation;

		TFormalParam left_result = left->GetFormalParameter();
		if (left_result.IsMethods())
			syntax_node->Error("Оператор доступа к члену класса нельзя применить к методу!");
		if (left_result.IsType())
		{
			//if (left_result.GetType()->IsEnum())
			//{
			//	int id = left_result.GetType()->GetEnumId(name);
			//	//TODO ввести спец функции min max count
			//	if (id == -1)
			//		Error("Перечислимого типа с таким именем не существует!");
			//	else
			//	{
			//		program.Push(TOp(TOpcode::PUSH, id), left_result.GetOps());
			//		return TFormalParam(left_result.GetType(), false, left_result.GetOps());
			//	}
			//}
			//else
			{
				TSClassField* static_member = left_result.GetType()->GetField(operation_node->name, true, true);
				if (static_member != NULL)
				{
					TSExpression::TGetClassField* result = new TSExpression::TGetClassField();
					result->left = left;
					result->field = (TSClassField*)static_member;
					return_new_operation = result;
					return;
				}
				else
				{
					std::vector<TSMethod*> methods;
					if (left_result.GetType()->GetMethods(methods, operation_node->name, true))
					{
						TSExpression::TGetMethods* result = new TSExpression::TGetMethods();
						result->left = left;
						result->left_result = left_result;
						result->result = TFormalParam(methods, method->GetSyntax()->IsStatic());
						return_new_operation = result;
						return;
					}
					else
					{
						TSClass* nested = left_result.GetType()->GetNested(operation_node->name);
						if (nested != NULL)
						{
							TSExpression::TGetClass* result = new TSExpression::TGetClass();
							result->left = (TSExpression::TGetClass*)left;
							assert(result->left != NULL);
							result->get_class = nested;
							return_new_operation = result;
							return;
						}else
							syntax_node->Error("Статического поля или метода с таким именем не существует!");
					}
				}
			}
		}
		else
		{
			TSClassField* member = left_result.GetClass() != NULL
				? left_result.GetClass()->GetField(operation_node->name, true)
				: NULL;
			if (member != NULL)
			{
				if (member->GetSyntax()->IsStatic())
					syntax_node->Error("Оператор доступа к члену класса нельзя применить к объекту для доступа к статическому члену, \nвместо объекта следует использовать имя класса!");
				ValidateAccess(syntax_node, owner, member);

				TSExpression::TGetClassField* result = new TSExpression::TGetClassField();
				result->left = left;
				result->left_result = left_result;
				result->field = member;
				return_new_operation = result;
				return;

				if (left_result.IsRef())
				{
					
					//if (member->IsReadOnly())
					{
						//TSMethod* copy_constr = result.GetClass()->GetCopyConstr();
						//program.Push(TOp(TOpcode::RVALUE, result.GetClass()->GetSize(), program.AddMethodToTable(copy_constr))
						//	, result.GetOps());
						//result.SetIsRef(false);
					}
				}
				else
				{
					//program.Push(TOp(TOpcode::PUSH, left_result.GetClass()->GetSize()), left_result.GetOps());
					//program.Push(TOp(TOpcode::PUSH, member->GetOffset()), left_result.GetOps());
					//program.Push(TOp(TOpcode::PUSH, member->GetClass()->GetSize()), left_result.GetOps());
					//program.Push(TOp(TOpcode::GET_MEMBER,
					//	program.AddMethodToTable(member->GetClass()->GetCopyConstr()),
					//	program.AddMethodToTable(member->GetClass()->GetDestructor())),
					//	left_result.GetOps());
					//return TFormalParam(member->GetClass(), false, left_result.GetOps());
				}
				//Error("Оператор доступа к члену объекта нельзя использовать для временного объекта!");//TODO
			}
			else
			{
				if (!left_result.IsRef())
					syntax_node->Error("Вызов метода для временного объекта недопустим!");
				std::vector<TSMethod*> methods;
				if (left_result.GetClass()->GetMethods(methods, operation_node->name, false))
				{
					TSExpression::TGetMethods* result = new TSExpression::TGetMethods();
					result->left = left;
					result->left_result = left_result;
					result->result = TFormalParam(methods, false);
					return_new_operation = result;
					return;
				}
				else
					syntax_node->Error("Члена класса с таким именем не существует!");
			}
		}
	}
	void Visit(TExpression::TId* operation_node)
	{
		TVariable* var = parent->GetVar(operation_node->name);
		if (var != NULL)
		{
			switch (var->GetType())
			{
			case TVariableType::ClassField:
			{
				ValidateAccess(syntax_node, owner, (TSClassField*)var);
				if (!((TClassField*)var)->IsStatic() && method->GetSyntax()->IsStatic())
					syntax_node->Error("К нестатическому полю класса нельзя обращаться из статического метода!");
				TSExpression::TGetClassField* result = new TSExpression::TGetClassField();
				result->left = NULL;
				result->field = (TSClassField*)var;
				return_new_operation = result;
				return;
			}break;
			case TVariableType::Local:
			{
				TSExpression::TGetLocal* result = new TSExpression::TGetLocal();
				result->variable = (TSLocalVar*)var;
				return_new_operation = result;
				return;
			}break;
			case TVariableType::Parameter:
			{
				TSExpression::TGetParameter* result = new TSExpression::TGetParameter();
				result->parameter = (TSParameter*)var;
				return_new_operation = result;
				return;
			}break;
			default:
				assert(false);//ошибка в поиске переменной
			}
		}
		else
		{
			std::vector<TSMethod*> methods;
			if (method->GetSyntax()->IsStatic())
			{
				owner->GetMethods(methods, operation_node->name, true);
				if (methods.size() == 0)
				{
					std::vector<TSMethod*> temp;
					if (owner->GetMethods(temp, operation_node->name, false))
					{
						syntax_node->Error("К нестатическому методу класса нельзя обращаться из статического метода!");
					}
				}
			}
			else
			{
				owner->GetMethods(methods, operation_node->name);
			}
			if (methods.size() != 0)
			{
				TSExpression::TGetMethods* result = new TSExpression::TGetMethods();
				result->left = NULL;
				result->result = TFormalParam(methods, method->GetSyntax()->IsStatic());
				return_new_operation = result;
				return;
			}
			else
			{
				TSClass* type = owner->GetClass(operation_node->name);
				if (type != NULL)
				{
					TSExpression::TGetClass* result = new TSExpression::TGetClass();
					result->left = NULL;
					result->get_class = type;
					return_new_operation = result;
					return;
				}
				else
					syntax_node->Error("Неизвестный идентификатор!");
			}
		}
		assert(false);//сюда мы не должны попасть
	}
	void Visit(TExpression::TThis *operation_node)
	{
		if (method->GetSyntax()->IsStatic())
			syntax_node->Error("Ключевое слово 'this' можно использовать только в нестатических методах!");
		TSExpression::TGetThis* result = new TSExpression::TGetThis();
		result->owner = owner;
		return_new_operation = result;
	}
	void Visit(TExpression::TIntValue *op)
	{
		TSExpression::TInt* result = new TSExpression::TInt(owner, &op->type);
		result->val = op->val;
		result->type.Link();
		return_new_operation = result;
	}
	void Visit(TExpression::TStringValue *op)
	{

	}
	void Visit(TExpression::TCharValue* op)
	{

	}
	void Visit(TExpression::TFloatValue* op)
	{
		TSExpression::TFloat* result = new TSExpression::TFloat(owner, &op->type);
		result->val = op->val;
		result->type.Link();
		return_new_operation = result;
	}
	void Visit(TExpression::TBoolValue* op)
	{

	}
};

void TSExpression::Build()
{
	TSemanticTreeBuilder b(GetSyntax(),owner,method,this);
	GetSyntax()->Accept(&b);
	first_op = b.GetResult();
}

TVariable* TSExpression::GetVar(TNameId name)
{
	return parent->GetVar(name, GetSyntax()->stmt_id);
}

void TSExpression::TMethodCall::Build(const std::vector<TSExpression::TOperation*>& param_expressions, const std::vector<TFormalParam>& params, TSMethod* method)
{
	assert(param_expressions.size() == params.size());
	invoke = method;
	for (int i = 0; i < params.size(); i++)
	{
		input.emplace_back();
		input.back().BuildConvert(params[i], method->GetParam(i)->GetClass(), method->GetParam(i)->GetSyntax()->IsRef());
	}
}

TSExpression::TInt::TInt(TSClass* owner, TType* syntax_node)
	:type(owner,syntax_node)
{
}
void TSExpression::TInt::Run(int* &sp)
{
	sp++;
	*sp = val;
}
TSExpression::TFloat::TFloat(TSClass* owner, TType* syntax_node)
	:type(owner, syntax_node)
{
}
void TSExpression::TFloat::Run(int* &sp)
{
	sp++;
	*(float*)sp = val;
}
TFormalParam TSExpression::TFloat::GetFormalParameter()
{
	return TFormalParam(type.GetClass(), false);
}
TFormalParam TSExpression::TMethodCall::GetFormalParameter()
{
	return TFormalParam(invoke->GetRetClass(), invoke->GetSyntax()->IsReturnRef());
}
void TSExpression::TMethodCall::Run(int* &sp)
{
	sp += invoke->GetRuturnSize();
	for (TSExpression::TMethodCall::TOperationWithConversions& par : input)
	{
		par.expression->Run(sp);
		par.conversions.RunConversion(sp);
	}
	invoke->Run(sp);
}
TFormalParam TSExpression::TInt::GetFormalParameter()
{
	return TFormalParam(type.GetClass(),false);
}
TFormalParam TSExpression::TGetMethods::GetFormalParameter()
{
	return result;
}
void TSExpression::TGetMethods::Run(int* &sp)
{
	//nothing to do
}
TFormalParam TSExpression::TGetClassField::GetFormalParameter()
{
	return TFormalParam(field->GetClass(),left_result.IsRef());
}
void TSExpression::TGetClassField::Run(int* &sp)
{

}
TFormalParam TSExpression::TGetParameter::GetFormalParameter()
{
	return TFormalParam(parameter->GetClass(), true);
}
void TSExpression::TGetParameter::Run(int* &sp)
{
	//sp_base - parameter offset
}
TFormalParam TSExpression::TGetLocal::GetFormalParameter()
{
	return TFormalParam(variable->GetClass(), true);
}
void TSExpression::TGetLocal::Run(int* &sp)
{

}
TFormalParam TSExpression::TGetClass::GetFormalParameter()
{
	return TFormalParam(get_class);
}
void TSExpression::TGetClass::Run(int* &sp)
{
	//nothing to do
}
TFormalParam TSExpression::TGetThis::GetFormalParameter()
{
	return TFormalParam(owner, true);
}
void TSExpression::TGetThis::Run(int* &sp)
{

}

void TSExpression::Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value, int method_base)
{
	first_op->Run(sp);
}