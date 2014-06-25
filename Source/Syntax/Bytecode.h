#pragma once

#include <baluLib.h>
#include "../VirtualMachine/Op.h"
#include "Statement.h"
#include "FormalParam.h"
#include "../notOptimizedProgram.h"

class TClass;
class TMethod;
class TStatements;

class TBytecode:public TStatement
{
	
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
			f[0]=VALUE;
			f[1]=VALUE;
		}
	};
	TVector<TBytecodeOp> code;
	void operator=(const TBytecode& use_source);
public:
	TBytecode(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
	}
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TStatement* GetCopy()
	{
		return new TBytecode(*this);
	}
};