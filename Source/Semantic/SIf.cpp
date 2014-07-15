#include "SIf.h"
#include "SExpression.h"
#include "SStatements.h"
#include "../Syntax/If.h"

TSIf::TSIf(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TIf* use_syntax)
	:TSStatement(TStatementType::For, use_owner, use_method, use_parent, (TStatement*)use_syntax)
{

}

void TSIf::Build()
{
	bool_expr = std::unique_ptr<TSExpression>(new TSExpression(owner, method, parent, GetSyntax()->bool_expr.get()));
	bool_expr->Build();
	TFormalParam compare_result = bool_expr->GetFormalParam();
	TestBoolExpr(compare_result, bool_expr_conversion);

	statements = std::unique_ptr<TSStatements>(new TSStatements(owner, method, parent, GetSyntax()->statements.get()));
	statements->Build();
	else_statements = std::unique_ptr<TSStatements>(new TSStatements(owner, method, parent, GetSyntax()->else_statements.get()));
	else_statements->Build();
}

void TSIf::Run(std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	TStackValue compare_result;

		bool_expr->Run(formal_params, result_returned, compare_result, object, local_variables);
		bool_expr_conversion->RunConversion(compare_result);
		if ((bool*)compare_result.get())
		{
			statements->Run(formal_params, result_returned, result, object, local_variables);
			if (result_returned)
				return;
		}else
		{
			else_statements->Run(formal_params, result_returned, result, object, local_variables);
			if (result_returned)
				return;
		}
}