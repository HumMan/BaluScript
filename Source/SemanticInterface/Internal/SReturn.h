#include "SStatement.h"

class TSExpression;
class TSClass;
class TSMethod;
class TActualParamWithConversion;

class TSReturn :public TSStatement, public SemanticApi::ISReturn
{
	std::unique_ptr<TSExpression> result;
	std::unique_ptr<TActualParamWithConversion> conversions;
public:
	TSReturn(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IReturn* use_syntax);
	SyntaxApi::IReturn* GetSyntax();
	void Build(SemanticApi::TGlobalBuildContext build_context);

	SemanticApi::ISOperations::ISExpression* GetResult()const;
	SemanticApi::IActualParamWithConversion* GetConverstion()const;
	void Accept(SemanticApi::ISStatementVisitor* visitor);
};