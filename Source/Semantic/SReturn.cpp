#include "SReturn.h"

#include "FormalParam.h"
#include "SMethod.h"
#include "SExpression.h"
#include "../Syntax/LocalVar.h"
#include "../semanticAnalyzer.h"
#include "../Syntax/Return.h"

void TSReturn::Build()
{
	result = std::shared_ptr<TSExpression>(new TSExpression(owner, method, parent, &GetSyntax()->result));
	result->Build();
	TFormalParam result_result = result->GetFormalParam();
	if(method->GetRetClass()!=NULL)
	{
		int conv_needed;
		if(result_result.IsVoid())
			GetSyntax()->Error("После оператора return должно следовать какое-то выражение!");
		if(!IsEqualClasses(result_result,method->GetRetClass(),method->GetSyntax()->IsReturnRef(),conv_needed))
			GetSyntax()->Error("Выражение невозможно преобразовать в тип возвращаемого значения!");
		conversions = std::shared_ptr<TFormalParamWithConversions>(new TFormalParamWithConversions());
		conversions->BuildConvert(result_result, method->GetRetClass(), method->GetSyntax()->IsReturnRef())
	}else 
		if(result_result.GetClass()!=NULL)
			GetSyntax()->Error("Метод не должен ничего возвращать!");

}

TSReturn::TSReturn(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TReturn* use_syntax)
	:TSStatement(TStatementType::Return, use_owner, use_method, use_parent, (TStatement*)use_syntax)
{}

void TSReturn::Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value, int method_base)
{
	result->Run(stack, result_returned, return_value, method_base);
	result_returned = true;
	conversions->RunConversion(stack);
	int ret_size = method->GetRuturnSize();
	//copy ret value
}