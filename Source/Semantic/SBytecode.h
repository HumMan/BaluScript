#include "SStatement.h"

class TFormalParam;
class TBytecode;
class TSClassField;
class TSLocalVar;

class TSBytecode :public TSStatement
{
	std::vector<TSClass*> array_element_classes;
public:
	TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TBytecode* use_syntax);
	//TFormalParam Build();
	TBytecode* GetSyntax()
	{
		return (TBytecode*)TSyntaxNode::GetSyntax();
	}
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	void Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
};