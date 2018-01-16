#pragma once

#include "../SemanticInterface/SemanticTreeApi.h"

#include "FormalParam.h"
#include "RunContext.h"

class TExpressionRunner
{
public:
	static void Run(SemanticApi::ISOperations::ISExpression* _this, TStatementRunContext run_context);
};