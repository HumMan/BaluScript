
#include "SStatement.h"

#include "BuildContext.h"
#include "RunContext.h"

class TSClass;
class TSMethod;
class TSStatements;
class TSExpression;
class TSClassField;
class TSLocalVar;

class TSWhile :public TSStatement
{
	std::unique_ptr<TSExpression> compare;
	std::unique_ptr<TActualParamWithConversion> compare_conversion;
	std::unique_ptr<TSStatements> statements;
public:
	TSWhile(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TWhile* use_syntax);
	void Build(TGlobalBuildContext build_context);
	TWhile* GetSyntax()
	{
		return (TWhile*)TSyntaxNode::GetSyntax();
	}
	void Run(TStatementRunContext run_context);
};