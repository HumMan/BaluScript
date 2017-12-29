#pragma once

#include "../VirtualMachine/Program.h"
#include "../virtualMachine.h"

#include "../SemanticInterface/SemanticTreeApi.h"

#include "../TreeRunner/RunContext.h"

class TStaticValue;
class TStackValue;
class TSyntaxAnalyzer;
class TSClass;

class TStaticArr
{
public:
	static void get_element_op(TMethodRunContext* run_context);
	static void get_size(TMethodRunContext* run_context);

	static SemanticApi::TExternalClassDecl DeclareExternalClass();
};
