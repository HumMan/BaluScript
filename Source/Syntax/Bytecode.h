#pragma once

#include <baluLib.h>

#include "../VirtualMachine/Op.h"

#include "Statement.h"

class TClass;
class TMethod;
class TStatements;

class TBytecode :public TStatement
{
protected:
	struct TBytecodeOp
	{
		enum TOpParamType
		{
			VALUE,
			GET_ARR_ELEMENT_CLASS_ID
		};
		TOpParamType f[2];
		TNameId id[2];
		TOp op;
		TBytecodeOp()
		{
			f[0] = VALUE;
			f[1] = VALUE;
		}
	};
	std::vector<TBytecodeOp> code;
	void operator=(const TBytecode& use_source);
public:
	TBytecode(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void Accept(TStatementVisitor* visitor);
	void AnalyzeSyntax(TLexer& source);
};