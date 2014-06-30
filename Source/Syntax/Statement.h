#pragma once

#include <assert.h>
#include "../lexer.h"
#include "../notOptimizedProgram.h"

class TMethod;
class TStatements;
class TClass;

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
};