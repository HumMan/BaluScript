#pragma once

#include "../SemanticInterface/SemanticTreeApi.h"

#include "FormalParam.h"
#include "RunContext.h"

class TExpressionRunner
{
public:
	static void Run(SemanticApi::ISOperations::ISExpression* _this, TStatementRunContext& run_context);
	static void Run(SemanticApi::ISOperations::ISOperation* _this, TExpressionRunContext& run_context);
};