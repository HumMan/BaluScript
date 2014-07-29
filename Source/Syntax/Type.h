#pragma once 

#include "../lexer.h"
#include <memory>
#include <list>

class TClass;
struct TType_TTemplateParameter;

struct TType_TClassName
{
	///<summary>Текстовый идентификатор имени класса</summary>
	TNameId name;
	///<summary>Список шаблонных параметров. Например: TestType&lt;int,TDecl&gt; s;</summary>
	std::list<TType_TTemplateParameter> template_params;
	///<summary>Список измерений статических массивов ((-1)-динамический массив). Например: int[2][][6] variable;</summary>
	std::vector<int> dimensions;
	///<summary>Преобразование списка измерений в вызов соответствующих шаблонов</summary>
	void ConvertDimensionsToTemplates();
};

struct TType:public TTokenPos
{

private:
	TClass* owner;
	
	std::list<TType_TClassName> class_names;
	
	void AnalyzeClassName(TLexer& source);
	void AnalyzeDimensions(TLexer& source);
public:
	std::list<TType_TClassName>& GetClassNames()
	{
		return class_names;
	}
	TType(TClass* use_owner);
	TType(TNameId use_class_name, TClass* use_owner);
	void AnalyzeSyntax(TLexer& source);
};

struct TType_TTemplateParameter
{
	//в качестве шаблонных параметров могут использоваться значения int
	bool is_value;
	int value;
	std::unique_ptr<TType> type;
};
