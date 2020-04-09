#include "RunContext.h"

#include "FormalParam.h"

class TGlobalRunContextPrivate
{
public:
	std::vector<TStaticValue> static_fields;
	TRefsList refs_list;
};

class TMethodRunContextPrivate
{
public:
	std::vector<TStackValue> formal_params;
	TStackValue result;
	TStackValue object;
	TGlobalRunContext* global_context;
	bool result_returned;
};

class TStatementRunContextPrivate
{
public:	
	std::vector<TStackValue> local_variables;
	std::vector<TStackValue> temp_objects;
	TMethodRunContext* method_context;
};

class TExpressionRunContextPrivate
{
public:
	TStackValue expression_result;
	TStatementRunContext* statement_context;
};

std::vector<TStaticValue>& TGlobalRunContext::GetStaticFields()
{
	return p->static_fields;
}

TRefsList& TGlobalRunContext::GetRefsList()
{
	return p->refs_list;
}

TGlobalRunContext::TGlobalRunContext()
{
	p = new TGlobalRunContextPrivate();
}

TGlobalRunContext::~TGlobalRunContext()
{
	delete p;
}

void TMethodRunContext::AddRefsFromParams()
{
	for (auto& param : p->formal_params)
	{
		if (param.IsRef())
			p->global_context->GetRefsList().AddRef(param.get());
	}
}
void TMethodRunContext::RemoveRefsFromParams()
{
	for (auto& param : p->formal_params)
	{
		if (param.IsRef())
			p->global_context->GetRefsList().RemoveRef(param.get());
	}
}

void TExpressionRunContext::SetExpressionResult(TStackValue value)
{
	//assert(p->expression_result.get() == nullptr);
	p->expression_result = std::move(value);
}

TStackValue& TExpressionRunContext::GetExpressionResult()
{
	return p->expression_result;
}



TMethodRunContext::TMethodRunContext(TGlobalRunContext* global_context)
{
	p = new TMethodRunContextPrivate();
	p->global_context = global_context;
}
TMethodRunContext::~TMethodRunContext()
{
	delete p;
}

TGlobalRunContext* TMethodRunContext::GetGlobalContext()
{
	return p->global_context;
}

TGlobalRunContext* TStatementRunContext::GetGlobalContext()
{
	return p->method_context->GetGlobalContext();
}

TStackValue& TMethodRunContext::GetObject()
{
	return p->object;
}

std::vector<TStackValue>& TMethodRunContext::GetFormalParams()
{
	return p->formal_params;
}

bool TMethodRunContext::IsResultReturned()
{
	return p->result_returned;
}

void TMethodRunContext::SetResultReturned()
{
	assert(!p->result_returned);
	p->result_returned=true;
}

TStackValue& TMethodRunContext::GetResult()
{
	return p->result;
}

TMethodRunContext* TStatementRunContext::GetMethodContext()
{
	return p->method_context;
}



std::vector<TStackValue>& TStatementRunContext::GetLocalVariables()
{
	return p->local_variables;
}

std::vector<TStackValue>& TStatementRunContext::GetTempObjects()
{
	return p->temp_objects;
}

TStatementRunContext::TStatementRunContext(TMethodRunContext* method_context)
{
	assert(method_context != nullptr);
	p = new TStatementRunContextPrivate();
	p->method_context = method_context;
}

TStatementRunContext::~TStatementRunContext()
{
	delete p;
}

TStatementRunContext* TExpressionRunContext::GetStatementContext()
{
	return p->statement_context;
}

TExpressionRunContext::TExpressionRunContext(TStatementRunContext* statement_context)
{
	p = new TExpressionRunContextPrivate();
	p->statement_context = statement_context;
}

TMethodRunContext* TExpressionRunContext::GetMethodContext()
{
	return p->statement_context->GetMethodContext();
}

TGlobalRunContext* TExpressionRunContext::GetGlobalContext()
{
	return p->statement_context->GetMethodContext()->GetGlobalContext();
}

TExpressionRunContext::~TExpressionRunContext()
{
	delete p;
}
