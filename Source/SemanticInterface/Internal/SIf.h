
#include "SStatement.h"

class TSClass;
class TSMethod;
class TSStatements;
class TSExpression;

class TSIf :public TSStatement, public SemanticApi::ISIf
{
	std::unique_ptr<TSExpression> bool_expr;
	std::unique_ptr<TActualParamWithConversion> bool_expr_conversion;
	std::unique_ptr<TSStatements> statements, else_statements;
public:
	TSIf(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IIf* use_syntax);
	void Build(SemanticApi::TGlobalBuildContext build_context);
	SyntaxApi::IIf* GetSyntax();
	
	SemanticApi::ISOperations::ISExpression* GetBoolExpr()const;
	SemanticApi::IActualParamWithConversion* GetBoolExprConversion()const;
	SemanticApi::ISStatements* GetStatements()const;
	SemanticApi::ISStatements* GetElseStatements()const;
	void Accept(SemanticApi::ISStatementVisitor* visitor);
};