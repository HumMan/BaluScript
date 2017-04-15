#include "Type.h"

#include "Class.h"
#include "OverloadedMethod.h"
#include "Method.h"
#include "Statements.h"

using namespace Lexer;
using namespace SyntaxInternal;

std::vector<std::shared_ptr<SyntaxApi::IType_TClassName>> TType::GetClassNames() const
{
	auto result = std::vector<std::shared_ptr<SyntaxApi::IType_TClassName>>();
	for (auto v : class_names)
	{
		result.push_back(v);
	}
	return result;
}

std::vector<std::shared_ptr<TType_TClassName>> TType::GetClassNamesT() const
{
	return class_names;
}

TType::TType(TNameId use_class_name, TClass *use_owner) :
	owner(use_owner) 
{
	class_names.emplace_back();
	class_names.back().reset(new TType_TClassName(use_class_name));
}

TType::TType(TClass *use_owner) 
	: owner(use_owner) 
{
}

TType_TClassName::TType_TClassName(Lexer::TNameId name)
{
	this->name = name;
}
Lexer::TNameId TType_TClassName::GetName()const
{
	return name;
}
void TType_TClassName::AddDimension(int dimension)
{
	dimensions.push_back(dimension);
}
void TType_TClassName::AddTemplateParameter(const TType_TTemplateParameter& param)
{
	template_params.emplace_back(new TType_TTemplateParameter(param));
}
std::vector<int> TType_TClassName::GetDimensions()const
{
	return dimensions;
}

std::vector<std::shared_ptr<SyntaxApi::IType_TTemplateParameter>> TType_TClassName::GetTemplateParams() const
{
	auto result = std::vector<std::shared_ptr<SyntaxApi::IType_TTemplateParameter>>();
	for (auto v : template_params)
	{
		result.push_back(v);
	}
	return result;
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

void TType::AnalyzeClassName(Lexer::ILexer* source)
{
	//любое объявление типа начинается с идентификатора класса
	source->TestToken(TTokenType::Identifier);
	class_names.emplace_back(new TType_TClassName(source->NameId()));
	source->GetToken(TTokenType::Identifier);

	//далее могут следовать шаблонные параметры
	if (source->Test(TOperator::Less)) 
	{
		source->GetToken();
		while (!source->TestAndGet(TOperator::Greater)) 
		{
			
			//в качестве шаблонного параметра можно использовать целые числа
			if (source->Test(TValue::Int))
			{
				class_names.back()->AddTemplateParameter(TType_TTemplateParameter(true, source->Int(), nullptr));
				source->GetToken();
			}
			else
			{
				TType* template_param = new TType(owner);
				class_names.back()->AddTemplateParameter(TType_TTemplateParameter(false, 0, template_param));

				//рекурсивно разбираем шаблонный параметр
				template_param->AnalyzeSyntax(source);
			}
			if (!source->Test(TOperator::Greater))
				source->GetToken(TTokenType::Comma);
		}
	}
	//после этого следуют измерения массивов
	AnalyzeDimensions(source);
	//TODO
	//class_names.back().ConvertDimensionsToTemplates();
}

void TType::AnalyzeSyntax(Lexer::ILexer* source) 
{	
	InitPos(source);
	AnalyzeClassName(source);
	//после точки может следовать указание вложенного класса
	if (source->Test(TTokenType::Dot)) 
	{
		source->GetToken();
		AnalyzeClassName(source);
	}
}

void TType::AnalyzeDimensions(Lexer::ILexer* source) 
{
	if (source->Test(TTokenType::LBracket) || source->Test(TOperator::GetArrayElement)) 
	{
		while (true) 
		{
			if (source->TestAndGet(TTokenType::LBracket)) 
			{
				if (source->Test(TValue::Int)) 
				{
					class_names.back()->AddDimension(source->Int());
					source->GetToken();
				}
				source->GetToken(TTokenType::RBracket);
			}
			else if (source->TestAndGet(TOperator::GetArrayElement)) 
			{
				class_names.back()->AddDimension(-1);
			} 
			else
				break;
		}
	}
}

TType_TTemplateParameter::TType_TTemplateParameter(bool is_value, int value, TType* type)
{
	assert(is_value ^ (type != nullptr));
	this->is_value = is_value;
	this->value = value;
	this->type.reset(type);
}

bool TType_TTemplateParameter::IsValue()const
{
	return is_value;
}
int TType_TTemplateParameter::GetValue()const
{
	assert(is_value);
	return value;
}
TType* TType_TTemplateParameter::GetType()const
{
	return type.get();
}