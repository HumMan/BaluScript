#pragma once

#include <baluLib.h>
#include "../VirtualMachine/Program.h"
#include "../virtualMachine.h"

#include "../Semantic/RunContext.h"

class TStaticValue;
class TStackValue;
class TSyntaxAnalyzer;
class TSClass;

class TStaticArr
{
public:
	static void get_element_op(TMethodRunContext run_context);
	static void get_size(TMethodRunContext run_context);

	static void DeclareExternalClass(TSyntaxAnalyzer* syntax);
};
