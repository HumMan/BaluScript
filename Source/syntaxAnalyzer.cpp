#include "syntaxAnalyzer.h"

#include "Syntax/Class.h"
#include "Syntax/Method.h"
#include "Syntax/Statements.h"

#include "Semantic/SClass.h"
#include "Semantic/SStatements.h"

#include "NativeTypes/DynArray.h"
#include "NativeTypes/StaticArray.h"
#include "NativeTypes/String.h"

using namespace Lexer;

TSyntaxAnalyzer::TSyntaxAnalyzer()
{
	lexer.reset(Lexer::ILexer::Create());
}

TSyntaxAnalyzer::~TSyntaxAnalyzer()
{
}

TClass* TSyntaxAnalyzer::GetBaseClass()const
{
	return base_class.get();
}
TSClass* TSyntaxAnalyzer::GetCompiledBaseClass()const
{
	return sem_base_class.get();
}
Lexer::ILexer* TSyntaxAnalyzer::GetLexer()const
{
	return lexer.get();
}

void TSyntaxAnalyzer::Compile(char* use_source/*, TTime& time*/)
{
	//unsigned long long t = time.GetTime();
	lexer->ParseSource(use_source);
	//printf("Source parsing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
	//t = time.GetTime();
	base_class.reset(new TClass(NULL));

	base_class->AnalyzeSyntax(lexer.get());
	lexer->GetToken(Lexer::TTokenType::Done);

	sem_base_class.reset(new TSClass(NULL,base_class.get()));

	sem_base_class->Build();

	TDynArr::DeclareExternalClass(this);
	TStaticArr::DeclareExternalClass(this);
	TString::DeclareExternalClass(this);

	CreateInternalClasses();
	sem_base_class->LinkSignature(TGlobalBuildContext(&static_fields, &static_variables));
	sem_base_class->InitAutoMethods();
	sem_base_class->LinkBody(TGlobalBuildContext(&static_fields, &static_variables));

	std::vector<TSClass*> owners;
	sem_base_class->CalculateSizes(owners);
	sem_base_class->CalculateMethodsSizes();

	//printf("Syntax analyzing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
}

void TSyntaxAnalyzer::CreateInternalClasses()
{
	TNameId name_id = lexer->GetIdFromName("dword");
	TClass* t_syntax = new TClass(base_class.get());
	base_class->AddNested(t_syntax);
	t_syntax->SetName(name_id);
	TSClass* t = new TSClass(sem_base_class.get(), t_syntax);
	t->SetSize(1);
	t->SetSignatureLinked();
	t->SetBodyLinked();
	sem_base_class->AddClass(t);
}

TSMethod* TSyntaxAnalyzer::GetMethod(char* use_method)
{
	lexer->ParseSource(use_method);
	std::unique_ptr<TMethod> method_decl_syntax(new TMethod(base_class.get()));
	method_decl_syntax->AnalyzeSyntax(lexer.get(), false);
	lexer->GetToken(TTokenType::Done);
	std::unique_ptr<TSMethod> method_decl(new TSMethod(sem_base_class->GetNestedByFullName(method_decl_syntax->GetOwner()->GetFullClassName(),1), method_decl_syntax.get()));
	method_decl->Build();
	std::vector<TSMethod*> methods;
	TSMethod* method = NULL;
	switch (method_decl->GetSyntax()->GetMemberType())
	{
	case SyntaxApi::TClassMember::Func:
		method_decl->GetOwner()->GetMethods(methods, method_decl->GetSyntax()->GetName());
		break;
	case SyntaxApi::TClassMember::DefaultConstr:
		method = method_decl->GetOwner()->GetDefConstr();
		break;
	case SyntaxApi::TClassMember::CopyConstr:
		method_decl->GetOwner()->GetCopyConstructors(methods);
		break;
	case SyntaxApi::TClassMember::MoveConstr:
		method_decl->GetOwner()->GetMoveConstructors(methods);
		break;
	case SyntaxApi::TClassMember::Destr:
		method = method_decl->GetOwner()->GetDestructor();
		break;
	case SyntaxApi::TClassMember::Operator:
		method_decl->GetOwner()->GetOperators(methods, method_decl->GetSyntax()->GetOperatorType());
		break;
	case SyntaxApi::TClassMember::Conversion:
		method = method_decl->GetOwner()->GetConversion(method_decl->GetParam(0)->GetSyntax()->IsRef(), method_decl->GetRetClass());
		break;
	default:assert(false);
	}
	SyntaxApi::TClassMember temp = method_decl->GetSyntax()->GetMemberType();
	if (temp == SyntaxApi::TClassMember::Func ||
		temp == SyntaxApi::TClassMember::CopyConstr ||
		temp == SyntaxApi::TClassMember::MoveConstr ||
		temp == SyntaxApi::TClassMember::Operator)
	{
		for (size_t i = 0; i<methods.size(); i++)
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
			lexer->Error("Метод отличается по статичности!");
		if (method_decl->GetSyntax()->IsExternal() != method->GetSyntax()->IsExternal())
			lexer->Error("Несоответствует классификатор extern!");
		if (method_decl->GetRetClass() != method->GetRetClass()
			|| method_decl->GetSyntax()->IsReturnRef() != method->GetSyntax()->IsReturnRef())
			lexer->Error("Метод возвращает другое значение!");
		return method;
	}
	else
		lexer->Error("Такого метода не существует!");
	return NULL;
}

TSClassField* TSyntaxAnalyzer::GetStaticField(char* use_var)
{
	lexer->ParseSource(use_var);
	if (lexer->NameId() != base_class->GetName())
		lexer->Error("Ожидалось имя класса!");
	lexer->GetToken();
	TSClassField* result = NULL;
	TSClass* curr_class = sem_base_class.get();
	while (lexer->Test(TTokenType::Dot))
	{
		lexer->GetToken(TTokenType::Dot);
		lexer->TestToken(TTokenType::Identifier);
		TSClass* t = curr_class->GetNested(lexer->NameId());
		if (t == NULL)
		{
			result = curr_class->GetField(lexer->NameId(), true);
		}
		else curr_class = t;
		lexer->GetToken();
	}
	if (result == NULL)lexer->Error("Статического члена класса с таким именем не существует!");
	lexer->GetToken(TTokenType::Done);
	return result;
}


