#include "../Syntax/If.h"

#include "../Syntax/Void.h"
#include "../Syntax/Expression.h"
#include "../Syntax/Statements.h"
#include "../Syntax/ClassField.h"

TFormalParam TIf::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	TOpArray ops_array;
	TFormalParam bool_result=bool_expr->Build(program,local_var_offset);

	int end_if=program.GetUniqueLabel();
	TestBoolExpr(program,bool_result,end_if);
	ops_array+=bool_result.GetOps();

	ops_array+=statements->Build(program,local_var_offset).GetOps();
	if(else_statements->GetHigh()>=0)
	{
		int end_else=program.GetUniqueLabel();
		program.Push(TOp(TOpcode::GOTO,end_else),ops_array);
		program.Push(TOp(TOpcode::LABEL,end_if),ops_array);
		ops_array+=else_statements->Build(program,local_var_offset).GetOps();
		program.Push(TOp(TOpcode::LABEL,end_else),ops_array);
	}else
		program.Push(TOp(TOpcode::LABEL,end_if),ops_array);
	return TVoid(ops_array);
}
