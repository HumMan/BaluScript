#include "SStatement.h"

#include "SClass.h"
#include "SExpression.h"

#include "../semanticAnalyzer.h"

void TSStatement::TestBoolExpr(TExpressionResult& compare_result, std::unique_ptr<TActualParamWithConversion>& conversion)
{
	int conv_needed;
	
	TSClass* bool_class = owner->GetClass(GetSyntax()->GetLexer()->GetIdFromName("bool"));
	TFormalParameter formal_param(bool_class, false);
	if (!IsEqualClasses(compare_result, formal_param, conv_needed))
		GetSyntax()->Error("Выражение невозможно преобразовать в логический тип!");

	conversion.reset(new TActualParamWithConversion());
	conversion->BuildConvert(compare_result, formal_param);
}
