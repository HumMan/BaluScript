﻿#include "SStatements.h"

#include "SLocalVar.h"
#include "SMethod.h"
#include "FormalParam.h"
#include "SExpression.h"
#include "SBytecode.h"
#include "SReturn.h"
#include "SWhile.h"
#include "SIf.h"
#include "SFor.h"
#include "SClass.h"

#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"

class TSStatementBuilder :public TStatementVisitor
{
	TSStatement* return_new_operation;
	TSClass* owner;
	TSMethod* method;
	TSStatements* parent;
	TGlobalBuildContext build_context;
public:
	TSStatement* GetResult()
	{
		return return_new_operation;
	}
	TSStatementBuilder(TGlobalBuildContext build_context, TSClass* owner, TSMethod* method, TSStatements* parent)
	{
		this->owner = owner;
		this->method = method;
		this->parent = parent;
		this->build_context = build_context;
	}
	void Visit(TExpression* op)
	{
		TSExpression* new_node = new TSExpression(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(TLocalVar* op)
	{
		TSLocalVar* new_node = new TSLocalVar(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(TReturn* op)
	{
		TSReturn* new_node = new TSReturn(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(TStatements* op)
	{
		TSStatements* new_node = new TSStatements(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(TBytecode* op)
	{
		TSBytecode* new_node = new TSBytecode(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(TWhile* op)
	{
		TSWhile* new_node = new TSWhile(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(TFor* op)
	{
		TSFor* new_node = new TSFor(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(TIf* op)
	{
		TSIf* new_node = new TSIf(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
};

TSStatements::TSStatements(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TStatements* use_syntax)
	:TSStatement(TStatementType::Statements,use_owner,use_method,use_parent,use_syntax)
{

}
void TSStatements::AddVar(TSLocalVar* var, int stmt_id)
{
	var_declarations.push_back(TVarDecl(stmt_id, var));
	//для статических локальных переменных смещение будет задано при инициализации всех статических переменных
	if (!var->IsStatic())
		var_declarations.back().pointer->SetOffset(GetLastVariableOffset());//на единицу не увеличиваем, т.к. переменная уже добавлена
}

int TSStatements::GetLastVariableOffset()
{
	if (parent != NULL)
	{
		return parent->GetLastVariableOffset() + var_declarations.size();
	}
	else
	{
		return var_declarations.size() - 1;
	}
}

void TSStatements::Build(TGlobalBuildContext build_context)
{
	for (const std::unique_ptr<TStatement>& st : ((TStatements*)GetSyntax())->statements)
	{
		TSStatementBuilder b(build_context, owner, method, this);
		st->Accept(&b);
		statements.push_back(std::unique_ptr<TSStatement>(b.GetResult()));
	}
}

TVariable* TSStatements::GetVar(TNameId name, int sender_id)
{
	for (size_t i = 0; i < var_declarations.size(); i++)
	{
		if (var_declarations[i].stmt_id <= sender_id
			&&var_declarations[i].pointer->GetName() == name)
			return var_declarations[i].pointer;
	}
	//for(int i=0;i<=statement.GetHigh();i++)
	//	//TODO из-за этого перебора тормоза при большом количестве операторных скобок + невозможность искать локальную переменную за не линейную скорость
	//{
	//	if(except_this&&statement[i]==sender)break;
	//	if(statement[i]->GetType()==TStatementType::VarDecl&&((TLocalVar*)(statement[i]))->GetName()==name)
	//		return ((TLocalVar*)(statement[i]));
	//	if(statement[i]==sender)break;
	//}
	if (parent != NULL)return parent->GetVar(name, GetSyntax()->stmt_id);
	else if (method != NULL)return  method->GetVar(name);
	else return NULL;
}

void TSStatements::Run(TStatementRunContext run_context)
{
	for (const std::unique_ptr<TSStatement>& statement : statements)
	{
		statement->Run(run_context);
		if (*run_context.result_returned)
			break;
	}
	//for (TSStatements::TVarDecl& var_decl : var_declarations)
	for (int i = var_declarations.size() - 1; i >= 0;i--)
	{
		auto& var_decl = var_declarations[i];
		var_decl.pointer->Destruct(*run_context.static_fields,*run_context.local_variables);
		if(!var_decl.pointer->IsStatic())
			run_context.local_variables->pop_back();
	}
}