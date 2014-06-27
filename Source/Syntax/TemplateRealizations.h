#pragma once

#include "Class.h"

class TTemplateRealizations
{
public:
	struct TTemplateRealization
	{
		///<summary>”казатель на исходный шаблонный класс</summary>
		TClass* template_pointer;
		///<summary>—писок реализаций данного шаблона с различными шаблонными параметрами</summary>
		std::vector<std::unique_ptr<TClass>> realizations;
	};
	///<summary>—писок всех шаблонов и их реализаций</summary>
	std::vector<std::unique_ptr<TTemplateRealization>> templates;
	std::vector<std::unique_ptr<TClass>>* FindTemplate(TClass* use_template)
	{
		for (int i = 0; i < templates.size(); i++)
			if (templates[i]->template_pointer == use_template)
				return &templates[i]->realizations;
		return NULL;
	}
};
