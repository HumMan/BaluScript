#include "SWhile.h"
#include "SExpression.h"
#include "SStatements.h"

TSWhile::TSWhile(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IWhile* use_syntax)
	:TSStatement(SyntaxApi::TStatementType::For, use_owner, use_method, use_parent, dynamic_cast<SyntaxApi::IStatement*>(use_syntax))
{

}

void TSWhile::Build(SemanticApi::TGlobalBuildContext build_context)
{
	auto syntax = dynamic_cast<SyntaxApi::IWhile*>(GetSyntax());
	compare = std::unique_ptr<TSExpression>(new TSExpression(GetOwner(), GetMethod(), GetParentStatements(), syntax->GetCompare()));
	compare->Build(build_context);
	TExpressionResult compare_result = compare->GetFormalParameter();
	TestBoolExpr(compare_result, compare_conversion);

	statements = std::unique_ptr<TSStatements>(new TSStatements(GetOwner(), GetMethod(), GetParentStatements(), syntax->GetStatements()));
	statements->Build(build_context);
}

SemanticApi::ISOperations::ISExpression * TSWhile::GetCompare() const
{
	return compare.get();
}

SemanticApi::IActualParamWithConversion * TSWhile::GetCompareConversion() const
{
	return compare_conversion.get();
}

SemanticApi::ISStatements * TSWhile::GetStatements() const
{
	return statements.get();
}

void TSWhile::Accept(SemanticApi::ISStatementVisitor * visitor)
{
	visitor->Visit(this);
}
