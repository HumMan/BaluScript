#pragma once

#include "Class.h"

class TTemplateRealizations
{
public:
	struct TTemplateRealization
	{
		///<summary>Указатель на исходный шаблонный класс</summary>
		TClass* template_pointer;
		///<summary>Список реализаций данного шаблона с различными шаблонными параметрами</summary>
		std::vector<std::unique_ptr<TClass>> realizations;
	};
	///<summary>Список всех шаблонов и их реализаций</summary>
	std::vector<std::unique_ptr<TTemplateRealization>> templates;
	std::vector<std::unique_ptr<TClass>>* FindTemplate(TClass* use_template)
	{
		for (int i = 0; i < templates.size(); i++)
			if (templates[i]->template_pointer == use_template)
				return &templates[i]->realizations;
		return NULL;
	}
};
