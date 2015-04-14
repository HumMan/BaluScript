#include "SStatement.h"

#include "BuildContext.h"
#include "RunContext.h"

class TSExpression;
class TSClass;
class TSMethod;
class TExpressionResult;
class TActualParamWithConversion;
class TSClassField;
class TSLocalVar;

class TSReturn :public TSStatement
{
	std::unique_ptr<TSExpression> result;
	std::unique_ptr<TActualParamWithConversion> conversions;
public:
	TSReturn(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TReturn* use_syntax);
	TReturn* GetSyntax()
	{
		return (TReturn*)TSyntaxNode::GetSyntax();
	}
	TNameId GetName();
	TSClass* GetClass();
	bool IsStatic();
	void Build(TGlobalBuildContext build_context);
	void Run(TStatementRunContext run_context);
};