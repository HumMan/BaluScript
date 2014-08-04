
#include "SStatement.h"

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
	void Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	TWhile* GetSyntax()
	{
		return (TWhile*)TSyntaxNode::GetSyntax();
	}
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};