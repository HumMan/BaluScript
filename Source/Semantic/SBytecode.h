#include "SStatement.h"

#include "BuildContext.h"
#include "RunContext.h"

class TExpressionResult;
class TBytecode;
class TSClassField;
class TSLocalVar;

class TSBytecode :public TSStatement
{
	std::vector<TSClass*> array_element_classes;
public:
	TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TBytecode* use_syntax);
	//TExpressionResult Build();
	TBytecode* GetSyntax()
	{
		return (TBytecode*)TSyntaxNode::GetSyntax();
	}
	void Run(TStatementRunContext run_context);
	void Build(TGlobalBuildContext build_context);
};