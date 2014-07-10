#include "SStatement.h"

class TFormalParam;
class TBytecode;

class TSBytecode :public TSStatement
{
public:
	TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TBytecode* use_syntax);
	//TFormalParam Build();
	TBytecode* GetSyntax()
	{
		return (TBytecode*)TSyntaxNode::GetSyntax();
	}
	void Run(std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};