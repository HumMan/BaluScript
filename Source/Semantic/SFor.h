
#include "SStatement.h"

class TSClass;
class TSMethod;
class TSStatements;
class TSExpression;
class TFormalParam;
class TSClassField;
class TSLocalVar;

class TSFor :public TSStatement
{
	std::unique_ptr<TSExpression> compare;
	std::unique_ptr<TFormalParamWithConversions> compare_conversion;
	std::unique_ptr<TSStatements> increment;
	std::unique_ptr<TSStatements> statements;
public:
	TSFor(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TFor* use_syntax);
	void Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	TFor* GetSyntax()
	{
		return (TFor*)TSyntaxNode::GetSyntax();
	}
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};