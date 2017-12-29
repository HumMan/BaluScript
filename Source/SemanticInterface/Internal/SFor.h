
#include "SStatement.h"

class TSClass;
class TSMethod;
class TSStatements;
class TSExpression;

class TSFor :public TSStatement
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
};