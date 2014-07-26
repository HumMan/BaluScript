#include "SStatement.h"

#include "SClass.h"

#include "../semanticAnalyzer.h"

#include "../Syntax/Statement.h"
#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"

void TSStatement::TestBoolExpr(TFormalParam& compare_result, std::unique_ptr<TFormalParamWithConversions>& conversion)
{
	int conv_needed;
	TSClass* bool_class = owner->GetClass(GetSyntax()->source->GetIdFromName("bool"));
	if (!IsEqualClasses(compare_result, bool_class, false, conv_needed))
		GetSyntax()->Error("Выражение невозможно преобразовать в логический тип!");

	conversion = std::unique_ptr<TFormalParamWithConversions>(new TFormalParamWithConversions());
	conversion->BuildConvert(compare_result, bool_class, false);
}
