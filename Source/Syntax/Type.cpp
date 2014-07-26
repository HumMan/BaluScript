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

void TType::AnalyzeClassName(TLexer& source)
{
	source.TestToken(TTokenType::Identifier);
	class_names.emplace_back();
	class_names.back().name = source.NameId();
	source.GetToken(TTokenType::Identifier);

	if (source.Test(TTokenType::Operator, TOperator::Less)) 
	{
		source.GetToken();
		while (!source.TestAndGet(TTokenType::Operator, TOperator::Greater)) 
		{
			class_names.back().template_params.emplace_back();
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
				template_param->AnalyzeSyntax(source);
			}
			if (!source.Test(TTokenType::Operator, TOperator::Greater))
				source.GetToken(TTokenType::Comma);
		}
	}
	AnalyzeDimensions(source);
}

void TType::AnalyzeSyntax(TLexer& source) 
{	
	InitPos(source);
	AnalyzeClassName(source);
	if (source.Test(TTokenType::Dot)) 
	{
		source.GetToken();
		AnalyzeClassName(source);
	}
}

void TType::AnalyzeDimensions(TLexer& source) 
{
	//InitPos(source);
	//AnalyzeSyntax(&class_name, source);
	if (source.Test(TTokenType::LBracket) || source.Test(TTokenType::Operator, TOperator::GetArrayElement)) 
	{
		while (true) 
		{
			if (source.TestAndGet(TTokenType::LBracket)) 
			{
				if (source.Test(TTokenType::Value, TValue::Int)) 
				{
					//dim.push_back(source.Int());
					source.GetToken();
				}
				source.GetToken(TTokenType::RBracket);
			}
			else if (source.TestAndGet(TTokenType::Operator, TOperator::GetArrayElement)) 
			{
				//dim.push_back(0);
			} 
			else
				break;
		}
	}
}