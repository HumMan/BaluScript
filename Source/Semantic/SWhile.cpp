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

void TSWhile::Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	compare = std::unique_ptr<TSExpression>(new TSExpression(owner, method, parent, GetSyntax()->compare.get()));
	compare->Build(static_fields, static_variables);
	TFormalParam compare_result = compare->GetFormalParameter();
	TestBoolExpr(compare_result, compare_conversion);

	statements = std::unique_ptr<TSStatements>(new TSStatements(owner, method, parent, GetSyntax()->statements.get()));
	statements->Build(static_fields, static_variables);
}

void TSWhile::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	TStackValue compare_result;
	while (true)
	{
		compare->Run(static_fields, formal_params, result_returned, compare_result, object, local_variables);
		compare_conversion->RunConversion(static_fields, compare_result);
		if (*(bool*)compare_result.get())
		{
			statements->Run(static_fields, formal_params, result_returned, result, object, local_variables);
			if (result_returned)
				break;
		}
		else break;
	}
}