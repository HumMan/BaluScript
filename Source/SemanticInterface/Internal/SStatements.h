#pragma once

#include "../../SyntaxInterface/SyntaxTreeApi.h"

#include "SStatement.h"
#include "Variable.h"

class TSLocalVar;
class TSClass;
class TSMethod;

class TSStatements :public TSStatement, public virtual SemanticApi::ISStatement, public SemanticApi::ISStatements
{	
	std::vector<std::unique_ptr<TSStatement>> statements;
	std::vector<SemanticApi::TVarDecl> var_declarations;
public:
	SyntaxApi::IStatements* GetSyntax()const
	{
		return dynamic_cast<SyntaxApi::IStatements*>(TSyntaxNode::GetSyntax());
	}
	//void Add(TSStatement* use_statement);
	void AddVar(TSLocalVar* use_var,int stmt_id);
	int GetLastVariableOffset();
	//TSStatement* GetStatement(int i);
	//TSStatement* CreateNode(TStatement* use_syntax_node);
	SemanticApi::IVariable* GetVar(Lexer::TNameId name, int sender_id)const;
	TSStatements(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IStatements* use_syntax);
	void Build(SemanticApi::TGlobalBuildContext build_context);
	
	std::vector<SemanticApi::ISStatement*> GetStatements()const;
	std::vector<SemanticApi::TVarDecl> GetVarDeclarations()const;

	void Accept(SemanticApi::ISStatementVisitor* visitor);
};
