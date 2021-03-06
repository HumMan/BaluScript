﻿#include "SFor.h"
#include "SExpression.h"
#include "SStatements.h"

TSFor::TSFor(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IFor* use_syntax)
	:TSStatement(SyntaxApi::TStatementType::For, use_owner, use_method, use_parent, dynamic_cast<SyntaxApi::IStatement*>(use_syntax))
{

}

void TSFor::Build(SemanticApi::TGlobalBuildContext build_context)
{
	compare = std::unique_ptr<TSExpression>(new TSExpression(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetCompare()));
	compare->Build(build_context);
	TExpressionResult compare_result = compare->GetFormalParameter();
	TestBoolExpr(compare_result,compare_conversion);

	increment = std::unique_ptr<TSStatements>(new TSStatements(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetIncrement()));
	increment->Build(build_context);
	statements = std::unique_ptr<TSStatements>(new TSStatements(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetStatements()));
	statements->Build(build_context);
}

SyntaxApi::IFor* TSFor::GetSyntax()
{
	return dynamic_cast<SyntaxApi::IFor*>(TSyntaxNode::GetSyntax());
}

SemanticApi::ISOperations::ISExpression * TSFor::GetCompare() const
{
	return compare.get();
}

SemanticApi::IActualParamWithConversion * TSFor::GetCompareConversion() const
{
	return compare_conversion.get();
}

SemanticApi::ISStatements * TSFor::GetStatements() const
{
	return statements.get();
}

SemanticApi::ISStatements * TSFor::GetIncrement() const
{
	return increment.get();
}

void TSFor::Accept(SemanticApi::ISStatementVisitor * visitor)
{
	visitor->Visit(this);
}

