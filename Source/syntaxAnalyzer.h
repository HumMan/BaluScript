#pragma once

#include "lexer.h"
#include <vector>
#include <memory>

class TClass;
class TSClassField;
class TExpressionResult;
class TSClass;
class TSMethod;
class TTemplateRealizations;
class TSLocalVar;

class TSyntaxAnalyzer
{
public:
	TLexer lexer;
	std::unique_ptr<TClass> base_class;
	std::unique_ptr<TSClass> sem_base_class;
	std::vector<TSClassField*> static_fields;
	std::vector<TSLocalVar*> static_variables;
public:
	TSyntaxAnalyzer();
	~TSyntaxAnalyzer();
	void Compile(char* use_source,TTime& time);
	void CreateInternalClasses();
	TSMethod* GetMethod(char* use_method);
	TSClassField* GetStaticField(char* use_var);
};
