#include "RunContext.h"

#include "FormalParam.h"

void TMethodRunContext::AddRefsFromParams()
{
	if (formal_params == nullptr)
		return;	
	for (auto& p : *formal_params)
	{
		if (p.IsRef())
			refs_list->AddRef(p.get());
	}
}
void TMethodRunContext::RemoveRefsFromParams()
{
	if (formal_params == nullptr)
		return;
	for (auto& p : *formal_params)
	{
		if (p.IsRef())
			refs_list->RemoveRef(p.get());
	}
}