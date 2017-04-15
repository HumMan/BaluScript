﻿#include "SWhile.h"
#include "SExpression.h"
#include "SStatements.h"

#include "../Syntax/Expression.h"
#include "../Syntax/Statements.h"
#include "../Syntax/While.h"

TSWhile::TSWhile(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IWhile* use_syntax)
	:TSStatement(SyntaxApi::TStatementType::For, use_owner, use_method, use_parent, dynamic_cast<SyntaxApi::IStatement*>(use_syntax))
{

}

void TSWhile::Build(TGlobalBuildContext build_context)
{
	auto syntax = dynamic_cast<SyntaxApi::IWhile*>(GetSyntax());
	auto syntax2 = dynamic_cast<SyntaxInternal::TWhile*>(syntax);
	compare = std::unique_ptr<TSExpression>(new TSExpression(GetOwner(), GetMethod(), GetParentStatements(), syntax2->GetCompare()));
	compare->Build(build_context);
	TExpressionResult compare_result = compare->GetFormalParameter();
	TestBoolExpr(compare_result, compare_conversion);

	statements = std::unique_ptr<TSStatements>(new TSStatements(GetOwner(), GetMethod(), GetParentStatements(), syntax2->GetStatements()));
	statements->Build(build_context);
}

void TSWhile::Run(TStatementRunContext run_context)
{
	TStackValue compare_result;
	while (true)
	{
		TExpressionRunContext while_run_context(run_context, &compare_result);
		compare->Run(while_run_context);
		compare_conversion->RunConversion(*run_context.static_fields, compare_result);
		if (*(bool*)compare_result.get())
		{
			statements->Run(run_context);
			if (*run_context.result_returned)
				break;
		}
		else break;
	}
}