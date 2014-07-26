#pragma once

#include <assert.h>
#include <list>
#include <vector>

#include "../lexer.h"

class TSClass;
class TSType;
struct TSType_TTemplateParameter;

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
	class TTemplateParameter
	{
	public:
		bool is_value;
		int value;
		TSClass* type;
	};
private:
	Type type;
	///<summary>Если класс является реализацией одного из шаблонов, то содержит указатели на классы, заданные в параметрах шаблона</summary>
	std::vector<TTemplateParameter> template_params;
	bool template_params_set;
	///<summary>Если класс является реализацией одного из шаблонов, то содержит указатель на шаблонный класс по которому была сгенерирована данная реализация</summary>
	TSClass* template_class;
	bool template_class_set;
	std::vector<TSClass*> realizations;
public:
	TSClass* FindTemplateRealization(const std::list<TSType_TTemplateParameter>& params_to_find);
	int FindTemplateIntParameter(TNameId parameter_id);
	TNodeWithTemplates()
	{
		type = Unknown;
		template_class_set = false;
		template_params_set = false;
		template_class = NULL;
	}
	std::vector<TSClass*> GetRealizations()
	{
		assert(type == Template);
		return realizations;
	}
	TSClass* GetTemplateClass()
	{
		assert(type == Realization);
		return template_class;
	}
	std::vector<TTemplateParameter> GetTemplateParams()
	{
		assert(type == Realization);
		return template_params;
	}
	TTemplateParameter GetTemplateParam(int i)
	{
		return template_params[i];
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
	void SetTemplateParams(std::vector<TTemplateParameter> params)
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

class TSMultifield
{
	int size_multiplier;
	bool size_multiplier_set;
public:
	TSMultifield()
	{
		size_multiplier = -1;
		size_multiplier_set = false;
	}
	void SetSizeMultiplier(int mul)
	{
		assert(!size_multiplier_set);
		size_multiplier = mul;
		size_multiplier_set = true;
	}
	bool HasSizeMultiplier()
	{
		return size_multiplier_set;
	}
	int GetSizeMultiplier()
	{
		return size_multiplier;
	}
};