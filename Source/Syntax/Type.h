#pragma once 

#include "../lexer.h"
#include "../notOptimizedProgram.h"

class TClass;

struct TType:public TTokenPos
{
	friend class TTypes;
	struct TClassName
	{
		TNameId name;
		TClass* class_pointer;
		TVectorList<TClassName> template_params;
		TSmartPointer<TClassName> member;
		TClass* Build(bool use_build_methods,TClass* use_curr_class,TClass* owner,TTokenPos& source,TNotOptimizedProgram* program);
		TClassName():class_pointer(NULL){}
	};
private:
	TClass* owner;
	TVector<int> dim;
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