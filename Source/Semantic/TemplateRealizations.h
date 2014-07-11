#pragma once

#include <assert.h>
#include <list>
#include <vector>

class TSClass;
class TSType;

class TNodeWithTemplates
{
public:
	enum Type
	{
		Class,
		Template,
		Realization,
		Unknown
	};
private:
	Type type;
	///<summary>Если класс является реализацией одного из шаблонов, то содержит указатели на классы, заданные в параметрах шаблона</summary>
	std::vector<TSClass*> template_params;
	bool template_params_set;
	///<summary>Если класс является реализацией одного из шаблонов, то содержит указатель на шаблонный класс по которому была сгенерирована данная реализация</summary>
	TSClass* template_class;
	bool template_class_set;
	std::vector<TSClass*> realizations;
public:
	TSClass* FindTemplateRealization(const std::list<TSType>& params_to_find);
	TNodeWithTemplates()
	{
		type = Unknown;
		template_class_set = false;
		template_params_set = false;
		template_class = NULL;
	}
	TSClass* GetTemplateClass()
	{
		assert(type == Realization);
		return template_class;
	}
	std::vector<TSClass*> GetTemplateParams()
	{
		assert(type == Realization);
		return template_params;
	}
	void SetType(Type use_type)
	{
		assert(type == Unknown);
		assert(use_type != Unknown);
		type = use_type;
	}
	Type GetType()
	{
		assert(type != Unknown);
		return type;
	}
	void SetTemplateParams(std::vector<TSClass*> params)
	{
		assert(!template_params_set);
		assert(type == Realization);
		this->template_params = params;
		template_params_set = true;
	}
	void SetTemplateClass(TSClass* template_class)
	{
		assert(!template_class_set);
		assert(type == Realization);
		this->template_class = template_class;
		template_class_set = true;
	}
	void AddTemplateRealization(TSClass* realization)
	{
		assert(type == Template);
		realizations.push_back(realization);
	}
};