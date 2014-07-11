#include "TemplateRealizations.h"

#include "SType.h"
#include "SClass.h"

TSClass* TNodeWithTemplates::FindTemplateRealization(const std::list<TSType>& params_to_find)
{
	assert(type == Template);
	for (TSClass* realization : realizations)
	{
		const std::vector<TSClass*>& r_params = realization->GetTemplateParams();
		if (r_params.size() != params_to_find.size())
		{
			continue;
		}
		bool found = true;
		auto it = params_to_find.begin();
		for (int i = 0; i < r_params.size(); i++)
		{
			if (r_params[i] != it->GetClass())
			{
				found = false;
				break;
			}
			it++;
		}
		if (found)
			return realization;
	}
	return NULL;
}