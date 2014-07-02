#pragma once

#include "../lexer.h"
#include "Statement.h"


class TLocalVar;
class TClass;
class TVariable;

class TStatements:public TStatement
{
	friend class TSStatements;
	std::vector<std::shared_ptr<TStatement>> statements;
public:
	void Add(TStatement* use_statement);
	TStatement* GetStatement(int i);
	TStatements(const TStatements& use_source);
	int GetHigh();
	TStatements(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeStatement(TLexer& source,bool end_semicolon);
	void AnalyzeSyntax(TLexer& source);
};
