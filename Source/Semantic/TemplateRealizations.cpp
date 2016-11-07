#include "TemplateRealizations.h"

#include "SType.h"
#include "SClass.h"
#include "SStatements.h"

#include "../Syntax/Method.h"
#include "../Syntax/Statements.h"

int TNodeWithTemplates::FindTemplateIntParameter(Lexer::TNameId parameter_id)
{
	assert(type == Realization);
	for (size_t i = 0; i < template_params.size(); i++)
	{
		if (template_params[i].is_value&&template_class->GetSyntax()->template_params[i] == parameter_id)
			return template_params[i].value;
	}
	assert(false);//TODO поиск шаблонного параметра значения должен происходить не только здесь, можно инициализировать значения в выражениях
	return -1;
}

TSClass* TNodeWithTemplates::FindTemplateRealization(const std::list<TSType_TTemplateParameter>& params_to_find)
{
	assert(type == Template);
	for (const std::unique_ptr<TSClass>& realization : realizations)
	{
		const std::vector<TNodeWithTemplates::TTemplateParameter>& r_params = realization->GetTemplateParams();
		if (r_params.size() != params_to_find.size())
		{
			continue;
		}
		bool found = true;
		auto it = params_to_find.begin();
		for (size_t i = 0; i < r_params.size(); i++)
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
			return realization.get();
	}
	return NULL;
}

TNodeWithTemplates::TNodeWithTemplates()
{
	type = Unknown;
	template_class_set = false;
	template_params_set = false;
	template_class = NULL;
}

TNodeWithTemplates::~TNodeWithTemplates()
{
}

const std::vector<std::unique_ptr<TSClass>>& TNodeWithTemplates::GetRealizations()
{
	assert(type == Template);
	return realizations;
}
TSClass* TNodeWithTemplates::GetTemplateClass()
{
	assert(type == Realization);
	return template_class;
}
std::vector<TNodeWithTemplates::TTemplateParameter> TNodeWithTemplates::GetTemplateParams()
{
	assert(type == Realization);
	return template_params;
}
TNodeWithTemplates::TTemplateParameter TNodeWithTemplates::GetTemplateParam(int i)
{
	return template_params[i];
}
void TNodeWithTemplates::SetType(Type use_type)
{
	assert(type == Unknown);
	assert(use_type != Unknown);
	type = use_type;
}
TNodeWithTemplates::Type TNodeWithTemplates::GetType()
{
	assert(type != Unknown);
	return type;
}
void TNodeWithTemplates::SetTemplateParams(std::vector<TTemplateParameter> params)
{
	assert(!template_params_set);
	assert(type == Realization);
	this->template_params = params;
	template_params_set = true;
}
void TNodeWithTemplates::SetTemplateClass(TSClass* template_class)
{
	assert(!template_class_set);
	assert(type == Realization);
	this->template_class = template_class;
	template_class_set = true;
}
void TNodeWithTemplates::AddTemplateRealization(TSClass* realization)
{
	assert(type == Template);
	realizations.push_back(std::unique_ptr<TSClass>(realization));
}