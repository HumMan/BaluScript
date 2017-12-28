#include "SReturn.h"

#include "SCommon.h"

#include "SMethod.h"
#include "SExpression.h"
#include "SParameter.h"
#include "SClass.h"

void TSReturn::Build(TGlobalBuildContext build_context)
{
	result = std::unique_ptr<TSExpression>(new TSExpression(GetOwner(), GetMethod(), GetParentStatements(), GetSyntax()->GetResult()));
	result->Build(build_context);
	TExpressionResult result_result = result->GetFormalParameter();
	if (GetMethod()->GetRetClass() != nullptr)
	{
		int conv_needed;
		if(result_result.IsVoid())
			GetSyntax()->Error("После оператора return должно следовать какое-то выражение!");
		if (!IsEqualClasses(result_result, SemanticApi:: TFormalParameter(GetMethod()->GetRetClass(), GetMethod()->GetSyntax()->IsReturnRef()), conv_needed))
			GetSyntax()->Error("Выражение невозможно преобразовать в тип возвращаемого значения!");
		conversions.reset(new TActualParamWithConversion());
		conversions->BuildConvert(result_result, SemanticApi::TFormalParameter(GetMethod()->GetRetClass(), GetMethod()->GetSyntax()->IsReturnRef()));
	}
	else 
		if(result_result.GetClass()!=nullptr)
			GetSyntax()->Error("Метод не должен ничего возвращать!");
}

TSReturn::TSReturn(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IReturn* use_syntax)
	:TSStatement(SyntaxApi::TStatementType::Return, use_owner, use_method, use_parent, dynamic_cast<SyntaxApi::IStatement*>(use_syntax))
{}

SyntaxApi::IReturn* TSReturn::GetSyntax()
{
	return dynamic_cast<SyntaxApi::IReturn*>(TSyntaxNode::GetSyntax());
}
