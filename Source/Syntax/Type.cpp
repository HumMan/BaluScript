#include "Type.h"

#include "Class.h"
#include "OverloadedMethod.h"
#include "Method.h"
#include "Statements.h"

TType::TType(TNameId use_class_name, TClass *use_owner) :
	owner(use_owner) 
{
	class_names.emplace_back();
	class_names.back().name = use_class_name;
}

TType::TType(TClass *use_owner) 
	: owner(use_owner) 
{
}

void TType_TClassName::ConvertDimensionsToTemplates()
{
	//TODO конвертация измерений массивов в TDynArray<T>
	//if (dimensions.size() > 0)
	//{
	//	int curr_dim = dimensions.back();
	//	dimensions.pop_back();
	//	TNameId old_name = name;
	//	std::list<TType_TTemplateParameter> old_template_params = template_params;

	//	template_params.clear();
	//	template_params.emplace_back();
	//}
}

void TType::AnalyzeClassName(TLexer& source)
{
	//любое объявление типа начинается с идентификатора класса
	source.TestToken(TTokenType::Identifier);
	class_names.emplace_back();
	class_names.back().name = source.NameId();
	source.GetToken(TTokenType::Identifier);

	//далее могут следовать шаблонные параметры
	if (source.Test(TTokenType::Operator, TOperator::Less)) 
	{
		source.GetToken();
		while (!source.TestAndGet(TTokenType::Operator, TOperator::Greater)) 
		{
			class_names.back().template_params.emplace_back();
			//в качестве шаблонного параметра можно использовать целые числа
			if (source.Test(TTokenType::Value, TValue::Int))
			{
				class_names.back().template_params.back().is_value = true;
				class_names.back().template_params.back().value = source.Int();
				source.GetToken();
			}
			else
			{
				class_names.back().template_params.back().is_value = false;
				class_names.back().template_params.back().type.reset(new TType(owner));
				TType* template_param = class_names.back().template_params.back().type.get();

				//рекурсивно разбираем шаблонный параметр
				template_param->AnalyzeSyntax(source);
			}
			if (!source.Test(TTokenType::Operator, TOperator::Greater))
				source.GetToken(TTokenType::Comma);
		}
	}
	//после этого следуют измерения массивов
	AnalyzeDimensions(source);
	class_names.back().ConvertDimensionsToTemplates();
}

void TType::AnalyzeSyntax(TLexer& source) 
{	
	InitPos(source);
	AnalyzeClassName(source);
	//после точки может следовать указание вложенного класса
	if (source.Test(TTokenType::Dot)) 
	{
		source.GetToken();
		AnalyzeClassName(source);
	}
}

void TType::AnalyzeDimensions(TLexer& source) 
{
	if (source.Test(TTokenType::LBracket) || source.Test(TTokenType::Operator, TOperator::GetArrayElement)) 
	{
		while (true) 
		{
			if (source.TestAndGet(TTokenType::LBracket)) 
			{
				if (source.Test(TTokenType::Value, TValue::Int)) 
				{
					class_names.back().dimensions.push_back(source.Int());
					source.GetToken();
				}
				source.GetToken(TTokenType::RBracket);
			}
			else if (source.TestAndGet(TTokenType::Operator, TOperator::GetArrayElement)) 
			{
				class_names.back().dimensions.push_back(-1);
			} 
			else
				break;
		}
	}
}