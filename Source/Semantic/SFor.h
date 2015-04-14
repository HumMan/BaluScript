
#include "SStatement.h"

#include "BuildContext.h"
#include "RunContext.h"

class TSClass;
class TSMethod;
class TSStatements;
class TSExpression;
class TExpressionResult;
class TSClassField;
class TSLocalVar;

class TSFor :public TSStatement
{
	std::unique_ptr<TSExpression> compare;
	std::unique_ptr<TActualParamWithConversion> compare_conversion;
	std::unique_ptr<TSStatements> increment;
	std::unique_ptr<TSStatements> statements;
public:
	TSFor(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TFor* use_syntax);
	void Build(TGlobalBuildContext build_context);
	TFor* GetSyntax()
	{
		return (TFor*)TSyntaxNode::GetSyntax();
	}
	void Run(TStatementRunContext run_context);
};