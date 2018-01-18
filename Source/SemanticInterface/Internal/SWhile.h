#pragma once

#include "SStatement.h"

class TSClass;
class TSMethod;
class TSStatements;
class TSExpression;

class TSWhile :public TSStatement, public SemanticApi::ISWhile
{
	std::unique_ptr<TSExpression> compare;
	std::unique_ptr<TActualParamWithConversion> compare_conversion;
	std::unique_ptr<TSStatements> statements;
public:
	TSWhile(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IWhile* use_syntax);
	void Build(SemanticApi::TGlobalBuildContext build_context);
	//void Run(TStatementRunContext run_context);
	SemanticApi::ISOperations::ISExpression* GetCompare()const;
	SemanticApi::IActualParamWithConversion* GetCompareConversion()const;
	SemanticApi::ISStatements* GetStatements()const;
	void Accept(SemanticApi::ISStatementVisitor* visitor);
};