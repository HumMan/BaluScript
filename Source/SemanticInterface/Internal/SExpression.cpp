#include "SExpression.h"

#include "SCommon.h"

#include "SStatements.h"
#include "SLocalVar.h"
#include "SClass.h"
#include "SClassField.h"
#include "SConstructObject.h"

#include "../../NativeTypes/String.h"

class TSemanticTreeBuilder :public SyntaxApi::IExpressionTreeVisitor
{
	//общие для всех выражений поля
	SyntaxApi::IExpression* syntax_node;
	TSClass* owner;
	TSMethod* method;
	TSExpression* parent;
	SemanticApi::TGlobalBuildContext build_context;

	TSOperation* visit_result;
public:
	TSemanticTreeBuilder(SemanticApi::TGlobalBuildContext build_context, SyntaxApi::IExpression* syntax_node, TSClass* owner, TSMethod* method, TSExpression* parent)
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

		SemanticApi::ISMethod *bin_operator = nullptr;

		std::vector<TExpressionResult> param;
		std::vector<SyntaxApi::IMethod*> bin_operators;

		param.resize(2);
		param[0] = left->GetFormalParameter();
		param[1] = right->GetFormalParameter();

		if (param[0].IsVoid())
			syntax_node->Error("К левому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");
		if (param[1].IsVoid())
			syntax_node->Error("К правому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");

		std::vector<SemanticApi::ISMethod*> operators;

		//список доступных операторов получаем из левого операнда
		param[0].GetClass()->GetOperators(operators, operation_node->GetOp());
		bin_operator = FindMethod(syntax_node, operators, param);
		if (operation_node->GetOp() >= Lexer::TOperator::Assign&&operation_node->GetOp() <= Lexer::TOperator::OrA && !param[0].IsRef())
			syntax_node->Error("Для присваиваниия требуется ссылка, а не значение!");

		if (bin_operator != nullptr)
		{
			ValidateAccess(syntax_node, owner, bin_operator);
		}
		else syntax_node->Error("Бинарного оператора для данных типов не существует!");

		TSExpression_TMethodCall* method_call = nullptr;

		method_call = new TSExpression_TMethodCall(SemanticApi::TMethodCallType::Operator);
		method_call->Build(param_expressions, dynamic_cast<TSMethod*>(bin_operator));

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

		SemanticApi::ISMethod *unary_operator = nullptr;

		if (param[0].GetClass() == nullptr)
			syntax_node->Error("К данному операнду нельзя применить унарный оператор (нужен тип отличающийся от void)!");

		std::vector<SemanticApi::ISMethod*> operators;
		param[0].GetClass()->GetOperators(operators, operation_node->GetOp());

		unary_operator = FindMethod(syntax_node, operators, param);

		if (unary_operator != nullptr)
		{
			ValidateAccess(syntax_node, owner, unary_operator);

			TSExpression_TMethodCall* method_call = new TSExpression_TMethodCall(SemanticApi::TMethodCallType::Operator);
			method_call->Build(param_expressions, dynamic_cast<TSMethod*>(unary_operator));
			Return(method_call);
		}
		else
			syntax_node->Error("Унарного оператора для данного типа не существует!");
	}
	void Visit(SyntaxApi::IOperations::IConstructTempObject* operation_node)
	{
		TSExpression_TempObjectType* result = new TSExpression_TempObjectType(owner, operation_node->GetType(), build_context);
		Return(result);
	}
	void Visit(SyntaxApi::IOperations::ICallParamsOp* operation_node)
	{
		TSOperation *left;
		left = VisitNode(operation_node->GetLeft());

		TExpressionResult left_result = left->GetFormalParameter();
		
		std::vector<TExpressionResult> params_result;
		std::vector<TSOperation*> param_expressions;
		std::vector<SemanticApi::TFormalParameter> params_formals;
		auto param = operation_node->GetParam();
		for (size_t i = 0; i < param.size(); i++)
		{
			TSOperation* return_new_operation = VisitNode(param[i]);
			param_expressions.push_back(return_new_operation);
			params_result.push_back(return_new_operation->GetFormalParameter());
			params_formals.push_back(SemanticApi::TFormalParameter(params_result.back().GetClass(), params_result.back().IsRef()));
		}
		
		if (left_result.IsMethods())
		{
			//вызов метода
			if (operation_node->IsBracket())
				assert(false);//при вызове метода используются круглые скобки
			SemanticApi::ISMethod* invoke = nullptr;
			SemanticApi::ISMethod* method = FindMethod(syntax_node, left_result.GetMethods(), params_result);
			if (method != nullptr)
			{
				ValidateAccess(syntax_node, owner, method);
				invoke = method;
			}
			else 
				syntax_node->Error("Метода с такими параметрами не существует");

			TSExpression_TMethodCall* method_call = new TSExpression_TMethodCall(SemanticApi::TMethodCallType::Method);
			method_call->Build(param_expressions, dynamic_cast<TSMethod*>(invoke));
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
			TSClass* constr_class = dynamic_cast<TSClass*>(left_result.GetType());

			auto construct_object = new TSConstructObject(owner, method, parent->GetParentStatements(), constr_class);
			construct_object->Build(operation_node->GetOperationSource(), params_result, param_expressions, params_formals, build_context);

			TSExpression_TCreateTempObject* create_temp_obj = new TSExpression_TCreateTempObject(
				(TSExpression_TempObjectType*)left,
				construct_object
			);
			Return(create_temp_obj);
		}
		else
			//иначе вызов оператора () или []
		{
			//т.к. все операторы статические - первым параметром передаем ссылку на объект
			param_expressions.insert(param_expressions.begin(), left);
			params_result.insert(params_result.begin(), left_result);
			
			left = nullptr;
			SemanticApi::ISMethod* invoke = nullptr;
			std::vector<SemanticApi::ISMethod*> operators;
			left_result.GetClass()->GetOperators(operators, operation_node->IsBracket() ? (Lexer::TOperator::GetArrayElement) : (Lexer::TOperator::ParamsCall));
			SemanticApi::ISMethod* method = FindMethod(syntax_node, operators, params_result);
			if (method != nullptr)
			{
				ValidateAccess(syntax_node, owner, method);
				invoke = method;
			}
			else
				syntax_node->Error("Оператора с такими параметрами не существует!");

			TSExpression_TMethodCall* method_call = new TSExpression_TMethodCall(SemanticApi::TMethodCallType::Operator);
			method_call->Build(param_expressions, dynamic_cast<TSMethod*>(invoke));

			Return(method_call);
		}		
	}
	
	void Visit(SyntaxApi::IOperations::IGetMemberOp* operation_node)
	{
		TSOperation *left;
		left = VisitNode(operation_node->GetLeft());

		TExpressionResult left_result = left->GetFormalParameter();

		auto left_class = dynamic_cast<TSClass*>(left_result.GetClass());

		if (left_result.IsMethods())
			syntax_node->Error("Оператор доступа к члену класса нельзя применить к методу!");
		if (left_result.IsType())
		{
			auto left_result_class = dynamic_cast<TSClass*>(left_result.GetType());
			if (left_result_class->GetSyntax()->IsEnumeration())
			{
				int id = left_result_class->GetSyntax()->GetEnumId(operation_node->GetName());
				//TODO ввести спец функции min max count
				if (id == -1)
					syntax_node->Error("Перечислимого типа с таким именем не существует!");
				else
				{
					delete left;
					TSExpression::TEnumValue* result = new TSExpression::TEnumValue(owner, dynamic_cast<TSClass*>(left_result.GetType()));
					result->val = id;
					Return(result);
				}
			}
			else
			{
				TSClassField* static_member = left_result_class->GetField(operation_node->GetName(), true, true);
				if (static_member != nullptr)
				{
					TSExpression::TGetClassField* result = new TSExpression::TGetClassField(
						left, left_result, static_member);
					Return(result);
				}
				else
				{
					std::vector<SemanticApi::ISMethod*> methods;
					if (left_result_class->GetMethods(methods, operation_node->GetName(), SemanticApi::Filter::True))
					{
						TSExpression::TGetMethods* result = new TSExpression::TGetMethods(
							left, left_result, TExpressionResult(methods, method->GetSyntax()->IsStatic()));
						Return(result);
					}
					else
					{
						TSClass* nested = left_result_class->GetNested(operation_node->GetName());
						if (nested != nullptr)
						{
							TSExpression_TGetClass* result = new TSExpression_TGetClass(
								dynamic_cast<TSExpression_TGetClass*>(left), nested);

							Return(result);
						}else
							syntax_node->Error("Статического поля или метода с таким именем не существует!");
					}
				}
			}
		}
		else
		{
			TSClassField* member = (left_class != nullptr)
				? left_class->GetField(operation_node->GetName(), true)
				: nullptr;
			if (member != nullptr)
			{
				if (member->GetSyntax()->IsStatic())
					syntax_node->Error("Оператор доступа к члену класса нельзя применить к объекту для доступа к статическому члену, \nвместо объекта следует использовать имя класса!");
				ValidateAccess(syntax_node, owner, member);

				TSExpression::TGetClassField* result = new TSExpression::TGetClassField(
					left, left_result, member);
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
				std::vector<SemanticApi::ISMethod*> methods;
				if (left_class->GetMethods(methods, operation_node->GetName(), SemanticApi::Filter::False))
				{
					TSExpression::TGetMethods* result = new TSExpression::TGetMethods(
						left, left_result, TExpressionResult(methods, false));
					Return(result);
				}
				else
					syntax_node->Error("Члена класса с таким именем не существует!");
			}
		}
	}
	void Visit(SyntaxApi::IOperations::IId* operation_node)
	{
		SemanticApi::IVariable* var = parent->GetVar(operation_node->GetName());
		if (var != nullptr)
		{
			switch (var->GetVariableType())
			{
			case SemanticApi::VariableType::ClassField:
			{
				ValidateAccess(syntax_node, owner, (TSClassField*)var);
				auto var_field = dynamic_cast<TSClassField*>(var);
				if ((!var_field->GetSyntax()->IsStatic()) && method->GetSyntax()->IsStatic())
					syntax_node->Error("К нестатическому полю класса нельзя обращаться из статического метода!");
				TSExpression::TGetClassField* result = new TSExpression::TGetClassField(
					nullptr, TExpressionResult(dynamic_cast<TSClass*>(var_field->GetClass()), true), var_field);
				Return(result);
			}break;
			case SemanticApi::VariableType::Local:
			{
				TSExpression::TGetLocal* result = new TSExpression::TGetLocal(dynamic_cast<TSLocalVar*>(var));
				Return(result);
			}break;
			case SemanticApi::VariableType::Parameter:
			{
				TSExpression::TGetParameter* result = new TSExpression::TGetParameter(dynamic_cast<TSParameter*>(var));
				Return(result);
			}break;
			default:
				assert(false);//ошибка в поиске переменной
			}
		}
		else
		{
			std::vector<SemanticApi::ISMethod*> methods;
			if (method->GetSyntax()->IsStatic())
			{
				owner->GetMethods(methods, operation_node->GetName(), SemanticApi::Filter::True);
				if (methods.size() == 0)
				{
					std::vector<SemanticApi::ISMethod*> temp;
					if (owner->GetMethods(temp, operation_node->GetName(), SemanticApi::Filter::False))
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
				TSExpression::TGetMethods* result = new TSExpression::TGetMethods(nullptr, TExpressionResult(), TExpressionResult(methods, method->GetSyntax()->IsStatic()));
				Return(result);
			}
			else
			{
				TSClass* type = owner->GetClass(operation_node->GetName());
				if (type != nullptr)
				{
					TSExpression_TGetClass* result = new TSExpression_TGetClass(nullptr, type);					
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
		TSExpression::TGetThis* result = new TSExpression::TGetThis(owner);
		Return(result);
	}
	void Visit(SyntaxApi::IOperations::IIntValue *op)
	{
		TSExpression::TInt* result = new TSExpression::TInt(owner, op->GetType(), op->GetValue(), build_context);		
		Return(result);
	}
	void Visit(SyntaxApi::IOperations::IStringValue *op)
	{
		TSExpression::TString* result = new TSExpression::TString(owner, op->GetType(), op->GetValue(), build_context);
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
		TSExpression::TFloat* result = new TSExpression::TFloat(owner, op->GetType(), op->GetValue(), build_context);
		Return(result);
	}
	void Visit(SyntaxApi::IOperations::IBoolValue* op)
	{
		TSExpression::TBool* result = new TSExpression::TBool(owner, op->GetType(), op->GetValue(), build_context);
		Return(result);
	}
};

void TSExpression::Build(SemanticApi::TGlobalBuildContext build_context)
{
	TSemanticTreeBuilder b(build_context, GetSyntax(), GetOwner(), GetMethod(), this);
	auto syntax = GetSyntax();
	if (!syntax->IsEmpty())
		first_op.reset(b.VisitNode(syntax));

	expression_result = first_op->GetFormalParameter();
}

SemanticApi::IVariable* TSExpression::GetVar(Lexer::TNameId name)
{
	return GetParentStatements()->GetVar(name, GetSyntax()->GetStmtId());
}

void TSExpression_TMethodCall::Build(const std::vector<TSOperation*>& param_expressions, TSMethod* method)
{
	std::vector<SemanticApi::TFormalParameter> formal_params;
	{
		assert(param_expressions.size() == method->GetParamsCount());
		invoke = method;
		for (size_t i = 0; i < param_expressions.size(); i++)
		{
			formal_params.emplace_back(method->GetParam(i)->GetClass(), method->GetParam(i)->IsRef());
		}
	}
	params.Build(param_expressions, formal_params);

	expression_result = TExpressionResult(invoke->GetRetClass(), invoke->IsReturnRef());
}

TSExpression::TInt::TInt(TSClass* owner, SyntaxApi::IType* syntax_node, int value, SemanticApi::TGlobalBuildContext build_context)
	:type(owner,syntax_node)
{
	val = value;
	type.LinkSignature(build_context);
	type.LinkBody(build_context);
	expression_result = TExpressionResult(dynamic_cast<TSClass*>(type.GetClass()), false);
}

TSExpression::TFloat::TFloat(TSClass* owner, SyntaxApi::IType* syntax_node, float value, SemanticApi::TGlobalBuildContext build_context)
	: type(owner, syntax_node)
{
	val = value;
	type.LinkSignature(build_context);
	type.LinkBody(build_context);
	expression_result = TExpressionResult(dynamic_cast<TSClass*>(type.GetClass()), false);
}

TSExpression::TBool::TBool(TSClass* owner, SyntaxApi::IType* syntax_node, bool value, SemanticApi::TGlobalBuildContext build_context)
	:type(owner, syntax_node)
{
	val = value;
	type.LinkSignature(build_context);
	type.LinkBody(build_context);
	expression_result = TExpressionResult(dynamic_cast<TSClass*>(type.GetClass()), false);
}

TSExpression::TString::TString(TSClass* owner, SyntaxApi::IType* syntax_node, std::string value, SemanticApi::TGlobalBuildContext build_context)
	:type(owner, syntax_node)
{
	val = value;
	type.LinkSignature(build_context);
	type.LinkBody(build_context);
	expression_result = TExpressionResult(dynamic_cast<TSClass*>(type.GetClass()), false);
}

TSExpression::TEnumValue::TEnumValue(TSClass* owner, TSClass* _type)
	:type(_type)
{
	expression_result = TExpressionResult(type, false);
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

TSExpression_TempObjectType::TSExpression_TempObjectType(TSClass* owner, SyntaxApi::IType* syntax_node, SemanticApi::TGlobalBuildContext build_context)
	:type(owner,syntax_node)
{
	type.LinkSignature(build_context);
	type.LinkBody(build_context);
	expression_result = TExpressionResult(dynamic_cast<TSClass*>(type.GetClass()));
}

SyntaxApi::IExpression* TSExpression::GetSyntax()
{
	return dynamic_cast<SyntaxApi::IExpression*>(TSStatement::GetSyntax());
}

SemanticApi::ISOperations::ISOperation * TSExpression::GetFirstOp() const
{
	return first_op.get();
}

void TSExpression::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
void TSExpression::Accept(SemanticApi::ISStatementVisitor * visitor)
{
	visitor->Visit(this);
}
TSExpression_TCreateTempObject::TSExpression_TCreateTempObject(TSExpression_TempObjectType* left, TSConstructObject* construct_object)
	:left(left), construct_object(construct_object)
{
	expression_result = TExpressionResult(dynamic_cast<TSClass*>(left->GetFormalParameter().GetType()), false);
}
void TSExpression_TCreateTempObject::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
SemanticApi::ISOperations::ISExpression_TempObjectType * TSExpression_TCreateTempObject::GetLeft() const
{
	return left.get();
}
SemanticApi::ISConstructObject * TSExpression_TCreateTempObject::GetConstructObject() const
{
	return construct_object.get();
}
void TSExpression_TempObjectType::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
TSExpression::TGetThis::TGetThis(TSClass* owner)
	:owner(owner)
{
	expression_result = TExpressionResult(owner, true);
}
void TSExpression::TGetThis::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}

SemanticApi::ISClass * TSExpression::TGetThis::GetOwner() const
{
	return owner;
}

TSExpression_TGetClass::TSExpression_TGetClass(TSExpression_TGetClass* left, TSClass * get_class)
	:left(left), get_class(get_class)
{
	expression_result = TExpressionResult(get_class);
}

void TSExpression_TGetClass::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
TSExpression::TGetLocal::TGetLocal(TSLocalVar * variable)
	:variable(variable)
{
	expression_result = TExpressionResult(variable->GetClass(), true);
}
void TSExpression::TGetLocal::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
SemanticApi::ISLocalVar * TSExpression::TGetLocal::GetVariable() const
{
	return variable;
}
TSExpression::TGetParameter::TGetParameter(TSParameter * parameter)
	:parameter(parameter)
{
	expression_result = TExpressionResult(dynamic_cast<TSClass*>(parameter->GetClass()), true);
}
void TSExpression::TGetParameter::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
SemanticApi::ISParameter * TSExpression::TGetParameter::GetParameter() const
{
	return parameter;
}
TSExpression::TGetClassField::TGetClassField(TSOperation* left, TExpressionResult left_result, TSClassField* field)
	:left(left), left_result(left_result), field(field)
{
	expression_result = TExpressionResult(dynamic_cast<TSClass*>(field->GetClass()), true);
}
void TSExpression::TGetClassField::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
SemanticApi::ISClassField * TSExpression::TGetClassField::GetField() const
{
	return field;
}
SemanticApi::ISOperations::ISOperation * TSExpression::TGetClassField::GetLeft() const
{
	return left.get();
}
TSExpression::TGetMethods::TGetMethods(TSOperation* left, TExpressionResult left_result, TExpressionResult result)
	:left(left), left_result(left_result), result(result)
{
	expression_result = result;
}
void TSExpression::TGetMethods::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
SemanticApi::ISOperations::ISOperation * TSExpression::TGetMethods::GetLeft() const
{
	return left.get();
}
void TSExpression::TInt::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
int TSExpression::TInt::GetValue() const
{
	return val;
}
const SemanticApi::ISType* TSExpression::TInt::GetType()const
{
	return &type;
}
void TSExpression::TEnumValue::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
SemanticApi::ISClass * TSExpression::TEnumValue::GetType() const
{
	return type;
}
int TSExpression::TEnumValue::GetValue() const
{
	return val;
}
void TSExpression_TMethodCall::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
const SemanticApi::IActualParameters * TSExpression_TMethodCall::GetParams() const
{
	return &params;
}
SemanticApi::TMethodCallType TSExpression_TMethodCall::GetType() const
{
	return type;
}
SemanticApi::ISMethod * TSExpression_TMethodCall::GetInvoke() const
{
	return invoke;
}
SemanticApi::ISOperations::ISOperation * TSExpression_TMethodCall::GetLeft() const
{
	return left.get();
}
void TSExpression::TString::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
std::string TSExpression::TString::GetValue() const
{
	return val;
}
const SemanticApi::ISType * TSExpression::TString::GetType() const
{
	return &type;
}
void TSExpression::TBool::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}
bool TSExpression::TBool::GetValue() const
{
	return val;
}
const SemanticApi::ISType * TSExpression::TBool::GetType() const
{
	return &type;
}
void TSExpression::TFloat::Accept(ISExpressionVisitor * visitor)
{
	visitor->Visit(this);
}

float TSExpression::TFloat::GetValue() const
{
	return val;
}

const SemanticApi::ISType * TSExpression::TFloat::GetType() const
{
	return &type;
}

const SemanticApi::IExpressionResult * TSOperation::GetFormalParam() const
{
	return &expression_result;
}

TExpressionResult TSOperation::GetFormalParameter() const
{
	return expression_result;
}

TSExpression::TSExpression(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IExpression* syntax_node)
	:TSStatement(SyntaxApi::TStatementType::Expression, use_owner, use_method, use_parent, (SyntaxApi::IStatement*)(syntax_node)) 
{

}