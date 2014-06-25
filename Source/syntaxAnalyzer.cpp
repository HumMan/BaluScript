#include "SyntaxAnalyzer.h"

#include "Syntax/Class.h"
#include "Syntax/Method.h"
#include "Syntax/Statements.h"

TSyntaxAnalyzer::~TSyntaxAnalyzer()
{
	if (base_class != NULL)
		delete base_class;
}
void TSyntaxAnalyzer::GetProgram(TProgram& use_program, TTime& time)
{
	program.CreateOptimizedProgram(use_program, time);
}



