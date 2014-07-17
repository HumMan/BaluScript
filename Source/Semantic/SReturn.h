#include "SStatement.h"

class TSExpression;
class TSClass;
class TSMethod;
class TFormalParam;
class TFormalParamWithConversions;
class TSClassField;
class TSLocalVar;

class TSReturn :public TSStatement
{
	std::unique_ptr<TSExpression> result;
	std::unique_ptr<TFormalParamWithConversions> conversions;
public:
	TSReturn(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TReturn* use_syntax);
	TReturn* GetSyntax()
	{
		return (TReturn*)TSyntaxNode::GetSyntax();
	}
	TNameId GetName();
	TSClass* GetClass();
	bool IsStatic();
	void Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};