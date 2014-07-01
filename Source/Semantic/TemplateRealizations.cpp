#include "TemplateRealizations.h"

#include "SType.h"

std::vector<std::shared_ptr<TSClass>>* TTemplateRealizations::FindTemplateRealizations(TSClass* use_template)
{
	for (int i = 0; i < templates.size(); i++)
		if (templates[i]->template_pointer == use_template)
			return &templates[i]->realizations;
	return NULL;//assert(true);//шаблон должен присутствовать
}
TSClass* TTemplateRealizations::FindTemplateRealization(TSClass* use_template, const std::vector<std::list<TSType_TClassName>>& params_to_find)
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
		for (int i = 0; i < r_params.size(); i++)
		{
			if (r_params[i] != params_to_find[i].back().class_of_type)
			{
				found = false;
				break;
			}
		}
		if (found)
			return realization.get();
	}
	return NULL;
}