#include "SExpression.h"

#include "../Syntax/Expression.h"
#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"

#include "../semanticAnalyzer.h"

#include "FormalParam.h"
#include "SStatements.h"
#include "FormalParam.h"
#include "SLocalVar.h"
#include "SClass.h"
#include "SConstructObject.h"


class TSemanticTreeBuilder :public TExpressionTreeVisitor
{
	TOperation* return_new_operation;
	TExpression* syntax_node;
	TSClass* owner;
	TSMethod* method;
	TSExpression* parent;
	std::vector<TSClassField*>* static_fields;
	std::vector<TSLocalVar*>* static_variables;
public:
	TOperation* GetResult()
	{
		return return_new_operation;
	}
	TSemanticTreeBuilder(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables, TExpression* syntax_node, TSClass* owner, TSMethod* method, TSExpression* parent)
	{
		this->syntax_node = syntax_node;
		this->owner = owner;
		this->method = method;
		this->parent = parent;
		this->static_fields = static_fields;
	}
	void Visit(TExpression::TBinOp* operation_node)
	{
		std::vector<TOperation*> param_expressions;

		TOperation *left,*right;
		operation_node->left->Accept(this);
		left = return_new_operation;
		operation_node->right->Accept(this);
		right = return_new_operation;

		param_expressions.push_back(left);
		param_expressions.push_back(right);

		int left_conv_count = -1, right_conv_count = -1;

		TSMethod *bin_operator = NULL, *bin_operator2 = NULL;

		std::vector<TExpressionResult> param;
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
		bool memcpy_assign = false;
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
		else if (operation_node->op == TOperator::Assign)
		{
			memcpy_assign = true;
		}
		else syntax_node->Error("Бинарного оператора для данных типов не существует!");

		TSExpression_TMethodCall* method_call = new TSExpression_TMethodCall();
		method_call->memcpy_assign = memcpy_assign;
		if (memcpy_assign)
			method_call->Build(param_expressions);
		else
			method_call->Build(param_expressions, invoke);
		return_new_operation = method_call;
	}
	void Visit(TExpression::TUnaryOp* operation_node)
	{
		std::vector<TOperation*> param_expressions;
		TOperation *left;
		operation_node->left->Accept(this);
		left = return_new_operation;

		param_expressions.push_back(left);

		std::vector<TExpressionResult> param;

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

			TSExpression_TMethodCall* method_call = new TSExpression_TMethodCall();
			method_call->Build(param_expressions, unary_operator);
			return_new_operation = method_call;
		}
		else syntax_node->Error("Унарного оператора для данного типа не существует!");
	}
	void Visit(TExpression::TCallParamsOp* operation_node)
	{
		std::vector<TOperation*> param_expressions;
		TOperation *left;
		operation_node->left->Accept(this);
		left = return_new_operation;

		TExpressionResult left_result = left->GetFormalParameter();
		std::vector<TExpressionResult> params_result;

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

			

			//TODO
			//constr_class->GetConstructors(constructors);
			//TSMethod* constructor = FindMethod(syntax_node, constructors, params_result, conv_need);
			//if (constructor != NULL)
			//{
			//	ValidateAccess(syntax_node, owner, constructor);
			//	invoke = constructor;
			//}
			//else if (params_result.size()>0)
			//	syntax_node->Error("Конструктора с такими параметрами не существует!");
		}
		else
		{
			//т.к. все операторы статические - первым параметром передаем ссылку на объект
			param_expressions.insert(param_expressions.begin(), left);
			params_result.insert(params_result.begin(), left_result);
			
			left = NULL;

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
		
		TSExpression_TMethodCall* method_call = new TSExpression_TMethodCall();
		method_call->Build(param_expressions, invoke);
		method_call->left.reset(left);
		//method_call->construct_temp_object->Build()
		return_new_operation = method_call;
	}
	
	void Visit(TExpression::TGetMemberOp* operation_node)
	{
		TOperation *left;
		operation_node->left->Accept(this);
		left = return_new_operation;

		TExpressionResult left_result = left->GetFormalParameter();
		if (left_result.IsMethods())
			syntax_node->Error("Оператор доступа к члену класса нельзя применить к методу!");
		if (left_result.IsType())
		{
			if (left_result.GetType()->GetSyntax()->IsEnumeration())
			{
				int id = left_result.GetType()->GetSyntax()->GetEnumId(operation_node->name);
				//TODO ввести спец функции min max count
				if (id == -1)
					syntax_node->Error("Перечислимого типа с таким именем не существует!");
				else
				{
					TSExpression::TEnumValue* result = new TSExpression::TEnumValue(owner, left_result.GetType());
					result->val = id;
					return_new_operation = result;
				}
			}
			else
			{
				TSClassField* static_member = left_result.GetType()->GetField(operation_node->name, true, true);
				if (static_member != NULL)
				{
					TSExpression::TGetClassField* result = new TSExpression::TGetClassField();
					result->left.reset(left);
					result->left_result = left_result;
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
						result->left.reset(left);
						result->left_result = left_result;
						result->result = TExpressionResult(methods, method->GetSyntax()->IsStatic());
						return_new_operation = result;
						return;
					}
					else
					{
						TSClass* nested = left_result.GetType()->GetNested(operation_node->name);
						if (nested != NULL)
						{
							TSExpression::TGetClass* result = new TSExpression::TGetClass();
							result->left.reset((TSExpression::TGetClass*)left);
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
				result->left.reset(left);
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
					//return TExpressionResult(member->GetClass(), false, left_result.GetOps());
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
					result->left.reset(left);
					result->left_result = left_result;
					result->result = TExpressionResult(methods, false);
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
				if ((!(static_cast<TSClassField*>(var))->GetSyntax()->IsStatic()) && method->GetSyntax()->IsStatic())
					syntax_node->Error("К нестатическому полю класса нельзя обращаться из статического метода!");
				TSExpression::TGetClassField* result = new TSExpression::TGetClassField();
				result->left = NULL;
				result->field = (TSClassField*)var;
				result->left_result = TExpressionResult(result->field->GetClass(), true);
				return_new_operation = result;
				return;
			}break;
			case TVariableType::Local:
			{
				TSExpression::TGetLocal* result = new TSExpression::TGetLocal();
				result->variable=(TSLocalVar*)var;
				return_new_operation = result;
				return;
			}break;
			case TVariableType::Parameter:
			{
				TSExpression::TGetParameter* result = new TSExpression::TGetParameter();
				result->parameter.reset((TSParameter*)var);
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
				result->result = TExpressionResult(methods, method->GetSyntax()->IsStatic());
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
		result->type.LinkSignature(static_fields, static_variables);
		result->type.LinkBody(static_fields, static_variables);
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
		result->type.LinkSignature(static_fields, static_variables);
		result->type.LinkBody(static_fields, static_variables);
		return_new_operation = result;
	}
	void Visit(TExpression::TBoolValue* op)
	{
		TSExpression::TBool* result = new TSExpression::TBool(owner, &op->type);
		result->val = op->val;
		result->type.LinkSignature(static_fields, static_variables);
		result->type.LinkBody(static_fields, static_variables);
		return_new_operation = result;
	}
};

void TSExpression::Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	TSemanticTreeBuilder b(static_fields, static_variables, GetSyntax(),owner,method,this);
	GetSyntax()->Accept(&b);
	first_op.reset(b.GetResult());
}

TVariable* TSExpression::GetVar(TNameId name)
{
	return parent->GetVar(name, GetSyntax()->stmt_id);
}

void TSExpression_TMethodCall::Build(const std::vector<TOperation*>& param_expressions, TSMethod* method)
{
	assert(param_expressions.size() == method->GetParamsCount());
	invoke = method;
	std::vector<TFormalParameter> formal_params;
	for (int i = 0; i < param_expressions.size(); i++)
	{
		formal_params.emplace_back(method->GetParam(i)->GetClass(), method->GetParam(i)->IsRef());
		//input.emplace_back(param_expressions[i]);
		//input.back().BuildConvert(params[i], method->GetParam(i)->GetClass(), method->GetParam(i)->IsRef());
	}

	params.Build(param_expressions, formal_params);
}

void TSExpression_TMethodCall::Build(const std::vector<TOperation*>& param_expressions)
{
	invoke = NULL;
	std::vector<TFormalParameter> formal_params;
	for (int i = 0; i < param_expressions.size(); i++)
	{
		TExpressionResult res = param_expressions[i]->GetFormalParameter();
		formal_params.emplace_back(TFormalParameter(res.GetClass(),res.IsRef()));
		//input.emplace_back(param_expressions[i]);
		//input.back().BuildConvert(params[i], method->GetParam(i)->GetClass(), method->GetParam(i)->IsRef());
	}

	params.Build(param_expressions, formal_params);
}

TSExpression::TInt::TInt(TSClass* owner, TType* syntax_node)
	:type(owner,syntax_node)
{
}
void TSExpression::TInt::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	result = TStackValue(false, type.GetClass());
	*(int*)result.get() = val;
}
TSExpression::TFloat::TFloat(TSClass* owner, TType* syntax_node)
	:type(owner, syntax_node)
{
}
void TSExpression::TFloat::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	result = TStackValue(false, type.GetClass());
	*(float*)result.get() = val;
}
TExpressionResult TSExpression::TFloat::GetFormalParameter()
{
	return TExpressionResult(type.GetClass(), false);
}

TSExpression::TBool::TBool(TSClass* owner, TType* syntax_node)
	:type(owner, syntax_node)
{
}
void TSExpression::TBool::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	result = TStackValue(false, type.GetClass());
	*(int*)result.get() = val;
}
TExpressionResult TSExpression::TBool::GetFormalParameter()
{
	return TExpressionResult(type.GetClass(), false);
}

TSExpression::TEnumValue::TEnumValue(TSClass* owner, TSClass* _type)
	:type(_type)
{
}
void TSExpression::TEnumValue::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	result = TStackValue(false, type);
	*(int*)result.get() = val;
}
TExpressionResult TSExpression::TEnumValue::GetFormalParameter()
{
	return TExpressionResult(type, false);
}

TExpressionResult TSExpression_TMethodCall::GetFormalParameter()
{
	return TExpressionResult(invoke->GetRetClass(), invoke->GetSyntax()->IsReturnRef());
}
void TSExpression_TMethodCall::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	//соглашение о вызовах - вызывающий инициализирует параметры, резервирует место для возвращаемого значения, вызывает деструкторы параметров
	TStackValue temp_result;
	std::vector<TStackValue> method_call_formal_params;
	params.Construct(method_call_formal_params,static_fields, formal_params, object, local_variables);
	
	if (memcpy_assign)
	{
		memcpy(method_call_formal_params[0].get(), method_call_formal_params[1].get(), method_call_formal_params[0].GetClass()->GetSize()*sizeof(int));
	}
	else
	{
		TStackValue left_result;
		if (left != NULL)
			left->Run(static_fields, formal_params, left_result, object, local_variables);

		if (invoke->GetRetClass() != NULL)
			result = TStackValue(invoke->GetSyntax()->IsReturnRef(), invoke->GetRetClass());

		invoke->Run(static_fields, method_call_formal_params, result, left_result);
	}
	params.Destroy(method_call_formal_params, static_fields, formal_params, object, local_variables);
}
TExpressionResult TSExpression::TInt::GetFormalParameter()
{
	return TExpressionResult(type.GetClass(),false);
}
TExpressionResult TSExpression::TGetMethods::GetFormalParameter()
{
	return result;
}
void TSExpression::TGetMethods::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	if (left!=NULL)
		left->Run(static_fields,formal_params, result, object, local_variables);
}
TExpressionResult TSExpression::TGetClassField::GetFormalParameter()
{
	//при получении статической переменной возвращается ссылка
	if (left_result.IsType())
		return TExpressionResult(field->GetClass(), true);
	else
		return TExpressionResult(field->GetClass(),left_result.IsRef());
}
void TSExpression::TGetClassField::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	if (field->GetSyntax()->IsStatic())
	{ 
		result = TStackValue(true, field->GetClass());
		result.SetAsReference(static_fields[field->GetOffset()].get());
	}
	else
	{
		if (left != NULL)
		{
			TStackValue exp_result;
			left->Run(static_fields, formal_params, exp_result, object, local_variables);
			result = TStackValue(true, field->GetClass());
			result.SetAsReference(((int*)exp_result.get()) + field->GetOffset());
		}
		else
		{
			result = TStackValue(true, field->GetClass());
			result.SetAsReference(((int*)object.get()) + field->GetOffset());
		}
	}
}
TExpressionResult TSExpression::TGetParameter::GetFormalParameter()
{
	return TExpressionResult(parameter->GetClass(), true);
}
void TSExpression::TGetParameter::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	//sp_base - parameter offset
	void* param = formal_params[parameter->GetOffset()].get();
	result = TStackValue(true, parameter->GetClass());
	result.SetAsReference(param);
}
TExpressionResult TSExpression::TGetLocal::GetFormalParameter()
{
	return TExpressionResult(variable->GetClass(), true);
}
void TSExpression::TGetLocal::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	void* variable_value = NULL;
	if (variable->IsStatic())
	{
		variable_value = static_fields[variable->GetOffset()].get();
	}
	else
	{
		variable_value=local_variables[variable->GetOffset()].get();
	}
	result = TStackValue(true, variable->GetClass());
	result.SetAsReference(variable_value);
}
TExpressionResult TSExpression::TGetClass::GetFormalParameter()
{
	return TExpressionResult(get_class);
}
void TSExpression::TGetClass::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	//nothing to do
}
TExpressionResult TSExpression::TGetThis::GetFormalParameter()
{
	return TExpressionResult(owner, true);
}
void TSExpression::TGetThis::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	result = TStackValue(true, owner);
	result.SetAsReference(object.get());
}

void TSExpression::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	Run(static_fields, formal_params, result, object, local_variables);
}

void TSExpression::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	TStackValue exp_result;
	first_op->Run(static_fields, formal_params, exp_result, object, local_variables);
	result = exp_result;
}