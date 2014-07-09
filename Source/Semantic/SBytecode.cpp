#include "SBytecode.h"
#include "../virtualMachine.h"
#include "../Syntax/Bytecode.h"

TSBytecode::TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TBytecode* use_syntax)
	:TSStatement(TStatementType::Bytecode,use_owner,use_method,use_parent,(TStatement*)use_syntax)
{

}

void TSBytecode::Run(int* &sp, bool& result_returned, int* return_value)
{
	for (TBytecode::TBytecodeOp& op : GetSyntax()->code)
	{
		TVirtualMachine::ExecuteIntOps(&op.op, sp);
	}
}
