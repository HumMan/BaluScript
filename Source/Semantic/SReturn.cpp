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

void TSReturn::Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	result = std::unique_ptr<TSExpression>(new TSExpression(owner, method, parent, &GetSyntax()->result));
	result->Build(static_fields, static_variables);
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

void TSReturn::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& method_result_value, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	TStackValue return_value;
	result->Run(static_fields, formal_params, result_returned, return_value, object, local_variables);
	result_returned = true;
	conversions->RunConversion(static_fields, return_value);
	method_result_value = return_value;
}