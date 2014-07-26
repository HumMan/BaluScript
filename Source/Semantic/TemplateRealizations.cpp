#include "TemplateRealizations.h"

#include "SType.h"
#include "SClass.h"

#include "../Syntax/Method.h"
#include "../Syntax/Statements.h"

int TNodeWithTemplates::FindTemplateIntParameter(TNameId parameter_id)
{
	assert(type == Realization);
	for (int i = 0; i < template_params.size(); i++)
	{
		if (template_params[i].is_value&&template_class->GetSyntax()->template_params[i] == parameter_id)
			return template_params[i].value;
	}
	assert(false);//TODO поиск шаблонного параметра значения должен происходить не только здесь, можно инициализировать значения в выражениях
}

TSClass* TNodeWithTemplates::FindTemplateRealization(const std::list<TSType_TTemplateParameter>& params_to_find)
{
	assert(type == Template);
	for (TSClass* realization : realizations)
	{
		const std::vector<TNodeWithTemplates::TTemplateParameter>& r_params = realization->GetTemplateParams();
		if (r_params.size() != params_to_find.size())
		{
			continue;
		}
		bool found = true;
		auto it = params_to_find.begin();
		for (int i = 0; i < r_params.size(); i++)
		{
			if (r_params[i].is_value)
			{
				if (r_params[i].value != it->value)
				{
					found = false;
					break;
				}
			}
			else
			{
				if (r_params[i].type != it->type->GetClass())
				{
					found = false;
					break;
				}
			}
			it++;
		}
		if (found)
			return realization;
	}
	return NULL;
}