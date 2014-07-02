#include "../Syntax/Bytecode.h"

class TFormalParam;

class TBytecodeSemantic :public TBytecode
{

public:
	TBytecodeSemantic(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id);
	TFormalParam Build(TNotOptimizedProgram &program, int& local_var_offset);
};