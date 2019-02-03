#pragma once

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
	int size(TMethodRunContext* run_context);
	static void* operator_GetArrayElement(TMethodRunContext* run_context, TStaticArr* object, int index);

	static SemanticApi::TExternalClassDecl DeclareExternalClass();
};
