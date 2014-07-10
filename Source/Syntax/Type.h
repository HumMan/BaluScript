#pragma once 

#include "../lexer.h"
#include <memory>
#include <list>

class TClass;

struct TType:public TTokenPos
{
	struct TClassName
	{
		///<summary>Текстовый идентификатор имени класса</summary>
		TNameId name;
		///<summary>Список шаблонных параметров. Например: TestType&lt;int,TDecl&gt; s;</summary>
		std::list<TType> template_params;
		///<summary>Список измерений статических массивов (0-динамический массив). Например: int[2][][6] variable;</summary>
		std::vector<int> dimensions;
	};
private:
	TClass* owner;
	
	std::list<TClassName> class_names;
	
	void AnalyzeClassName(TLexer& source);
	void AnalyzeDimensions(TLexer& source);
public:
	std::list<TClassName>& GetClassNames()
	{
		return class_names;
	}
	TType(TClass* use_owner);
	TType(TNameId use_class_name, TClass* use_owner);
	void AnalyzeSyntax(TLexer& source);
};