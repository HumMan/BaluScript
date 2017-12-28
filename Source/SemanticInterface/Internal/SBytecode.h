#include "SStatement.h"

#include "BuildContext.h"

class TSBytecode :public TSStatement
{
	std::vector<TSClass*> array_element_classes;
public:
	TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IBytecode* use_syntax);
	SyntaxApi::IBytecode* GetSyntax();
	//void Run(TStatementRunContext run_context);
	void Build(TGlobalBuildContext build_context);
};