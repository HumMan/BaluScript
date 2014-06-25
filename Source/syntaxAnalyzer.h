#pragma once

#include "notOptimizedProgram.h"
#include "Syntax/TemplateRealizations.h"
#include "lexer.h"
#include <vector>

class TClass;
class TClassField;
class TMethod;
class TFormalParam;

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



void ValidateAccess(TTokenPos* field_pos,TClass* source,TClassField* target);
void ValidateAccess(TTokenPos* field_pos,TClass* source,TMethod* target);

TMethod* FindMethod(TTokenPos* source,std::vector<TMethod*> &methods_to_call,const std::vector<TFormalParam> &formal_params,int& conv_needed);

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
	~TSyntaxAnalyzer();
	void Compile(char* use_source,TTime& time);
	void GetProgram(TProgram& use_program, TTime& time);
	int GetMethod(char* use_method);
	TClassField* GetStaticField(char* use_var);
};
