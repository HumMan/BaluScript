#include "SStatement.h"

class TFormalParam;
class TBytecode;

class TSBytecode :public TSStatement
{
public:
	TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TBytecode* use_syntax);
	TFormalParam Build();
};