#pragma once

#include "../lexer.h"
#include "Statement.h"
#include "../syntaxAnalyzer.h"


class TLocalVar;
class TClass;
class TVariable;

class TStatements:public TStatement
{
	struct TVarDecl
	{
		int stmt_id;
		TLocalVar* pointer;
		TVarDecl(){}
		TVarDecl(int use_stmt_id, TLocalVar* use_pointer) :stmt_id(use_stmt_id), pointer(use_pointer){}
	};
	int curr_local_var_offset;
	int last_locals_offset;
	std::vector<std::shared_ptr<TStatement>> statement;
	std::vector<TVarDecl> var_declarations;
	void operator=(const TStatements& use_source);
public:
	void Add(TStatement* use_statement);
	void AddVar(TLocalVar* use_var);
	TStatement* GetStatement(int i);
	TStatements(const TStatements& use_source);
	int GetHigh();
	TStatements(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeStatement(TLexer& source,bool end_semicolon);
	void AnalyzeSyntax(TLexer& source);
};
