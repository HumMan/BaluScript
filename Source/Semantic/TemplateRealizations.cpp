#include "TemplateRealizations.h"

#include "SType.h"

std::vector<std::shared_ptr<TSClass>>* TTemplateRealizations::FindTemplateRealizations(TSClass* use_template)
{
	for (int i = 0; i < templates.size(); i++)
		if (templates[i]->template_pointer == use_template)
			return &templates[i]->realizations;
	return NULL;//assert(true);//шаблон должен присутствовать
}
TSClass* TTemplateRealizations::FindTemplateRealization(TSClass* use_template, const std::list<TSType>& params_to_find)
{
	std::vector<std::shared_ptr<TSClass>>* arr = FindTemplateRealizations(use_template);
	for (const std::shared_ptr<TSClass>& realization : *arr)
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
			return realization.get();
	}
	return NULL;
}