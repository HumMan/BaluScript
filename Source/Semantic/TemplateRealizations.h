#pragma once

#include "SClass.h"

class TSClass;
class TSType_TClassName;

class TTemplateRealizations
{
public:
	struct TTemplateRealization
	{
		///<summary>”казатель на исходный шаблонный класс</summary>
		TSClass* template_pointer;
		///<summary>—писок реализаций данного шаблона с различными шаблонными параметрами</summary>
		std::vector<std::shared_ptr<TSClass>> realizations;
	};
	///<summary>—писок всех шаблонов и их реализаций</summary>
	std::vector<std::shared_ptr<TTemplateRealization>> templates;
	std::vector<std::shared_ptr<TSClass>>* FindTemplateRealizations(TSClass* use_template);
	TSClass* FindTemplateRealization(TSClass* use_template, const std::vector<std::list<TSType_TClassName>>& params_to_find);
};
