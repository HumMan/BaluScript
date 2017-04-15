#pragma once 

#include "../SyntaxTree/SyntaxTreeApi.h"

namespace SyntaxInternal
{

	class TClass;
	class TType_TTemplateParameter;

	class TType_TClassName : public SyntaxApi::IType_TClassName
	{
		///<summary>Текстовый идентификатор имени класса</summary>
		Lexer::TNameId name;
		///<summary>Список шаблонных параметров. Например: TestType&lt;int,TDecl&gt; s;</summary>
		std::vector<std::shared_ptr<TType_TTemplateParameter>> template_params;
		///<summary>Список измерений статических массивов ((-1)-динамический массив). Например: int[2][][6] variable;</summary>
		std::vector<int> dimensions;
		///<summary>Преобразование списка измерений в вызов соответствующих шаблонов</summary>
		void ConvertDimensionsToTemplates();
	public:
		TType_TClassName(Lexer::TNameId name);
		Lexer::TNameId GetName()const;
		void AddDimension(int dimension);
		void AddTemplateParameter(const TType_TTemplateParameter& param);
		std::vector<int> GetDimensions()const;
		std::vector<std::shared_ptr<SyntaxApi::IType_TTemplateParameter>> GetTemplateParams() const;
	};

	class TType :public Lexer::TTokenPos, public SyntaxApi::IType
	{
		TClass* owner;

		std::vector<std::shared_ptr<TType_TClassName>> class_names;

		void AnalyzeClassName(Lexer::ILexer* source);
		void AnalyzeDimensions(Lexer::ILexer* source);
	public:
		std::vector<std::shared_ptr<SyntaxApi::IType_TClassName>> GetClassNames() const;
		std::vector<std::shared_ptr<TType_TClassName>> GetClassNamesT() const;
		TType(TClass* use_owner);
		TType(Lexer::TNameId use_class_name, TClass* use_owner);
		void AnalyzeSyntax(Lexer::ILexer* source);
	};

	class TType_TTemplateParameter : public SyntaxApi::IType_TTemplateParameter
	{
	private:
		//в качестве шаблонных параметров могут использоваться значения int
		bool is_value;
		int value;
		std::shared_ptr<TType> type;
	public:
		TType_TTemplateParameter(bool is_value, int value, TType* type);
		bool IsValue()const;
		int GetValue()const;
		TType* GetType()const;
	};

}