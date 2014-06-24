#pragma once

class TClass;
class TVariable;
class TMethod;

struct TOpArray
{
	TListItem<TOp>* first;
	TListItem<TOp>* last;
	TOpArray():first(NULL),last(NULL){}
	bool IsNull()const
	{
		assert((first==NULL)==(last==NULL));
		return first==NULL;
	}
	TOpArray operator+(const TOpArray& use_right)const
	{
		if(first==NULL)return use_right;
		if(use_right.first==NULL)return *this;

		last->next=use_right.first;
		use_right.first->prev=last;
		TOpArray result;
		result.first=first;
		result.last=use_right.last;
		return result;
	}
	void operator+=(const TOpArray& use_right)
	{
		assert((first==NULL)==(last==NULL));
		*this=*this+use_right;
	}
};

#include "notOptimizedProgram.h"

class TTokenPos
{
	int token_id;
public:
	TLexer* source;
	void InitPos(TLexer& use_source)
	{
		source=&use_source;
		token_id=use_source.GetCurrentToken();
	}
	void Error(char* s,...)
	{
		va_list args;
        va_start(args, s);
		source->Error(s,token_id,args);
	}
};

#include "type.h"

class TStatements;
class TLocalVar;
#include "expression.h"

//TODO из-за statements тормоза при большом количестве кода вида, видимо приходится просматривать все statements имеющиеся до
//{
//TDynArray<int> dyn_test,dyn_test2;
//TDynArray<TDynArray<int>> dyn_dyn_test;
//}
//{
//TDynArray<int> dyn_test,dyn_test2;
//TDynArray<TDynArray<int>> dyn_dyn_test;
//}
//{
//TDynArray<int> dyn_test,dyn_test2;
//TDynArray<TDynArray<int>> dyn_dyn_test;
//}

#include "LocalVar.h"

class TTemplateRealizations
{
public:
	struct TTemplateRealization
	{
		TClass* template_pointer;
		TVectorList<TClass> realizations;
	};
	TVectorList<TTemplateRealization> templates;
	TVectorList<TClass>* FindTemplate(TClass* use_template)
	{
		for(int i=0;i<=templates.GetHigh();i++)
			if(templates[i]->template_pointer==use_template)
				return &templates[i]->realizations;
		return NULL;
	}
};

#include "class.h"

void ValidateAccess(TTokenPos* field_pos,TClass* source,TClassField* target);
void ValidateAccess(TTokenPos* field_pos,TClass* source,TMethod* target);

TMethod* FindMethod(TTokenPos* source,TVector<TMethod*> &methods_to_call,TVector<TFormalParam> &formal_params,int& conv_needed);

inline bool IsEqualClasses(TFormalParam formal_par,TClass* param_class,bool param_ref,int& need_conv);

class TSyntaxAnalyzer
{
public:
	TLexer lexer;
	TClass* base_class;
	TNotOptimizedProgram program;
	TTemplateRealizations templates;
public:
	TSyntaxAnalyzer():base_class(NULL){}
	~TSyntaxAnalyzer()
	{
		if(base_class!=NULL)
			delete base_class;
	}
	void Compile(char* use_source,TTime& time);
	void GetProgram(TProgram& use_program,TTime& time)
	{
		program.CreateOptimizedProgram(use_program,time);
	}
	int GetMethod(char* use_method);
	TClassField* GetStaticField(char* use_var);
};
