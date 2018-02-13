#include "TemplateRealizations.h"

#include "SType.h"
#include "SClass.h"
#include "SStatements.h"

int TNodeWithTemplates::FindTemplateIntParameter(Lexer::TNameId parameter_id)
{
	assert(type == SemanticApi::TNodeWithTemplatesType::Realization);
	for (size_t i = 0; i < template_params.size(); i++)
	{
		if (template_params[i].is_value&&template_class->GetSyntax()->GetTemplateParam(i) == parameter_id)
			return template_params[i].value;
	}
	assert(false);//TODO поиск шаблонного параметра значения должен происходить не только здесь, можно инициализировать значения в выражениях
	return -1;
}

TSClass* TNodeWithTemplates::FindTemplateRealization(const std::list<TSType_TTemplateParameter>& params_to_find)
{
	assert(type == SemanticApi::TNodeWithTemplatesType::Template);
	for (const std::unique_ptr<TSClass>& realization : realizations)
	{
		const std::vector<SemanticApi::TTemplateParameter>& r_params = realization->GetTemplateParams();
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
	return nullptr;
}

TNodeWithTemplates::TNodeWithTemplates()
{
	type = SemanticApi::TNodeWithTemplatesType::Unknown;
	template_class_set = false;
	template_params_set = false;
	template_class = nullptr;
}

TNodeWithTemplates::~TNodeWithTemplates()
{
}

const std::vector<std::unique_ptr<TSClass>>& TNodeWithTemplates::GetRealizations()
{
	assert(type == SemanticApi::TNodeWithTemplatesType::Template);
	return realizations;
}
TSClass* TNodeWithTemplates::GetTemplateClass()const
{
	assert(type == SemanticApi::TNodeWithTemplatesType::Realization);
	return template_class;
}
std::vector<SemanticApi::TTemplateParameter> TNodeWithTemplates::GetTemplateParams()
{
	assert(type == SemanticApi::TNodeWithTemplatesType::Realization || type == SemanticApi::TNodeWithTemplatesType::Template);
	return template_params;
}
SemanticApi::TTemplateParameter TNodeWithTemplates::GetTemplateParam(int i)const
{
	return template_params[i];
}
void TNodeWithTemplates::SetType(SemanticApi::TNodeWithTemplatesType use_type)
{
	assert(type == SemanticApi::TNodeWithTemplatesType::Unknown);
	assert(use_type != SemanticApi::TNodeWithTemplatesType::Unknown);
	type = use_type;
}
SemanticApi::TNodeWithTemplatesType TNodeWithTemplates::GetType()const
{
	assert(type != SemanticApi::TNodeWithTemplatesType::Unknown);
	return type;
}
void TNodeWithTemplates::SetTemplateParams(std::vector<SemanticApi::TTemplateParameter> params)
{
	assert(!template_params_set);
	assert(type == SemanticApi::TNodeWithTemplatesType::Realization);
	this->template_params = params;
	template_params_set = true;
}
void TNodeWithTemplates::SetTemplateClass(TSClass* template_class)
{
	assert(!template_class_set);
	assert(type == SemanticApi::TNodeWithTemplatesType::Realization);
	this->template_class = template_class;
	template_class_set = true;
}
void TNodeWithTemplates::AddTemplateRealization(TSClass* realization)
{
	assert(type == SemanticApi::TNodeWithTemplatesType::Template);
	realizations.push_back(std::unique_ptr<TSClass>(realization));
}

//bool TNodeWithTemplates::TTemplateParameter::IsValue()const
//{
//	return is_value;
//}
//int TNodeWithTemplates::TTemplateParameter::GetValue()const
//{
//	assert(is_value);
//	return value;
//}
//SemanticApi::ISClass* TNodeWithTemplates::TTemplateParameter::GetType()const
//{
//	assert(!is_value);
//	return type;
//}