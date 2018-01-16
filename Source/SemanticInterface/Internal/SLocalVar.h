#pragma once

#include "SType.h"
#include "SStatement.h"
#include "Variable.h"
#include "SClass.h"

class TSExpression;
class TSConstructObject;

class TSLocalVar :public TSStatement, public TVariable, public TNodeWithOffset,
	public SemanticApi::ISLocalVar
{
	std::unique_ptr<TSExpression> assign_expr;
	std::unique_ptr<TSConstructObject> construct_object;
	TSType type;
public:
	SyntaxApi::ILocalVar* GetSyntax()const;
	TSLocalVar(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::ILocalVar* use_syntax);
	Lexer::TNameId GetName()const;
	TSClass* GetClass()const;
	bool IsStatic()const;
	void Build(SemanticApi::TGlobalBuildContext build_context);
	/*void Run(TStatementRunContext run_context);
	void Destruct(std::vector<TStaticValue> &static_fields, std::vector<TStackValue>& local_variables);*/
};