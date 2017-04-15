﻿#include "SExpression.h"

#include "../Syntax/Expression.h"
#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"
#include "../Syntax/ClassField.h"

#include "../semanticAnalyzer.h"

#include "FormalParam.h"
#include "SStatements.h"
#include "SLocalVar.h"
#include "SClass.h"
#include "SClassField.h"
#include "SConstructObject.h"

#include "../NativeTypes/String.h"

class TSemanticTreeBuilder :public SyntaxApi::IExpressionTreeVisitor
{
	//общие для всех выражений поля
	SyntaxApi::IExpression* syntax_node;
	TSClass* owner;
	TSMethod* method;
	TSExpression* parent;
	TGlobalBuildContext build_context;

	TSOperation* visit_result;
public:
	TSemanticTreeBuilder(TGlobalBuildContext build_context, SyntaxApi::IExpression* syntax_node, TSClass* owner, TSMethod* method, TSExpression* parent)
	{
		this->syntax_node = syntax_node;
		this->owner = owner;
		this->method = method;
		this->parent = parent;
		this->build_context = build_context;
	}

	TSOperation* VisitNode(SyntaxApi::IOperations::IOperation* op)
	{
		assert(this->visit_result == nullptr);
		op->Accept(this);
		assert(this->visit_result != nullptr);
		auto result = this->visit_result;
		this->visit_result = nullptr;
		return result;
	}

	TSOperation* VisitNode(const std::unique_ptr<SyntaxApi::IOperations::IOperation>& op)
	{
		assert(this->visit_result == nullptr);
		op->Accept(this);
		assert(this->visit_result != nullptr);
		auto result = this->visit_result;
		this->visit_result = nullptr;
		return result;
	}

	void Return(TSOperation* visit_result)
	{
		assert(this->visit_result == nullptr);
		this->visit_result = visit_result;
	}

	void Visit(SyntaxApi::IOperations::IBinOp* operation_node)
	{
		std::vector<TSOperation*> param_expressions;

		TSOperation *left,*right;
		left = VisitNode(operation_node->GetLeft());
		right = VisitNode(operation_node->GetRight());

		param_expressions.push_back(left);
		param_expressions.push_back(right);

		TSMethod *bin_operator = NULL;

		std::vector<TExpressionResult> param;
		std::vector<SyntaxApi::IMethod*> bin_operators;

		param.resize(2);
		param[0] = left->GetFormalParameter();
		param[1] = right->GetFormalParameter();

		if (param[0].IsVoid())
			syntax_node->Error("К левому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");
		if (param[1].IsVoid())
			syntax_node->Error("К правому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");

		std::vector<TSMethod*> operators;

		//список доступных операторов получаем из левого операнда
		param[0].GetClass()->GetOperators(operators, operation_node->GetOp());
		bin_operator = FindMethod(syntax_node, operators, param);
		if (operation_node->GetOp() >= Lexer::TOperator::Assign&&operation_node->GetOp() <= Lexer::TOperator::OrA && !param[0].IsRef())
			syntax_node->Error("Для присваиваниия требуется ссылка, а не значение!");

		if (bin_operator != NULL)
		{
			ValidateAccess(syntax_node, owner, bin_operator);
		}
		else syntax_node->Error("Бинарного оператора для данных типов не существует!");

		TSExpression_TMethodCall* method_call = NULL;

		method_call = new TSExpression_TMethodCall(TSExpression_TMethodCall::Operator);
		method_call->Build(param_expressions, bin_operator);

		Return(method_call);
	}
	void Visit(SyntaxApi::IOperations::IUnaryOp* operation_node)
	{
		std::vector<TSOperation*> param_expressions;
		TSOperation *left;
		left = VisitNode(operation_node->GetLeft());

		param_expressions.push_back(left);

		std::vector<TExpressionResult> param;

		param.resize(1);
		param[0] = left->GetFormalParameter();

		TSMethod *unary_operator = NULL;

		if (param[0].GetClass() == NULL)
			syntax_node->Error("К данному операнду нельзя применить унарный оператор (нужен тип отличающийся от void)!");

		std::vector<TSMethod*> operators;
		param[0].GetClass()->GetOperators(operators, operation_node->GetOp());

		unary_operator = FindMethod(syntax_node, operators, param);

		if (unary_operator != NULL)
		{
			ValidateAccess(syntax_node, owner, unary_operator);

			TSExpression_TMethodCall* method_call = new TSExpression_TMethodCall(TSExpression_TMethodCall::Operator);
			method_call->Build(param_expressions, unary_operator);
			Return(method_call);
		}
		else syntax_node->Error("Унарного оператора для данного типа не существует!");
	}
	void Visit(SyntaxApi::IOperations::IConstructTempObject* operation_node)
	{
		TSExpression_TempObjectType* result = new TSExpression_TempObjectType(owner, operation_node->GetType());
		result->type.LinkSignature(build_context);
		result->type.LinkBody(build_context);
		Return(result);
	}
	void Visit(SyntaxApi::IOperations::ICallParamsOp* operation_node)
	{
		TSOperation *left;
		left = VisitNode(operation_node->GetLeft());

		TExpressionResult left_result = left->GetFormalParameter();
		
		std::vector<TExpressionResult> params_result;
		std::vector<TSOperation*> param_expressions;
		std::vector<TFormalParameter> params_formals;
		auto param = operation_node->GetParam();
		for (size_t i = 0; i < param.size(); i++)
		{
			TSOperation* return_new_operation = VisitNode(param[i]);
			param_expressions.push_back(return_new_operation);
			params_result.push_back(return_new_operation->GetFormalParameter());
			params_formals.push_back(TFormalParameter(params_result.back().GetClass(), params_result.back().IsRef()));
		}
		
		if (left_result.IsMethods())
		{
			//вызов метода
			if (operation_node->IsBracket())
				assert(false);//при вызове метода используются круглые скобки
			TSMethod* invoke = NULL;
			TSMethod* method = FindMethod(syntax_node, left_result.GetMethods(), params_result);
			if (method != NULL)
			{
				ValidateAccess(syntax_node, owner, method);
				invoke = method;
			}
			else 
				syntax_node->Error("Метода с такими параметрами не существует");

			TSExpression_TMethodCall* method_call = new TSExpression_TMethodCall(TSExpression_TMethodCall::Method);
			method_call->Build(param_expressions, invoke);
			method_call->left.reset(left);
			//method_call->construct_temp_object->Build()
			Return(method_call);
		}
		else if (left_result.IsType())
		{
			//if(left_result.GetType()->GetType()==TYPE_ENUM)
			//	Error("Для перечислений нельзя использовать оператор вызова параметров!");
			int conv_need = -1;
			std::vector<TSMethod*> constructors;
			TSClass* constr_class = left_result.GetType();

			TSExpression_TCreateTempObject* create_temp_obj = new TSExpression_TCreateTempObject();
			create_temp_obj->construct_object.reset(new TSConstructObject(owner, method, parent->GetParentStatements(), constr_class));
			create_temp_obj->construct_object->Build(operation_node->GetOperationSource(), params_result, param_expressions, params_formals, build_context);
			create_temp_obj->left.reset((TSExpression_TempObjectType*)left);
			Return(create_temp_obj);
		}
		else
			//иначе вызов оператора () или []
		{
			//т.к. все операторы статические - первым параметром передаем ссылку на объект
			param_expressions.insert(param_expressions.begin(), left);
			params_result.insert(params_result.begin(), left_result);
			
			left = NULL;
			TSMethod* invoke = NULL;
			std::vector<TSMethod*> operators;
			left_result.GetClass()->GetOperators(operators, operation_node->IsBracket() ? (Lexer::TOperator::GetArrayElement) : (Lexer::TOperator::ParamsCall));
			TSMethod* method = FindMethod(syntax_node, operators, params_result);
			if (method != NULL)
			{
				ValidateAccess(syntax_node, owner, method);
				invoke = method;
			}
			else
				syntax_node->Error("Оператора с такими параметрами не существует!");

			TSExpression_TMethodCall* method_call = new TSExpression_TMethodCall(TSExpression_TMethodCall::Operator);
			method_call->Build(param_expressions, invoke);

			Return(method_call);
		}		
	}
	
	void Visit(SyntaxApi::IOperations::IGetMemberOp* operation_node)
	{
		TSOperation *left;
		left = VisitNode(operation_node->GetLeft());

		TExpressionResult left_result = left->GetFormalParameter();
		if (left_result.IsMethods())
			syntax_node->Error("Оператор доступа к члену класса нельзя применить к методу!");
		if (left_result.IsType())
		{
			if (left_result.GetType()->GetSyntax()->IsEnumeration())
			{
				int id = left_result.GetType()->GetSyntax()->GetEnumId(operation_node->GetName());
				//TODO ввести спец функции min max count
				if (id == -1)
					syntax_node->Error("Перечислимого типа с таким именем не существует!");
				else
				{
					TSExpression::TEnumValue* result = new TSExpression::TEnumValue(owner, left_result.GetType());
					result->val = id;
					Return(result);
				}
			}
			else
			{
				TSClassField* static_member = left_result.GetType()->GetField(operation_node->GetName(), true, true);
				if (static_member != NULL)
				{
					TSExpression::TGetClassField* result = new TSExpression::TGetClassField();
					result->left.reset(left);
					result->left_result = left_result;
					result->field = (TSClassField*)static_member;
					Return(result);
				}
				else
				{
					std::vector<TSMethod*> methods;
					if (left_result.GetType()->GetMethods(methods, operation_node->GetName(), true))
					{
						TSExpression::TGetMethods* result = new TSExpression::TGetMethods();
						result->left.reset(left);
						result->left_result = left_result;
						result->result = TExpressionResult(methods, method->GetSyntax()->IsStatic());
						Return(result);
					}
					else
					{
						TSClass* nested = left_result.GetType()->GetNested(operation_node->GetName());
						if (nested != NULL)
						{
							TSExpression_TGetClass* result = new TSExpression_TGetClass();
							result->left.reset((TSExpression_TGetClass*)left);
							assert(result->left != NULL);
							result->get_class = nested;
							Return(result);
						}else
							syntax_node->Error("Статического поля или метода с таким именем не существует!");
					}
				}
			}
		}
		else
		{
			TSClassField* member = left_result.GetClass() != NULL
				? left_result.GetClass()->GetField(operation_node->GetName(), true)
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
				Return(result);

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
				std::vector<TSMethod*> methods;
				if (left_result.GetClass()->GetMethods(methods, operation_node->GetName(), false))
				{
					TSExpression::TGetMethods* result = new TSExpression::TGetMethods();
					result->left.reset(left);
					result->left_result = left_result;
					result->result = TExpressionResult(methods, false);
					Return(result);
				}
				else
					syntax_node->Error("Члена класса с таким именем не существует!");
			}
		}
	}
	void Visit(SyntaxApi::IOperations::IId* operation_node)
	{
		TVariable* var = parent->GetVar(operation_node->GetName());
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
				Return(result);
			}break;
			case TVariableType::Local:
			{
				TSExpression::TGetLocal* result = new TSExpression::TGetLocal();
				result->variable=(TSLocalVar*)var;
				Return(result);
			}break;
			case TVariableType::Parameter:
			{
				TSExpression::TGetParameter* result = new TSExpression::TGetParameter();
				result->parameter = (TSParameter*)var;
				Return(result);
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
				owner->GetMethods(methods, operation_node->GetName(), true);
				if (methods.size() == 0)
				{
					std::vector<TSMethod*> temp;
					if (owner->GetMethods(temp, operation_node->GetName(), false))
					{
						syntax_node->Error("К нестатическому методу класса нельзя обращаться из статического метода!");
					}
				}
			}
			else
			{
				owner->GetMethods(methods, operation_node->GetName());
			}
			if (methods.size() != 0)
			{
				TSExpression::TGetMethods* result = new TSExpression::TGetMethods();
				result->left = NULL;
				result->result = TExpressionResult(methods, method->GetSyntax()->IsStatic());
				Return(result);
			}
			else
			{
				TSClass* type = owner->GetClass(operation_node->GetName());
				if (type != NULL)
				{
					TSExpression_TGetClass* result = new TSExpression_TGetClass();
					result->left = NULL;
					result->get_class = type;
					Return(result);
				}
				else
					syntax_node->Error("Неизвестный идентификатор!");
			}
		}
	}
	void Visit(SyntaxApi::IOperations::IThis *operation_node)
	{
		if (method->GetSyntax()->IsStatic())
			syntax_node->Error("Ключевое слово 'this' можно использовать только в нестатических методах!");
		TSExpression::TGetThis* result = new TSExpression::TGetThis();
		result->owner = owner;
		Return(result);
	}
	void Visit(SyntaxApi::IOperations::IIntValue *op)
	{
		TSExpression::TInt* result = new TSExpression::TInt(owner, op->GetType());
		result->val = op->GetValue();
		result->type.LinkSignature(build_context);
		result->type.LinkBody(build_context);
		Return(result);
	}
	void Visit(SyntaxApi::IOperations::IStringValue *op)
	{
		TSExpression::TString* result = new TSExpression::TString(owner, op->GetType());
		result->val = op->GetValue();
		result->type.LinkSignature(build_context);
		result->type.LinkBody(build_context);
		Return(result);
	}
	void Visit(SyntaxApi::IOperations::ICharValue* op)
	{
		//TODO тип char
		//TSExpression::TChar* result = new TSExpression::TChar(owner, op->GetType());
		//result->val = op->GetValue();
		//result->type.LinkSignature(build_context);
		//result->type.LinkBody(build_context);
		//Return(result);
	}
	void Visit(SyntaxApi::IOperations::IFloatValue* op)
	{
		TSExpression::TFloat* result = new TSExpression::TFloat(owner, op->GetType());
		result->val = op->GetValue();
		result->type.LinkSignature(build_context);
		result->type.LinkBody(build_context);
		Return(result);
	}
	void Visit(SyntaxApi::IOperations::IBoolValue* op)
	{
		TSExpression::TBool* result = new TSExpression::TBool(owner, op->GetType());
		result->val = op->GetValue();
		result->type.LinkSignature(build_context);
		result->type.LinkBody(build_context);
		Return(result);
	}
};

void TSExpression::Build(TGlobalBuildContext build_context)
{
	TSemanticTreeBuilder b(build_context, GetSyntax(), GetOwner(), GetMethod(), this);
	first_op.reset(b.VisitNode(GetSyntax()));
}

TVariable* TSExpression::GetVar(Lexer::TNameId name)
{
	return GetParentStatements()->GetVar(name, GetSyntax()->GetStmtId());
}

void TSExpression_TMethodCall::Build(const std::vector<TSOperation*>& param_expressions, TSMethod* method)
{
	std::vector<TFormalParameter> formal_params;
	{
		assert(param_expressions.size() == method->GetParamsCount());
		invoke = method;
		for (size_t i = 0; i < param_expressions.size(); i++)
		{
			formal_params.emplace_back(method->GetParam(i)->GetClass(), method->GetParam(i)->IsRef());
		}
	}
	params.Build(param_expressions, formal_params);
}

TSExpression::TInt::TInt(TSClass* owner, SyntaxApi::IType* syntax_node)
	:type(owner,syntax_node)
{
}
void TSExpression::TInt::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, type.GetClass());
	run_context.expression_result->get_as<int>() = val;
}
TSExpression::TFloat::TFloat(TSClass* owner, SyntaxApi::IType* syntax_node)
	:type(owner, syntax_node)
{
}
void TSExpression::TFloat::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, type.GetClass());
	run_context.expression_result->get_as<float>() = val;
}
TExpressionResult TSExpression::TFloat::GetFormalParameter()
{
	return TExpressionResult(type.GetClass(), false);
}

TSExpression::TBool::TBool(TSClass* owner, SyntaxApi::IType* syntax_node)
	:type(owner, syntax_node)
{
}
void TSExpression::TBool::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, type.GetClass());
	run_context.expression_result->get_as<int>() = val;
}
TExpressionResult TSExpression::TBool::GetFormalParameter()
{
	return TExpressionResult(type.GetClass(), false);
}

TSExpression::TString::TString(TSClass* owner, SyntaxApi::IType* syntax_node)
	:type(owner, syntax_node)
{
}
void TSExpression::TString::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, type.GetClass());
	run_context.expression_result->get_as< ::TString>().Init(val);
}
TExpressionResult TSExpression::TString::GetFormalParameter()
{
	return TExpressionResult(type.GetClass(), false);
}

TSExpression::TEnumValue::TEnumValue(TSClass* owner, TSClass* _type)
	:type(_type)
{
}
void TSExpression::TEnumValue::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, type);
	run_context.expression_result->get_as<int>() = val;
}
TExpressionResult TSExpression::TEnumValue::GetFormalParameter()
{
	return TExpressionResult(type, false);
}

TExpressionResult TSExpression_TMethodCall::GetFormalParameter()
{
	return TExpressionResult(invoke->GetRetClass(), invoke->GetSyntax()->IsReturnRef());
}
void TSExpression_TMethodCall::Run(TExpressionRunContext run_context)
{
	//соглашение о вызовах - вызывающий инициализирует параметры, резервирует место для возвращаемого значения, вызывает деструкторы параметров
	std::vector<TStackValue> method_call_formal_params;
	params.Construct(method_call_formal_params, run_context);
	switch (type)
	{
	//case TSExpression_TMethodCall::DefaultAssignOperator:
	//{
	//	memcpy(method_call_formal_params[0].get(), method_call_formal_params[1].get(), method_call_formal_params[0].GetClass()->GetSize()*sizeof(int));
	//}break;

	case TSExpression_TMethodCall::Method:
	{
		TStackValue left_result;
		if (left != NULL)
			left->Run(TExpressionRunContext(run_context, &left_result));

		if (invoke->GetRetClass() != NULL)
			*run_context.expression_result = TStackValue(invoke->GetSyntax()->IsReturnRef(), invoke->GetRetClass());

		invoke->Run(TMethodRunContext(run_context.static_fields, &method_call_formal_params, run_context.expression_result, &left_result));
	}break;

	case TSExpression_TMethodCall::Operator:
	{
		if (invoke->GetRetClass() != NULL)
			*run_context.expression_result = TStackValue(invoke->GetSyntax()->IsReturnRef(), invoke->GetRetClass());
		invoke->Run(TMethodRunContext(run_context.static_fields, &method_call_formal_params, run_context.expression_result, nullptr));
	}break;

	case TSExpression_TMethodCall::ObjectConstructor:
	{
		invoke->Run(TMethodRunContext(run_context.static_fields, &method_call_formal_params, nullptr, run_context.object));
	}break;

	default:
		assert(false);
	}
	params.Destroy(method_call_formal_params, run_context);
}
//void TSExpression_TMethodCall::Run(TStackValue& object_to_construct, TMethodRunContext run_context, std::vector<TStackValue>& local_variables)
//{
//	//соглашение о вызовах - вызывающий инициализирует параметры, резервирует место для возвращаемого значения, вызывает деструкторы параметров
//	std::vector<TStackValue> method_call_formal_params;
//	params.Construct(method_call_formal_params, static_fields, formal_params, object, local_variables);
//	
//	switch (type)
//	{
//	case TSExpression_TMethodCall::ObjectConstructorInitWithAssign:
//	{
//		assert(object_to_construct.GetSize() == method_call_formal_params[0].GetSize());
//		memcpy(object_to_construct.get(), method_call_formal_params[0].get(), object_to_construct.GetClass()->GetSize()*sizeof(int));
//	}break;
//	case TSExpression_TMethodCall::ObjectConstructor:
//	{
//		TStackValue withoutresult;
//		invoke->Run(static_fields, method_call_formal_params, withoutresult, object_to_construct);
//	}break;
//	default:
//		assert(false);
//	}
//	params.Destroy(method_call_formal_params, static_fields, formal_params, object, local_variables);
//}
TExpressionResult TSExpression::TInt::GetFormalParameter()
{
	return TExpressionResult(type.GetClass(),false);
}
TExpressionResult TSExpression::TGetMethods::GetFormalParameter()
{
	return result;
}
void TSExpression::TGetMethods::Run(TExpressionRunContext run_context)
{
	if (left == NULL)
		return;

	//auto exp_result_type = left->GetFormalParameter();
	//if (!exp_result_type.IsRef())
	//{
	//	//TODO не в пустоту, а в массив временных объектов выражения
	//	left->Run(static_fields, formal_params, result, *new TStackValue(false, exp_result_type.GetClass()), local_variables);
	//}
	//else
		left->Run(run_context);
}
TExpressionResult TSExpression::TGetClassField::GetFormalParameter()
{
	//при получении статической переменной возвращается ссылка
	if (left_result.IsType())
		return TExpressionResult(field->GetClass(), true);
	else
		//return TExpressionResult(field->GetClass(),left_result.IsRef());
		return TExpressionResult(field->GetClass(), true);
}
void TSExpression::TGetClassField::Run(TExpressionRunContext run_context)
{
	if (field->GetSyntax()->IsStatic())
	{ 
		*run_context.expression_result = TStackValue(true, field->GetClass());
		run_context.expression_result->SetAsReference((*run_context.static_fields)[field->GetOffset()].get());
	}
	else
	{
		if (left != NULL)
		{
			auto exp_result_type = left->GetFormalParameter();
			if (!exp_result_type.IsRef())
			{
				//TODO не в пустоту, а в массив временных объектов выражения
				auto temp = new TStackValue(false, exp_result_type.GetClass());
				left->Run(TExpressionRunContext(run_context, temp));
				*run_context.expression_result = TStackValue(true, field->GetClass());
				run_context.expression_result->SetAsReference(((int*)(*temp).get()) + field->GetOffset());
			}
			else
			{
				TStackValue exp_result;
				left->Run(TExpressionRunContext(run_context, &exp_result));
				*run_context.expression_result = TStackValue(true, field->GetClass());
				run_context.expression_result->SetAsReference(((int*)exp_result.get()) + field->GetOffset());
			}			
		}
		else
			//иначе необходимо получить ссылку на поле данного класса
		{
			*run_context.expression_result = TStackValue(true, field->GetClass());
			run_context.expression_result->SetAsReference(((int*)run_context.object->get()) + field->GetOffset());
		}
	}
}
TExpressionResult TSExpression::TGetParameter::GetFormalParameter()
{
	return TExpressionResult(parameter->GetClass(), true);
}
void TSExpression::TGetParameter::Run(TExpressionRunContext run_context)
{
	//sp_base - parameter offset
	void* param = (*run_context.formal_params)[parameter->GetOffset()].get();
	*run_context.expression_result = TStackValue(true, parameter->GetClass());
	run_context.expression_result->SetAsReference(param);
}
TExpressionResult TSExpression::TGetLocal::GetFormalParameter()
{
	return TExpressionResult(variable->GetClass(), true);
}
void TSExpression::TGetLocal::Run(TExpressionRunContext run_context)
{
	void* variable_value = NULL;
	if (variable->IsStatic())
	{
		variable_value = (*run_context.static_fields)[variable->GetOffset()].get();
	}
	else
	{
		variable_value = (*run_context.local_variables)[variable->GetOffset()].get();
	}
	*run_context.expression_result = TStackValue(true, variable->GetClass());
	run_context.expression_result->SetAsReference(variable_value);
}

TExpressionResult TSExpression_TGetClass::GetFormalParameter()
{
	return TExpressionResult(get_class);
}

void TSExpression_TGetClass::Run(TExpressionRunContext run_context)
{
	//nothing to do
}

void TSExpression_TCreateTempObject::Build(const std::vector<SyntaxApi::IExpression*>& param_expressions)
{

}
TSExpression_TempObjectType::TSExpression_TempObjectType(TSClass* owner, SyntaxApi::IType* syntax_node)
	:type(owner,syntax_node)
{
}
TExpressionResult TSExpression_TCreateTempObject::GetFormalParameter()
{
	return TExpressionResult(left->GetFormalParameter().GetType(),false);
}

void TSExpression_TCreateTempObject::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, left->GetFormalParameter().GetType());
	construct_object->Construct(*run_context.expression_result, run_context);
}
TExpressionResult TSExpression_TempObjectType::GetFormalParameter()
{
	return TExpressionResult(type.GetClass());
}
void TSExpression_TempObjectType::Run(TExpressionRunContext run_context)
{
	//nothing to do
}
TExpressionResult TSExpression::TGetThis::GetFormalParameter()
{
	return TExpressionResult(owner, true);
}
void TSExpression::TGetThis::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(true, owner);
	run_context.expression_result->SetAsReference(run_context.object->get());
}

void TSExpression::Run(TStatementRunContext run_context)
{
	TStackValue exp_result;
	first_op->Run(TExpressionRunContext(run_context, &exp_result));
}

void TSExpression::Run(TExpressionRunContext run_context)
{
	first_op->Run(run_context);
}

SyntaxApi::IExpression* TSExpression::GetSyntax()
{
	return dynamic_cast<SyntaxApi::IExpression*>(TSStatement::GetSyntax());
}
