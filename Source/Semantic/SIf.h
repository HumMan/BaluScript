
#include "SStatement.h"

class TSClass;
class TSMethod;
class TSStatements;
class TSExpression;

class TSIf :public TSStatement
{
	std::unique_ptr<TSExpression> bool_expr;
	std::unique_ptr<TFormalParamWithConversions> bool_expr_conversion;
	std::unique_ptr<TSStatements> statements, else_statements;
public:
	TSIf(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TIf* use_syntax);
	void Build();
	TIf* GetSyntax()
	{
		return (TIf*)TSyntaxNode::GetSyntax();
	}
	void Run(std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};