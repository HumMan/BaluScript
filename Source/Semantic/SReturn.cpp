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
	result = std::unique_ptr<TSExpression>(new TSExpression(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetResult()));
	result->Build(build_context);
	TExpressionResult result_result = result->GetFormalParameter();
	if (GetMethod()->GetRetClass() != NULL)
	{
		int conv_needed;
		if(result_result.IsVoid())
			GetSyntax()->Error("После оператора return должно следовать какое-то выражение!");
		if (!IsEqualClasses(result_result, TFormalParameter(GetMethod()->GetRetClass(), GetMethod()->GetSyntax()->IsReturnRef()), conv_needed))
			GetSyntax()->Error("Выражение невозможно преобразовать в тип возвращаемого значения!");
		conversions.reset(new TActualParamWithConversion());
		conversions->BuildConvert(result_result, TFormalParameter(GetMethod()->GetRetClass(), GetMethod()->GetSyntax()->IsReturnRef()));
	}
	else 
		if(result_result.GetClass()!=NULL)
			GetSyntax()->Error("Метод не должен ничего возвращать!");
}

TSReturn::TSReturn(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IReturn* use_syntax)
	:TSStatement(SyntaxApi::TStatementType::Return, use_owner, use_method, use_parent, dynamic_cast<SyntaxApi::IStatement*>(use_syntax))
{}

SyntaxApi::IReturn* TSReturn::GetSyntax()
{
	return dynamic_cast<SyntaxApi::IReturn*>(TSyntaxNode::GetSyntax());
}

void TSReturn::Run(TStatementRunContext run_context)
{
	TStackValue return_value;
	result->Run(TExpressionRunContext(run_context, &return_value));
	*run_context.result_returned = true;
	conversions->RunConversion(*run_context.static_fields, return_value);
	*run_context.result = return_value;
}