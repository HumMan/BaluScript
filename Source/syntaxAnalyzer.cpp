#include "SyntaxAnalyzer.h"

#include "Syntax/Class.h"
#include "Syntax/Method.h"
#include "Syntax/Statements.h"

#include "Semantic/SClass.h"
#include "Semantic/SStatements.h"

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


	sem_base_class = new TSClass(NULL,base_class);

	
	sem_base_class->Build();

	sem_base_class->CreateInternalClasses();
	sem_base_class->LinkSignature(&static_fields, &static_variables);
	sem_base_class->LinkBody(&static_fields, &static_variables);
	std::vector<TSClass*> owners;
	sem_base_class->CalculateSizes(owners);
	sem_base_class->CalculateMethodsSizes();

	printf("Syntax analyzing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
}


TSMethod* TSyntaxAnalyzer::GetMethod(char* use_method)
{
	lexer.ParseSource(use_method);
	TMethod* method_decl_syntax = new TMethod(base_class);
	method_decl_syntax->AnalyzeSyntax(lexer, false);
	TSMethod* method_decl = new TSMethod(sem_base_class, method_decl_syntax);
	method_decl->Build();
	//method_decl->LinkBody(&static_fields, &static_variables);
	std::vector<TSMethod*> methods;
	TSMethod* method = NULL;
	switch (method_decl->GetSyntax()->GetMemberType())
	{
	case TClassMember::Func:
		method_decl->GetOwner()->GetMethods(methods, method_decl->GetSyntax()->GetName());
		break;
	case TClassMember::Constr:
		method_decl->GetOwner()->GetConstructors(methods);
		break;
	case TClassMember::Destr:
		method = method_decl->GetOwner()->GetDestructor();
		break;
	case TClassMember::Operator:
		method_decl->GetOwner()->GetOperators(methods, method_decl->GetSyntax()->GetOperatorType());
		break;
	case TClassMember::Conversion:
		method = method_decl->GetOwner()->GetConversion(method_decl->GetParam(0)->GetSyntax()->IsRef(), method_decl->GetRetClass());
		break;
	default:assert(false);
	}
	TClassMember::Enum temp = method_decl->GetSyntax()->GetMemberType();
	if (temp == TClassMember::Func ||
		temp == TClassMember::Constr ||
		temp == TClassMember::Operator)
	{
		for (int i = 0; i<methods.size(); i++)
		{
			if (method_decl->GetParamsCount() == 0 && methods[i]->GetParamsCount() == 0){
				method = methods[i];
				break;
			}
			if (method_decl->GetParamsCount() != methods[i]->GetParamsCount())continue;
			for (int k = 0; k < method_decl->GetParamsCount(); k++)
			{
				if (!methods[i]->GetParam(k)->IsEqualTo(*method_decl->GetParam(k)))
					goto end_search;
			}
			method = methods[i];
			i = methods.size(); end_search:continue;
		}
	}

	if (method != NULL)
	{
		if (method_decl->GetSyntax()->IsStatic() != method->GetSyntax()->IsStatic())
			lexer.Error("Метод отличается по статичности!");
		if (method_decl->GetSyntax()->IsExternal() != method->GetSyntax()->IsExternal())
			lexer.Error("Несоответствует классификатор extern!");
		if (method_decl->GetRetClass() != method->GetRetClass()
			|| method_decl->GetSyntax()->IsReturnRef() != method->GetSyntax()->IsReturnRef())
			lexer.Error("Метод возвращает другое значение!");
		delete method_decl;
		return method;
	}
	else
		lexer.Error("Такого метода не существует!");
	delete method_decl;
	return NULL;
}

TSClassField* TSyntaxAnalyzer::GetStaticField(char* use_var)
{
	lexer.ParseSource(use_var);
	if (lexer.NameId() != base_class->GetName())
		lexer.Error("Ожидалось имя класса!");
	lexer.GetToken();
	TSClassField* result = NULL;
	TSClass* curr_class = sem_base_class;
	while (lexer.Test(TTokenType::Dot))
	{
		lexer.GetToken(TTokenType::Dot);
		lexer.TestToken(TTokenType::Identifier);
		TSClass* t = curr_class->GetNested(lexer.NameId());
		if (t == NULL)
		{
			result = curr_class->GetField(lexer.NameId(), true);
		}
		else curr_class = t;
	}
	if (result == NULL)lexer.Error("Статического члена класса с таким именем не существует!");
	//if(!result->IsStatic())lexer->Error("Член класса с таким именем не является статическим!");
	return result;
}


