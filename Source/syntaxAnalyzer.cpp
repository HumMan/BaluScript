#include "SyntaxAnalyzer.h"

#include "Syntax/Class.h"
#include "Syntax/Method.h"
#include "Syntax/Statements.h"

#include "Semantic/SClass.h"

TSyntaxAnalyzer::~TSyntaxAnalyzer()
{
	if (base_class != NULL)
		delete base_class;
}

void TSyntaxAnalyzer::Compile(char* use_source, TTime& time)
{
	unsigned long long t = time.GetTime();
	lexer.ParseSource(use_source);
	printf("Source parsing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
	t = time.GetTime();
	base_class = new TClass(NULL);
	base_class->InitPos(lexer);

	base_class->AnalyzeSyntax(lexer);

	TTemplateRealizations templates;

	TSClass* sem_base_class = new TSClass(NULL,&templates,base_class);
	sem_base_class->Build();
	sem_base_class->Link();

	printf("Syntax analyzing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
}



