#pragma once

#include "notOptimizedProgram.h"
#include "lexer.h"
#include <vector>

class TClass;
class TSClassField;
class TFormalParam;
class TSClass;
class TSMethod;
class TTemplateRealizations;

class TSyntaxAnalyzer
{
public:
	TLexer lexer;
	TClass* base_class;
	TSClass* sem_base_class;
	TTemplateRealizations* templates;
public:
	TSyntaxAnalyzer():base_class(NULL){}
	~TSyntaxAnalyzer();
	void Compile(char* use_source,TTime& time);
	TSMethod* GetMethod(char* use_method);
	TSClassField* GetStaticField(char* use_var);
};
