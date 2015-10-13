#pragma once

#include "SStatement.h"
#include "Variable.h"

#include "RunContext.h"
#include "BuildContext.h"

class TSLocalVar;
class TSClass;
class TSMethod;
class TSClassField;

class TSStatements :public TSStatement
{
	struct TVarDecl
	{
		int stmt_id;
		TSLocalVar* pointer;
		TVarDecl(){}
		TVarDecl(int use_stmt_id, TSLocalVar* use_pointer) :stmt_id(use_stmt_id), pointer(use_pointer){}
	};
	std::vector<std::unique_ptr<TSStatement>> statements;
	std::vector<TVarDecl> var_declarations;
	void operator=(const TStatements& use_source);
public:
	TStatements* GetSyntax()
	{
		return (TStatements*)TSyntaxNode::GetSyntax();
	}
	void Add(TSStatement* use_statement);
	void AddVar(TSLocalVar* use_var,int stmt_id);
	int GetLastVariableOffset();
	TSStatement* GetStatement(int i);
	//TSStatement* CreateNode(TStatement* use_syntax_node);
	TVariable* GetVar(TNameId name, int sender_id);
	TSStatements(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TStatements* use_syntax);
	void Build(TGlobalBuildContext build_context);
	//void Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value);
	void Run(TStatementRunContext run_context);
};