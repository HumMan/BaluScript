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
		std::list<TClassName> template_params;
		///<summary>Список измерений статических массивов (0-динамический массив). Например: int[2][][6] variable;</summary>
		std::vector<int> dimensions;
		std::unique_ptr<TClassName> member;
		TClassName(){}
		TClassName(const TClassName& use_copy)
		{
			Assign(use_copy);
		}
		void Assign(const TClassName& use_copy)
		{
			name = use_copy.name;
			template_params = use_copy.template_params;
			dimensions = use_copy.dimensions;
			if (use_copy.member)
				member = std::unique_ptr<TClassName>(new TClassName(*use_copy.member.get()));
		}
		void operator=(const TClassName& right)
		{
			Assign(right);
		}
	};
private:
	TClass* owner;
	
	TClassName class_name;
	void AnalyzeSyntax(TClassName* use_class_name,TLexer& source);
public:
	TClassName& GetClassName()
	{
		return class_name;
	}
	TType(TClass* use_owner);
	TType(TNameId use_class_name, TClass* use_owner);
	void AnalyzeSyntax(TLexer& source);
};