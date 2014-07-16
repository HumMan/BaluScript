#include "SLocalVar.h"

#include "SMethod.h"
#include "SSyntaxNode.h"
#include "../Syntax/LocalVar.h"
#include "SStatements.h"
#include "../semanticAnalyzer.h"
#include "FormalParam.h"
#include "SExpression.h"
#include "../Syntax/Expression.h"
#include "../Syntax/Statements.h"

TFormalParam TSLocalVar::Build()
{
	std::vector<TSMethod*> methods;
	if (owner->GetMethods(methods, GetSyntax()->name))
		GetSyntax()->Error("Метод не может быть именем переменной!");
	if (owner->GetClass(GetSyntax()->name) != NULL)
		GetSyntax()->Error("Класс не может быть именем переменной!");
	TVariable* t = parent->GetVar(GetSyntax()->name, GetSyntax()->stmt_id);
	if (t != NULL&&t != this)
	{
		switch (t->GetType())
		{
		case TVariableType::ClassField:
			GetSyntax()->Error("Локальная переменная перекрывает член класса!");
		case TVariableType::Parameter:
			GetSyntax()->Error("Локальная переменная перекрывает параметр!");
		case TVariableType::Local:
			GetSyntax()->Error("Локальная переменная с таким именем уже существует!");
		default:assert(false);
		}
	}
	if (GetSyntax()->is_static)
	{
		assert(false);
	}
	else
	{
	}

	type.LinkSignature();
	type.LinkBody();

	parent->AddVar(this, GetSyntax()->stmt_id);

	std::vector<TFormalParam> params_result;

	for (const std::unique_ptr<TExpression>& param_syntax : GetSyntax()->params)
	{
		auto t = new TSExpression(owner, method, parent, param_syntax.get());
		t->Build();
		params.push_back(std::unique_ptr<TSExpression>(t));
		params_result.push_back(params.back()->GetFormalParameter());
	}

	int conv_need = 0;
	std::vector<TSMethod*> constructors;

	type.GetClass()->GetConstructors(constructors);
	TSMethod* constructor = FindMethod(GetSyntax(), constructors, params_result, conv_need);
	if (constructor == NULL&&params_result.size() > 0)
		GetSyntax()->Error("Коструктора с такими парметрами не существует!");

	//bool need_testandget=GetSyntax()->is_static&&(params_result.size()>0||assign_expr!=NULL);

	if (constructor == NULL)
		//program.Push(TOp(TOpcode::PUSH_COUNT,type.GetClass()->GetSize()),before_params);
	{
		if (GetSyntax()->is_static)
		{
			assert(false);
			if (constructor != NULL)
			{
				assert(false);
				ValidateAccess(GetSyntax(), owner, constructor);
				//if(need_testandget)
				//{
				//	program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,offset),before_params);
				//	after_params+=constructor->BuildCall(program,params_result).GetOps();
				//	/*if(constructor->GetType()==MT_INTERNAL)
				//	program.Push(TOp(TOpcode::ASSIGN,(char)false,type.GetClass()->GetSize()),ops_array);*/
				//}else
				//{
				//	program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,offset),program.static_vars_init);
				//	program.static_vars_init+=constructor->BuildCall(program,params_result).GetOps();
				//	/*if(constructor->GetType()==MT_INTERNAL)
				//	program.Push(TOp(TOpcode::ASSIGN,(char)false,type.GetClass()->GetSize()),program.static_vars_constr_last_op);*/
				//}
			}
			TSMethod* destructor = type.GetClass()->GetDestructor();
			if (destructor != NULL)
			{
				ValidateAccess(GetSyntax(), owner, destructor);
				//program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,offset),program.static_vars_destroy);
				//program.static_vars_destroy+=destructor->BuildCall(program,params_result).GetOps();
			}
		}
	}
	else if (constructor != NULL)
	{
			std::vector<TOperation*> param_expressions;
			for (const std::unique_ptr<TOperation>& v : params)
			{
				param_expressions.push_back(v.get());
			}
			constructor_call = std::unique_ptr<TSExpression_TMethodCall>(new TSExpression_TMethodCall());
			
			TSExpression::TGetLocal* get_local_id = new TSExpression::TGetLocal();
			get_local_id->variable = this;
			constructor_call->left = get_local_id;
			constructor_call->Build(param_expressions, params_result, constructor);
		ValidateAccess(GetSyntax(), owner, constructor);
	}

	if (GetSyntax()->assign_expr != NULL)
	{
		assign_expr = std::unique_ptr<TSExpression>(new TSExpression(owner, method, parent, GetSyntax()->assign_expr.get()));
		assign_expr->Build();
	}
	return TVoid();
}

TSLocalVar::TSLocalVar(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TLocalVar* use_syntax)
	:TSStatement(TStatementType::VarDecl, use_owner, use_method, use_parent, use_syntax)
	, TVariable(TVariableType::Local)
	, type(use_owner, use_syntax->GetVarType())
{

}

TNameId TSLocalVar::GetName()
{
	return GetSyntax()->GetName();
}

TSClass* TSLocalVar::GetClass()
{
	return type.GetClass();
}

void TSLocalVar::Run(std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	local_variables.push_back(TStackValue(false, type.GetClass()));
	assert(GetOffset() == local_variables.size() - 1);//иначе ошибка Build локальных переменных

	if (assign_expr)
		assign_expr->Run(formal_params, result_returned, result, object, local_variables);

	else if (constructor_call)
	{
		TStackValue constr_result;
		//constructor_call->Run(formal_params, constr_result, local_variables[GetOffset()], local_variables);
		constructor_call->Run(formal_params, constr_result, object, local_variables);
	}
}

void TSLocalVar::Destruct()
{

}