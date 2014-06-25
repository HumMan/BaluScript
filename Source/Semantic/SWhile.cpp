#include "../Syntax/While.h"

#include "../Syntax/Void.h"

TFormalParam TWhile::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	/*TFormalParam bool_result=bool_expr.Build(ops_array,program,local_var_offset);
	int conv_needed;
	if(!IsEqualClasses(bool_result,owner->GetClassCmp("bool"),false,conv_needed))
	Error("Выражение невозможно преобразовать в логический тип!");
	method->BuildFormalParamConversion(program,bool_result,owner->GetClassCmp("bool"),false);	
	last_op=bool_result.LastOp();
	int end_if=program.GetUniqueLabel();
	program.Push(TOp(TOpcode::GOFALSE,end_if),ops_array);
	statements.Build(ops_array,program,local_var_offset);
	if(else_statements.GetHigh()>=0)
	{
	int end_else=program.GetUniqueLabel();
	program.Push(TOp(TOpcode::GOTO,end_else),ops_array);
	program.Push(TOp(TOpcode::LABEL,end_if),ops_array);
	else_statements.Build(ops_array,program,local_var_offset);
	program.Push(TOp(TOpcode::LABEL,end_else),ops_array);
	}else
	program.Push(TOp(TOpcode::LABEL,end_if),ops_array);*/
	return TVoid();
}
