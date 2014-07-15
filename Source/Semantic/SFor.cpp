#include "SFor.h"
#include "SExpression.h"
#include "SStatements.h"

TSFor::TSFor(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TFor* use_syntax)
	:TSStatement(TStatementType::For, use_owner, use_method, use_parent, (TStatement*)use_syntax)
{

}

void TSFor::Build()
{

}

void TSFor::Run(std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{

}