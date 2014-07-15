#pragma once

#include <assert.h>
#include "../lexer.h"
#include "../notOptimizedProgram.h"

class TMethod;
class TStatements;
class TClass;
class TStatementVisitor;

namespace TStatementType
{
	enum Enum
	{
		VarDecl,
		For,
		If,
		While,
		Return,
		Expression,
		Bytecode,
		Statements
	};
}

class TStatement:public TTokenPos
{
protected:
	TMethod* method;
	TStatements* parent;
	TClass* owner;
	int stmt_id;//порядковый номер блока в родительском блоке
	TStatementType::Enum stmt_type;
public:
	void SetStmtId(int use_id);
	TStatement(TStatementType::Enum use_stmt_type, TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	TStatementType::Enum GetType();
	virtual void Accept(TStatementVisitor* visitor) = 0;
	virtual ~TStatement()
	{
	}
};

class TExpression;
class TFor;
class TIf;
class TLocalVar;
class TReturn;
class TBytecode;
class TStatements;
class TWhile;

class TStatementVisitor
{
public:
	virtual void Visit(TBytecode* op) = 0;
	virtual void Visit(TExpression* op) = 0;
	virtual void Visit(TFor* op) = 0;
	virtual void Visit(TIf* op) = 0;
	virtual void Visit(TLocalVar* op) = 0;
	virtual void Visit(TReturn* op) = 0;
	virtual void Visit(TStatements* op) = 0;
	virtual void Visit(TWhile* op) = 0;
};