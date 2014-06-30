#include "SyntaxAnalyzer.h"

#include "Syntax/Class.h"
#include "Syntax/Method.h"
#include "Syntax/Statements.h"

TSyntaxAnalyzer::~TSyntaxAnalyzer()
{
	if (base_class != NULL)
		delete base_class;
}

void ValidateAccess(TTokenPos* field_pos, TClass* source, TClassField* target)
{
	if (target->GetAccess() == TTypeOfAccess::Public)return;
	if (source == target->GetOwner())return;
	if (target->GetAccess() == TTypeOfAccess::Protected&&!source->IsChildOf(target->GetOwner()))
		field_pos->Error("Данное поле класса доступно только из классов наследников (protected)!");
	else if (target->GetAccess() == TTypeOfAccess::Private&&source != target->GetOwner())
		field_pos->Error("Данное поле класса доступно только из класса в котором оно объявлено (private)!");
}

void ValidateAccess(TTokenPos* field_pos, TClass* source, TMethod* target)
{
	if (target->GetAccess() == TTypeOfAccess::Public)return;
	if (source == target->GetOwner())return;
	if (target->GetAccess() == TTypeOfAccess::Protected&&!source->IsChildOf(target->GetOwner()))
		field_pos->Error("Данный метод доступен только из классов наследников (protected)!");
	else if (target->GetAccess() == TTypeOfAccess::Private&&source != target->GetOwner())
		field_pos->Error("Данный метод доступен только из класса в котором он объявлен (private)!");
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
	printf("Syntax analyzing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
}



