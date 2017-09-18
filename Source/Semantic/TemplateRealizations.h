#pragma once

#include "../SyntaxTree/SyntaxTreeApi.h"

class TSClass;
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
	std::vector<std::unique_ptr<TSClass>> realizations;
public:
	TNodeWithTemplates();
	~TNodeWithTemplates();

	TSClass* FindTemplateRealization(const std::list<TSType_TTemplateParameter>& params_to_find);
	int FindTemplateIntParameter(Lexer::TNameId parameter_id);
	const std::vector<std::unique_ptr<TSClass>>& GetRealizations();
	TSClass* GetTemplateClass();
	std::vector<TTemplateParameter> GetTemplateParams();
	TTemplateParameter GetTemplateParam(int i);
	void SetType(Type use_type);
	Type GetType();
	void SetTemplateParams(std::vector<TTemplateParameter> params);
	void SetTemplateClass(TSClass* template_class);
	void AddTemplateRealization(TSClass* realization);
};

class TSMultifield
{
	size_t size_multiplier;
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
	size_t GetSizeMultiplier()
	{
		return size_multiplier;
	}
};