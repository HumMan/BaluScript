#include "SFor.h"
#include "SExpression.h"
#include "SStatements.h"
#include "../Syntax/For.h"

TSFor::TSFor(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TFor* use_syntax)
	:TSStatement(TStatementType::For, use_owner, use_method, use_parent, (TStatement*)use_syntax)
{

}

void TSFor::Build()
{
	compare = std::unique_ptr<TSExpression>(new TSExpression(owner, method, parent, GetSyntax()->compare.get()));
	compare->Build();
	TFormalParam compare_result = compare->GetFormalParam();
	TestBoolExpr(compare_result,compare_conversion);

	increment = std::unique_ptr<TSStatements>(new TSStatements(owner, method, parent, GetSyntax()->increment.get()));
	increment->Build();
	statements = std::unique_ptr<TSStatements>(new TSStatements(owner, method, parent, GetSyntax()->statements.get()));
	statements->Build();
}

void TSFor::Run(std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	TStackValue compare_result;
	while (true)
	{
		compare->Run(formal_params, result_returned, compare_result, object, local_variables);
		compare_conversion->RunConversion(compare_result);
		if ((bool*)compare_result.get())
		{

			statements->Run(formal_params, result_returned, result, object, local_variables);
			if (result_returned)
				break;
			increment->Run(formal_params, result_returned, result, object, local_variables);
			if (result_returned)
				break;
		}
		else break;
	}
}