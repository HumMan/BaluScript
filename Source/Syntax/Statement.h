#pragma once

#include <assert.h>
#include "../lexer.h"

class TMethod;
class TStatements;
class TClass;
class TStatementVisitor;

enum class TStatementType
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

class TStatement:public Lexer::TTokenPos
{
private:
	TMethod* method;
	TStatements* parent;
	TClass* owner;
	int stmt_id;//порядковый номер блока в родительском блоке
	TStatementType stmt_type;
public:
	void SetStmtId(int use_id);
	int GetStmtId()
	{
		return stmt_id;
	}
	TStatementType GetStmtType()
	{
		return stmt_type;
	}
	TMethod* GetMethod()
	{
		return method;
	}
	TStatements* GetParent()
	{
		return parent;
	}
	TClass* GetOwner()
	{
		return owner;
	}
	TStatement(TStatementType use_stmt_type, TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	TStatementType GetType();
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