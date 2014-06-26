#pragma once 

#include "../lexer.h"
#include <memory>

class TClass;
class TNotOptimizedProgram;

struct TType:public TTokenPos
{
	struct TClassName
	{
		///<summary>Текстовый идентификатор имени класса</summary>
		TNameId name;
		TClass* class_pointer;
		///<summary>Список шаблонных параметров. Например: TestType&lt;int,TDecl&gt; s;</summary>
		std::vector<std::shared_ptr<TClassName>> template_params;
		///<summary>Список измерений статических массивов (0-динамический массив). Например: int[2][][6] variable;</summary>
		std::vector<int> dimensions;
		std::shared_ptr<TClassName> member;
		TClass* Build(bool use_build_methods,TClass* use_curr_class,TClass* owner,TTokenPos& source,TNotOptimizedProgram* program);
		TClassName():class_pointer(NULL){}
	};
private:
	TClass* owner;
	
	TClassName class_name;
	TClass* class_pointer;
	void AnalyzeSyntax(TClassName* use_class_name,TLexer& source);
public:
	bool IsEqualTo(const TType& use_right)const;
	void InitOwner(TClass* use_owner);
	void SetAs(TClass* use_class_pointer);
	TClass* GetClass(bool use_build_methods=false,TNotOptimizedProgram* program=NULL);
	TType(TClass* use_owner);
	TType(TNameId use_class_name,TClass* use_owner);
	void AnalyzeSyntax(TLexer& source);
};