#pragma once

#include <vector>

class TStaticValue;
class TStackValue;

class TGlobalRunContext
{
public:
	std::vector<TStaticValue>* static_fields;
	TGlobalRunContext()
	{
		static_fields = nullptr;
	}
};

class TMethodRunContext : public TGlobalRunContext
{
public:	
	std::vector<TStackValue>* formal_params;
	TStackValue* result;
	TStackValue* object;
	TMethodRunContext()
	{
		static_fields = nullptr;
		formal_params = nullptr;
		result = nullptr;
		object = nullptr;
	}
	TMethodRunContext(std::vector<TStaticValue>* static_fields, std::vector<TStackValue>* formal_params, TStackValue* result, TStackValue* object)
	{
		this->static_fields = static_fields;
		this->formal_params = formal_params;
		this->result = result;
		this->object = object;
	}
};

class TStatementRunContext : public TMethodRunContext
{
public:
	bool* result_returned;
	std::vector<TStackValue>* local_variables;

	TStatementRunContext()
	{
		result_returned = nullptr;
		local_variables = nullptr;
	}
	TStatementRunContext(TMethodRunContext method_context, bool* result_returned, std::vector<TStackValue>* local_variables)
		:TMethodRunContext(method_context)
	{
		this->result_returned = result_returned;
		this->local_variables = local_variables;
	}
	TStatementRunContext(TMethodRunContext method_context, std::vector<TStackValue>* local_variables)
		:TMethodRunContext(method_context)
	{
		this->result_returned = nullptr;
		this->local_variables = local_variables;
	}
};

class TExpressionRunContext : public TStatementRunContext
{
public:
	TStackValue* expression_result;

	TExpressionRunContext()
	{
		expression_result = nullptr;
	}
	TExpressionRunContext(TStatementRunContext statement_context, TStackValue* expression_result)
		:TStatementRunContext(statement_context)
	{
		this->expression_result = expression_result;
	}
};