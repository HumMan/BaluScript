#include "SReturn.h"

#include "../Syntax/LocalVar.h"
#include "../Syntax/Return.h"
#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"

#include "../semanticAnalyzer.h"

#include "FormalParam.h"
#include "SMethod.h"
#include "SExpression.h"
#include "SParameter.h"
#include "SClass.h"

void TSReturn::Build(TGlobalBuildContext build_context)
{
	result = std::unique_ptr<TSExpression>(new TSExpression(owner, method, parent, &GetSyntax()->result));
	result->Build(build_context);
	TExpressionResult result_result = result->GetFormalParameter();
	if(method->GetRetClass()!=NULL)
	{
		int conv_needed;
		if(result_result.IsVoid())
			GetSyntax()->Error("После оператора return должно следовать какое-то выражение!");
		if(!IsEqualClasses(result_result,TFormalParameter(method->GetRetClass(),method->GetSyntax()->IsReturnRef()),conv_needed))
			GetSyntax()->Error("Выражение невозможно преобразовать в тип возвращаемого значения!");
		conversions.reset(new TActualParamWithConversion());
		conversions->BuildConvert(result_result, TFormalParameter(method->GetRetClass(), method->GetSyntax()->IsReturnRef()));
	}
	else 
		if(result_result.GetClass()!=NULL)
			GetSyntax()->Error("Метод не должен ничего возвращать!");

}

TSReturn::TSReturn(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TReturn* use_syntax)
	:TSStatement(TStatementType::Return, use_owner, use_method, use_parent, (TStatement*)use_syntax)
{}

void TSReturn::Run(TStatementRunContext run_context)
{
	TStackValue return_value;
	auto return_run_context = run_context;
	return_run_context.result = &return_value;
	result->Run(return_run_context);
	*run_context.result_returned = true;
	conversions->RunConversion(*run_context.static_fields, return_value);
	*run_context.result = return_value;
}