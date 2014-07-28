#pragma once

#include "lexer.h"
#include <vector>

class TClass;
class TSClassField;
class TFormalParam;
class TSClass;
class TSMethod;
class TTemplateRealizations;
class TSLocalVar;

class TSyntaxAnalyzer
{
public:
	TLexer lexer;
	TClass* base_class;
	TSClass* sem_base_class;
	std::vector<TSClassField*> static_fields;
	std::vector<TSLocalVar*> static_variables;
public:
	TSyntaxAnalyzer():base_class(NULL){}
	~TSyntaxAnalyzer();
	void Compile(char* use_source,TTime& time);
	TSMethod* GetMethod(char* use_method);
	TSClassField* GetStaticField(char* use_var);
};
