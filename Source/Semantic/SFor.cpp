#include "../Syntax/For.h"

#include "../Syntax/Void.h"
#include "../Syntax/Expression.h"
#include "../Syntax/Statements.h"

TFormalParam TFor::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	TOpArray ops_array;
	int start_loop=program.GetUniqueLabel();
	program.Push(TOp(TOpcode::LABEL,start_loop),ops_array);
	TFormalParam compare_result=compare->Build(program,local_var_offset);

	int end_loop=program.GetUniqueLabel();
	TestBoolExpr(program,compare_result,end_loop);
	ops_array+=compare_result.GetOps();

	ops_array+=statements->Build(program,local_var_offset).GetOps();
	ops_array+=increment->Build(program,local_var_offset).GetOps();
	program.Push(TOp(TOpcode::GOTO,start_loop),ops_array);
	program.Push(TOp(TOpcode::LABEL,end_loop),ops_array);
	return TVoid(ops_array);
}