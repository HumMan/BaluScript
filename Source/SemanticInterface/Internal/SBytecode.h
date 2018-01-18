#include "SStatement.h"

class TSBytecode :public TSStatement, public SemanticApi::ISBytecode
{
	std::vector<TSClass*> array_element_classes;
public:
	TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IBytecode* use_syntax);
	SyntaxApi::IBytecode* GetSyntax()const;
	//void Run(TStatementRunContext run_context);
	void Build(SemanticApi::TGlobalBuildContext build_context);
	const std::vector<SyntaxApi::TBytecodeOp>& GetBytecode()const;

	void Accept(SemanticApi::ISStatementVisitor* visitor);
};