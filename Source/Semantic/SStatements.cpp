#include "SStatements.h"

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

class TSStatementBuilder :public SyntaxApi::IStatementVisitor
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
	void Visit(SyntaxApi::IExpression* op)
	{
		TSExpression* new_node = new TSExpression(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(SyntaxApi::ILocalVar* op)
	{
		TSLocalVar* new_node = new TSLocalVar(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(SyntaxApi::IReturn* op)
	{
		TSReturn* new_node = new TSReturn(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(SyntaxApi::IStatements* op)
	{
		TSStatements* new_node = new TSStatements(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(SyntaxApi::IBytecode* op)
	{
		TSBytecode* new_node = new TSBytecode(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(SyntaxApi::IWhile* op)
	{
		TSWhile* new_node = new TSWhile(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(SyntaxApi::IFor* op)
	{
		TSFor* new_node = new TSFor(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
	void Visit(SyntaxApi::IIf* op)
	{
		TSIf* new_node = new TSIf(owner, method, parent, op);
		new_node->Build(build_context);
		return_new_operation = new_node;
	}
};

TSStatements::TSStatements(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IStatements* use_syntax)
	:TSStatement(SyntaxApi::TStatementType::Statements, use_owner, use_method, use_parent, use_syntax)
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
	if (GetParentStatements() != NULL)
	{
		return GetParentStatements()->GetLastVariableOffset() + var_declarations.size();
	}
	else
	{
		return var_declarations.size() - 1;
	}
}

void TSStatements::Build(TGlobalBuildContext build_context)
{
	auto syntax = dynamic_cast<SyntaxApi::IStatements*>(GetSyntax());
	for (int i = 0; i < syntax->GetStatementsCount(); i++)
	{
		SyntaxApi::IStatement* st = syntax->GetStatement(i);
		TSStatementBuilder b(build_context, GetOwner(), GetMethod(), this);
		st->Accept(&b);
		statements.push_back(std::unique_ptr<TSStatement>(b.GetResult()));
	}
}

TVariable* TSStatements::GetVar(Lexer::TNameId name, int sender_id)
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
	if (GetParentStatements() != NULL)
		return GetParentStatements()->GetVar(name, GetSyntax()->GetStmtId());
	else if (GetMethod() != NULL)
		return  GetMethod()->GetVar(name);
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