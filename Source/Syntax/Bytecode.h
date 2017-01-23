#pragma once

#include "../VirtualMachine/Op.h"

#include "Statement.h"

class TClass;
class TMethod;
class TStatements;

struct TBytecodeOp
{
	enum TOpParamType
	{
		VALUE,
		GET_ARR_ELEMENT_CLASS_ID
	};
	TOpParamType f[2];
	Lexer::TNameId id[2];
	TOp op;
	TBytecodeOp()
	{
		f[0] = VALUE;
		f[1] = VALUE;
	}
};

class TBytecode :public TStatement
{
private:
	std::vector<TBytecodeOp> code;
	void operator=(const TBytecode& use_source);
public:
	TBytecode(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void Accept(TStatementVisitor* visitor);
	void AnalyzeSyntax(Lexer::ILexer* source);
	const std::vector<TBytecodeOp>& GetBytecode();
};