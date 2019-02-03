#include "SStatement.h"

class TSBytecode :public TSStatement, public SemanticApi::ISBytecode
{
public:
	TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IBytecode* use_syntax);
	SyntaxApi::IBytecode* GetSyntax()const;
	//void Run(TStatementRunContext run_context);
	void Build(SemanticApi::TGlobalBuildContext build_context);
	const std::vector<TOpcode::Enum>& GetBytecode()const;

	void Accept(SemanticApi::ISStatementVisitor* visitor);
};