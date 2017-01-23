#pragma once

#include <memory>

#include "../lexer.h"
#include "Statement.h"


class TLocalVar;
class TClass;
class TVariable;

class TStatements:public TStatement
{
	std::vector<std::unique_ptr<TStatement>> statements;
public:
	void Add(TStatement* use_statement);
	void AddVar(TLocalVar* use_var);
	TStatement* GetStatement(int i);
	int GetStatementsCount()const;
	TStatements(const TStatements& use_source);
	int GetHigh();
	TStatements(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void AnalyzeStatement(Lexer::ILexer* source,bool end_semicolon);
	void AnalyzeSyntax(Lexer::ILexer* source);
	void Accept(TStatementVisitor* visitor);
};
