#include "SLocalVar.h"

#include "SMethod.h"
#include "SSyntaxNode.h"
#include "../Syntax/LocalVar.h"
#include "SStatements.h"
#include "../semanticAnalyzer.h"
#include "FormalParam.h"
#include "SExpression.h"
#include "../Syntax/Expression.h"

TFormalParam TSLocalVar::Build()
{
	std::vector<TSMethod*> methods;
	if(owner->GetMethods(methods,GetSyntax()->name))
		GetSyntax()->Error("Метод не может быть именем переменной!");
	if (owner->GetClass(GetSyntax()->name) != NULL)
		GetSyntax()->Error("Класс не может быть именем переменной!");
	TVariable* t = parent->GetVar(GetSyntax()->name, GetSyntax()->stmt_id);
	if(t!=NULL&&t!=this)
	{
		switch(t->GetType())
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
	}else
	{
	}

	type.Link();

	parent->AddVar(this,GetSyntax()->stmt_id);

	for (std::shared_ptr<TExpression> param_syntax : GetSyntax()->params)
	{
		params.push_back(std::shared_ptr<TSExpression>(new TSExpression(owner, method, parent, param_syntax.get())));
		params.back()->Build();
	}

	if (GetSyntax()->assign_expr)
	{
		assign_expr = std::shared_ptr<TSExpression>(new TSExpression(owner, method, parent, GetSyntax()->assign_expr.get()));
		assign_expr->Build();
	}

	int conv_need=0;
	std::vector<TSMethod*> constructors;
	std::vector<TFormalParam> params_result;

	type.GetClass()->GetConstructors(constructors);
	TSMethod* constructor=FindMethod(GetSyntax(),constructors,params_result,conv_need);
	if(constructor==NULL&&params_result.size()>0)
		GetSyntax()->Error("Коструктора с такими парметрами не существует!");

	bool need_testandget=GetSyntax()->is_static&&(params_result.size()>0||assign_expr!=NULL);

	if ((constructor == NULL || !constructor->GetSyntax()->IsBytecode()) && !GetSyntax()->is_static)
		//program.Push(TOp(TOpcode::PUSH_COUNT,type.GetClass()->GetSize()),before_params);

		if (GetSyntax()->is_static)
	{
		if(constructor!=NULL)
		{
			ValidateAccess(GetSyntax(),owner,constructor);
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
		TSMethod* destructor=type.GetClass()->GetDestructor();
		if(destructor!=NULL)
		{
			ValidateAccess(GetSyntax(), owner, destructor);
			//program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,offset),program.static_vars_destroy);
			//program.static_vars_destroy+=destructor->BuildCall(program,params_result).GetOps();
		}
	}else if(constructor!=NULL)
	{
		if(!constructor->GetSyntax()->IsBytecode())
		{
			//program.Push(TOp(TOpcode::PUSH_STACK_HIGH_REF,type.GetClass()->GetSize()),before_params);
		}
		ValidateAccess(GetSyntax(), owner, constructor);
		//after_params+=constructor->BuildCall(program,params_result).GetOps();
	}

	if (GetSyntax()->assign_expr != NULL)
	{
		assign_expr = std::shared_ptr<TSExpression>(new TSExpression(owner, method, parent, GetSyntax()->assign_expr.get()));
		assign_expr->Build();
		//assign_expr->Build(program, local_var_offset).GetOps();
	}
	return TVoid();
}

TSLocalVar::TSLocalVar(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TLocalVar* use_syntax) 
	:TSStatement(TStatementType::VarDecl,use_owner,use_method,use_parent,use_syntax)
	, TVariable(TVariableType::Local)
	, type(use_owner,use_syntax->GetVarType())
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

void TSLocalVar::Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value)
{

}

void TSLocalVar::Destruct()
{

}