#include "../Syntax/Statement.h"

void TStatement::TestBoolExpr(TNotOptimizedProgram &program,TFormalParam& compare_result,int go_false_label)
{
	int conv_needed;
	TClass* bool_class=owner->GetClass(source->GetIdFromName("bool"));
	if(!IsEqualClasses(compare_result,bool_class,false,conv_needed))
		Error("Выражение невозможно преобразовать в логический тип!");
	method->BuildFormalParamConversion(program,compare_result,bool_class,false);	
	program.Push(TOp(TOpcode::GOFALSE,go_false_label),compare_result.GetOps());
}
