﻿#include "SFor.h"
#include "SExpression.h"
#include "SStatements.h"

#include "../Syntax/For.h"
#include "../Syntax/Statements.h"

TSFor::TSFor(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TFor* use_syntax)
	:TSStatement(TStatementType::For, use_owner, use_method, use_parent, (TStatement*)use_syntax)
{

}

void TSFor::Build(TGlobalBuildContext build_context)
{
	compare = std::unique_ptr<TSExpression>(new TSExpression(owner, method, parent, GetSyntax()->compare.get()));
	compare->Build(build_context);
	TExpressionResult compare_result = compare->GetFormalParameter();
	TestBoolExpr(compare_result,compare_conversion);

	increment = std::unique_ptr<TSStatements>(new TSStatements(owner, method, parent, GetSyntax()->increment.get()));
	increment->Build(build_context);
	statements = std::unique_ptr<TSStatements>(new TSStatements(owner, method, parent, GetSyntax()->statements.get()));
	statements->Build(build_context);
}

void TSFor::Run(TStatementRunContext run_context)
{
	TStackValue compare_result;
	while (true)
	{
		compare->Run(TExpressionRunContext(run_context, &compare_result));
		compare_conversion->RunConversion(*run_context.static_fields, compare_result);
		if (*(bool*)compare_result.get())
		{

			statements->Run(run_context);
			if (*run_context.result_returned)
				break;
			increment->Run(run_context);
			if (*run_context.result_returned)
				break;
		}
		else break;
	}
}