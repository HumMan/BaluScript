#pragma once

#include "notOptimizedProgram.h"
#include "lexer.h"
#include <vector>

class TClass;
class TClassField;
class TMethod;
class TFormalParam;

void ValidateAccess(TTokenPos* field_pos,TClass* source,TClassField* target);
void ValidateAccess(TTokenPos* field_pos,TClass* source,TMethod* target);

class TSyntaxAnalyzer
{
public:
	TLexer lexer;
	TClass* base_class;
public:
	TSyntaxAnalyzer():base_class(NULL){}
	~TSyntaxAnalyzer();
	void Compile(char* use_source,TTime& time);
	int GetMethod(char* use_method);
	TClassField* GetStaticField(char* use_var);
};
