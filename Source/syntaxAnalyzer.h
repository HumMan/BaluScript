#pragma once

#include "lexer.h"
#include <vector>
#include <memory>

namespace SyntaxInternal
{
	class TClass;
}

class TSClassField;
class TSClass;
class TSMethod;
class TTemplateRealizations;
class TSLocalVar;

class TSyntaxAnalyzer
{
	std::unique_ptr<Lexer::ILexer> lexer;
	std::unique_ptr<SyntaxInternal::TClass> base_class;
	std::unique_ptr<TSClass> sem_base_class;
	std::vector<TSClassField*> static_fields;
	std::vector<TSLocalVar*> static_variables;
public:
	SyntaxInternal::TClass* GetBaseClass()const;
	TSClass* GetCompiledBaseClass()const;
	Lexer::ILexer* GetLexer()const;
	TSyntaxAnalyzer();
	~TSyntaxAnalyzer();
	void Compile(char* use_source/*,TTime& time*/);
	void CreateInternalClasses();
	TSMethod* GetMethod(char* use_method);
	TSClassField* GetStaticField(char* use_var);

};

class ISyntaxAnalyzer
{

};
