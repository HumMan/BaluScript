#include "SStatement.h"

class TSExpression;
class TSClass;
class TSMethod;
class TFormalParam;
class TFormalParamWithConversions;

class TSReturn :public TSStatement
{
	std::shared_ptr<TSExpression> result;
	std::shared_ptr<TFormalParamWithConversions> conversions;
public:
	TSReturn(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TReturn* use_syntax);
	TReturn* GetSyntax()
	{
		return (TReturn*)TSyntaxNode::GetSyntax();
	}
	TNameId GetName();
	TSClass* GetClass();
	bool IsStatic();
	void Build();
	void Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value, int method_base);
};