#include "SIf.h"

#include "SExpression.h"
#include "SStatements.h"

#include "../Syntax/If.h"
#include "../Syntax/Statements.h"

TSIf::TSIf(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TIf* use_syntax)
	:TSStatement(TStatementType::For, use_owner, use_method, use_parent, (TStatement*)use_syntax)
{

}

void TSIf::Build(TGlobalBuildContext build_context)
{
	bool_expr = std::unique_ptr<TSExpression>(new TSExpression(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetBoolExpr()));
	bool_expr->Build(build_context);
	TExpressionResult compare_result = bool_expr->GetFormalParameter();
	TestBoolExpr(compare_result, bool_expr_conversion);

	statements = std::unique_ptr<TSStatements>(new TSStatements(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetStatements()));
	statements->Build(build_context);
	else_statements = std::unique_ptr<TSStatements>(new TSStatements(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetElseStatements()));
	else_statements->Build(build_context);
}

void TSIf::Run(TStatementRunContext run_context)
{
	TStackValue compare_result;



	bool_expr->Run(TExpressionRunContext(run_context, &compare_result));
	bool_expr_conversion->RunConversion(*run_context.static_fields, compare_result);
	if (*(bool*)compare_result.get())
	{
		statements->Run(run_context);
		if (*run_context.result_returned)
			return;
	}
	else
	{
		else_statements->Run(run_context);
		if (*run_context.result_returned)
			return;
	}
}