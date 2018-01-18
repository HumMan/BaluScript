#include "SStatement.h"

#include "SClass.h"
#include "SExpression.h"

#include "SCommon.h"

SemanticApi::ISClass * TSStatement::IGetOwner() const
{
	return owner;
}

SemanticApi::ISMethod * TSStatement::IGetMethod() const
{
	return method;
}

TSClass * TSStatement::GetOwner() const
{
	return owner;
}

TSMethod * TSStatement::GetMethod() const
{
	return method;
}

void TSStatement::TestBoolExpr(TExpressionResult& compare_result, std::unique_ptr<TActualParamWithConversion>& conversion)
{
	int conv_needed;
	
	TSClass* bool_class = owner->GetClass(GetSyntax()->GetLexer()->GetIdFromName("bool"));
	SemanticApi::TFormalParameter formal_param(bool_class, false);
	if (!IsEqualClasses(compare_result, formal_param, conv_needed))
		GetSyntax()->Error("Выражение невозможно преобразовать в логический тип!");

	conversion.reset(new TActualParamWithConversion());
	conversion->BuildConvert(compare_result, formal_param);
}
