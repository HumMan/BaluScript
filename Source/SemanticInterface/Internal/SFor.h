
#include "SStatement.h"

class TSClass;
class TSMethod;
class TSStatements;
class TSExpression;

class TSFor :public TSStatement, public SemanticApi::ISFor
{
	std::unique_ptr<TSExpression> compare;
	std::unique_ptr<TActualParamWithConversion> compare_conversion;
	std::unique_ptr<TSStatements> increment;
	std::unique_ptr<TSStatements> statements;
public:
	TSFor(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IFor* use_syntax);
	void Build(SemanticApi::TGlobalBuildContext build_context);
	SyntaxApi::IFor* GetSyntax();
	//void Run(TStatementRunContext run_context);
	SemanticApi::ISOperations::ISExpression* GetCompare()const;
	SemanticApi::IActualParamWithConversion* GetCompareConversion()const;
	SemanticApi::ISStatements* GetStatements()const;
	SemanticApi::ISStatements* GetIncrement()const;
	void Accept(SemanticApi::ISStatementVisitor* visitor);
};