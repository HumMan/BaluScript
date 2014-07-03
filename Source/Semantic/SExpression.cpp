#include "SExpression.h"

#include "../Syntax/Expression.h"
#include "FormalParam.h"
#include "../semanticAnalyzer.h"
#include "SStatements.h"

class TSemanticTreeBuilder :public TExpressionTreeVisitor
{
	TSExpression::TOperation* return_new_operation;
	TExpression* syntax_node;
	TSClass* owner;
	TSMethod* method;
	TSExpression* parent;
public:
	TSemanticTreeBuilder(TExpression* syntax_node, TSClass* owner, TSMethod* method, TSExpression* parent)
	{
		this->syntax_node = syntax_node;
		this->owner = owner;
		this->method = method;
		this->parent = parent;
	}
	void Visit(TExpression::TBinOp* operation_node)
	{
		TSExpression::TOperation *left,*right;
		operation_node->left->Accept(this);
		left = return_new_operation;
		operation_node->right->Accept(this);
		right = return_new_operation;

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

		TFormalParam result;
		if (bin_operator != NULL && (bin_operator2 == NULL || left_conv_count<right_conv_count))
		{
			ValidateAccess(syntax_node, owner, bin_operator);
			//result = bin_operator->BuildCall(program, param);
		}
		else if (bin_operator2 != NULL)
		{
			ValidateAccess(syntax_node, owner, bin_operator2);
			//result = bin_operator2->BuildCall(program, param);
		}
		else syntax_node->Error("Бинарного оператора для данных типов не существует!");


		//return param[0].GetOps() + param[1].GetOps() + result;
	}
	void Visit(TExpression::TUnaryOp* operation_node)
	{
		TSExpression::TOperation *left;
		operation_node->left->Accept(this);
		left = return_new_operation;

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
			//TFormalParam result = unary_operator->BuildCall(program, param);
			//return param[0].GetOps() + result;
		}
		else syntax_node->Error("Унарного оператора для данного типа не существует!");
		//return TVoid();
	}
	void Visit(TExpression::TCallParamsOp* operation_node)
	{
		TSExpression::TOperation *left;
		operation_node->left->Accept(this);
		left = return_new_operation;

		TFormalParam left_result = left->GetFormalParameter();//Build(program, parent);
		std::vector<TFormalParam> params_result;

		//for (int i = 0; i<operation_node->param.size(); i++)
		//	params_result.push_back(operation_node->param[i]->Build(program, parent));
		int conv_need = 0;
		if (left_result.IsMethods())
		{
			//вызов метода
			if (operation_node->is_bracket)
				assert(false);//при вызове метода используются круглые скобки
			TSMethod* method = FindMethod(syntax_node, left_result.GetMethods(), params_result, conv_need);
			if (method != NULL)
			{
				ValidateAccess(syntax_node, owner, method);
				//TOpArray params_ops;
				//TFormalParam call_ops = method->BuildCall(program, params_result);
				//for (int i = 0; i<param.size(); i++)
				//	params_ops += params_result[i].GetOps();
				//if (!method->IsStatic() && left_result.NeedPushThis())
				//	program.Push(TOp(TOpcode::PUSH_THIS), params_ops);
				//return left_result.GetOps() + params_ops + call_ops;
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
			//TOpArray before_params;
			//if (constructor == NULL)
			//	program.PushFront(TOp(TOpcode::PUSH_COUNT, constr_class->GetSize()), before_params);
			if (constructor != NULL)
			{
				ValidateAccess(syntax_node, owner, constructor);
				//if(constructor->GetType()==MT_USER||constructor->GetType()==MT_EXTERNAL)
				//{
				//program.Push(TOp(TOpcode::PUSH_STACK_HIGH_REF, constr_class->GetSize()), before_params);
				////}
				//TOpArray call_ops = constructor->BuildCall(program, params_result).GetOps();
				//TOpArray params_ops;
				//for (int i = 0; i<param.size(); i++)
				//	params_ops += params_result[i].GetOps();
				//return before_params + params_ops + TFormalParam(constr_class, false, call_ops);
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
				//TOpArray params_ops;
				//TFormalParam call_ops = method->BuildCall(program, params_result);
				//for (int i = 0; i<params_result.size(); i++)
				//	params_ops += params_result[i].GetOps();
				//return params_ops + call_ops;
			}
			else
				syntax_node->Error("Оператора с такими параметрами не существует!");
		}
		//assert(false); return TVoid();
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
				//if (static_member != NULL)
				//	return static_member->PushRefToStack(program);
				std::vector<TSMethod*> methods;
				//if (left_result.GetType()->GetMethods(methods, operation_node->name, true))
				//	return TFormalParam(methods, false);
				TSClass* nested = left_result.GetType()->GetNested(operation_node->name);
				//if (nested != NULL)
				//	return TFormalParam(nested);
				//Error("Статического поля или метода с таким именем не существует!");
			}
		}
		TSClassField* member = left_result.GetClass() != NULL
			? left_result.GetClass()->GetField(operation_node->name, true)
			: NULL;
		if (member != NULL)
		{
			if (member->GetSyntax()->IsStatic())
				syntax_node->Error("Оператор доступа к члену класса нельзя применить к объекту для доступа к статическому члену, \nвместо объекта следует использовать имя класса!");
			ValidateAccess(syntax_node, owner, member);
			if (left_result.IsRef())
			{
				//TFormalParam result = member->PushRefToStack(program);
				//if (member->IsReadOnly())
				{
					//TSMethod* copy_constr = result.GetClass()->GetCopyConstr();
					//program.Push(TOp(TOpcode::RVALUE, result.GetClass()->GetSize(), program.AddMethodToTable(copy_constr))
					//	, result.GetOps());
					//result.SetIsRef(false);
				}
				//return left_result.GetOps() + result;
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
			if (!left_result.GetClass()->GetMethods(methods, operation_node->name, false))
				syntax_node->Error("Члена класса с таким именем не существует!");
			//TFormalParam result(methods, false);
			//result.GetOps() = left_result.GetOps();
			//return result;
		}
		//assert(false);
	}
	void Visit(TExpression::TId* operation_node)
	{
		TVariable* var = parent->GetVar(operation_node->name);
		if (var != NULL)
		{
			if (var->GetType() == TVariableType::ClassField)
			{
				ValidateAccess(syntax_node, owner, (TSClassField*)var);
				if (!((TClassField*)var)->IsStatic() && method->GetSyntax()->IsStatic())
					syntax_node->Error("К нестатическому полю класса нельзя обращаться из статического метода!");
			}
			//TOpArray temp;
			//if (var->NeedPushThis())
			//	program.Push(TOp(TOpcode::PUSH_THIS), temp);
			//return temp + var->PushRefToStack(program);
		}
		std::vector<TSMethod*> methods;
		if (method->GetSyntax()->IsStatic())
			owner->GetMethods(methods, operation_node->name, true);
		else
			owner->GetMethods(methods, operation_node->name);
		if (methods.size() != 0)
		{
			//return TFormalParam(methods, true);
		}
		else
		{
			if (method->GetSyntax()->IsStatic())
			{
				methods.clear();
				if (owner->GetMethods(methods, operation_node->name, false))
				{
					syntax_node->Error("К нестатическому методу класса нельзя обращаться из статического метода!");
				}
			}
		}
		TSClass* type = owner->GetClass(operation_node->name);
		if (type == NULL)
		{
			syntax_node->Error("Неизвестный идентификатор!");
		}
		//return TFormalParam(type);
	}
	void Visit(TExpression::TThis *operation_node)
	{
		if (method->GetSyntax()->IsStatic())
			syntax_node->Error("Ключевое слово 'this' можно использовать только в нестатических методах!");
		//TOpArray temp;
		//program.Push(TOp(TOpcode::PUSH_THIS), temp);
		//return TFormalParam(parent->owner, true, temp);
	}
	void Visit(TExpression::TIntValue *op)
	{

	}
	void Visit(TExpression::TStringValue *op)
	{

	}
	void Visit(TExpression::TCharValue* op)
	{

	}
	void Visit(TExpression::TFloatValue* op)
	{

	}
	void Visit(TExpression::TBoolValue* op)
	{

	}
};

void TSExpression::Build()
{
	TSemanticTreeBuilder b(GetSyntax(),owner,method,this);
	GetSyntax()->Accept(&b);

}

TVariable* TSExpression::GetVar(TNameId name)
{
	return parent->GetVar(name, GetSyntax()->stmt_id);
}