#pragma once

#include <assert.h>
#include "../lexer.h"
#include "FormalParam.h"
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
	void SetStmtId(int use_id){
		stmt_id=use_id;
	}
	TStatement(TStatementType::Enum use_stmt_type,TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id):
		method(use_method),parent(use_parent),owner(use_owner),
		stmt_id(use_stmt_id),stmt_type(use_stmt_type){}
	virtual ~TStatement(){}
	TStatementType::Enum GetType(){
		return stmt_type;
	}
	virtual TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset)=0;
	void _InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		owner=use_owner;
		method=use_method;
		parent=use_parent;
	}
	virtual void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)=0;
	/*{
		owner=use_owner;
		method=use_method;
		parent=use_parent;
	}*/
	virtual TOpArray BuildLocalsAndParamsDestructor(TNotOptimizedProgram &program,int &deallocate_size);
	virtual int BuildCurrLocalsDestructor(TOpArray& ops_array,TNotOptimizedProgram &program,bool deallocate_stack)
	{
		assert(false);
		return -1;
	}
	void TestBoolExpr(TNotOptimizedProgram &program,TFormalParam& compare_result,int go_false_label);
	virtual TStatement* GetCopy()=0;
};