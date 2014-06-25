#pragma once

#include <baluLib.h>

#include "../lexer.h"
#include "Statement.h"

#include "../syntaxAnalyzer.h"


class TLocalVar;
class TClass;
class TStatements;
class TVariable;

class TStatements:public TStatement
{
	struct TVarDecl
	{
		int stmt_id;
		TLocalVar* pointer;
		TVarDecl(){}
		TVarDecl(int use_stmt_id,TLocalVar* use_pointer):stmt_id(use_stmt_id),pointer(use_pointer){}
	};
	int curr_local_var_offset;
	int last_locals_offset;
	TVector<TStatement*> statement;
	TVector<TVarDecl> var_declarations;
	void operator=(const TStatements& use_source);
public:
	void Add(TStatement* use_statement);
	void AddVar(TLocalVar* use_var);
	TStatement* GetStatement(int i);
	TStatement* GetCopy();
	TStatements(const TStatements& use_source);
	void InitOwner(TClass* use_owner, TMethod* use_method, TStatements* use_parent);
	int GetHigh();
	TStatements(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	~TStatements();
	void AnalyzeStatement(TLexer& source,bool end_semicolon);
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TVariable* GetVar(TNameId name,int sender_id);
	TOpArray BuildLocalsAndParamsDestructor(TNotOptimizedProgram &program,int &deallocate_size);
	int BuildCurrLocalsDestructor(TOpArray& ops_array,TNotOptimizedProgram &program,bool deallocate_stack);
};
