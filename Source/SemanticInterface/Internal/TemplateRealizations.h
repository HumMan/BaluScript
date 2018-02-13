#pragma once

#include "../../SyntaxInterface/SyntaxTreeApi.h"

#include "../SemanticTreeApi.h"

class TSClass;
class TSType_TTemplateParameter;

enum class TNodeWithTemplatesType
{
	Class,
	Template,
	Realization,
	Unknown
};

class TNodeWithTemplates: public virtual SemanticApi::INodeWithTemplates
{
private:
	TNodeWithTemplatesType type;
	///<summary>Если класс является реализацией одного из шаблонов, то массив содержит указатели на классы, заданные в параметрах шаблона</summary>
	std::vector<SemanticApi::TTemplateParameter> template_params;
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
	TSClass* GetTemplateClass()const;
	std::vector<SemanticApi::TTemplateParameter> GetTemplateParams();
	SemanticApi::TTemplateParameter GetTemplateParam(int i)const;
	void SetType(TNodeWithTemplatesType use_type);
	TNodeWithTemplatesType GetType()const;
	void SetTemplateParams(std::vector<SemanticApi::TTemplateParameter> params);
	void SetTemplateClass(TSClass* template_class);
	void AddTemplateRealization(TSClass* realization);
};

class TSMultifield: public virtual SemanticApi::ISMultifield
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
	bool HasSizeMultiplier()const
	{
		return size_multiplier_set;
	}
	size_t GetSizeMultiplier()const
	{
		return size_multiplier;
	}
};