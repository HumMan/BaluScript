
#include "SStatement.h"

#include "BuildContext.h"
#include "RunContext.h"

class TSClass;
class TSMethod;
class TSStatements;
class TSExpression;

class TSIf :public TSStatement
{
	std::unique_ptr<TSExpression> bool_expr;
	std::unique_ptr<TActualParamWithConversion> bool_expr_conversion;
	std::unique_ptr<TSStatements> statements, else_statements;
public:
	TSIf(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IIf* use_syntax);
	void Build(TGlobalBuildContext build_context);
	SyntaxApi::IIf* GetSyntax();
	void Run(TStatementRunContext run_context);
};