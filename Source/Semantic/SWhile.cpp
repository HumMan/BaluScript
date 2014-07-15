#include "SWhile.h"
#include "SExpression.h"
#include "SStatements.h"

#include "../Syntax/Expression.h"
#include "../Syntax/Statements.h"
#include "../Syntax/While.h"

TSWhile::TSWhile(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TWhile* use_syntax)
	:TSStatement(TStatementType::For, use_owner, use_method, use_parent, (TStatement*)use_syntax)
{

}

void TSWhile::Build()
{
	compare = std::unique_ptr<TSExpression>(new TSExpression(owner, method, parent, GetSyntax()->compare.get()));
	compare->Build();
	TFormalParam compare_result = compare->GetFormalParam();
	TestBoolExpr(compare_result, compare_conversion);

	statements = std::unique_ptr<TSStatements>(new TSStatements(owner, method, parent, GetSyntax()->statements.get()));
	statements->Build();
}

void TSWhile::Run(std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
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
		}
		else break;
	}
}