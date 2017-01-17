#pragma once 

#include "../lexer.h"
#include <memory>
#include <list>

class TClass;
struct TType_TTemplateParameter;

struct TType_TClassName
{
	///<summary>Текстовый идентификатор имени класса</summary>
	Lexer::TNameId name;
	///<summary>Список шаблонных параметров. Например: TestType&lt;int,TDecl&gt; s;</summary>
	std::list<TType_TTemplateParameter> template_params;
	///<summary>Список измерений статических массивов ((-1)-динамический массив). Например: int[2][][6] variable;</summary>
	std::vector<int> dimensions;
	///<summary>Преобразование списка измерений в вызов соответствующих шаблонов</summary>
	void ConvertDimensionsToTemplates();
};

class TType:public Lexer::TTokenPos
{

private:
	TClass* owner;
	
	std::list<TType_TClassName> class_names;
	
	void AnalyzeClassName(Lexer::ILexer* source);
	void AnalyzeDimensions(Lexer::ILexer* source);
public:
	std::list<TType_TClassName> GetClassNames() const
	{
		return class_names;
	}
	TType(TClass* use_owner);
	TType(Lexer::TNameId use_class_name, TClass* use_owner);
	void AnalyzeSyntax(Lexer::ILexer* source);
};

struct TType_TTemplateParameter
{
	//в качестве шаблонных параметров могут использоваться значения int
	bool is_value;
	int value;
	std::shared_ptr<TType> type;
};
