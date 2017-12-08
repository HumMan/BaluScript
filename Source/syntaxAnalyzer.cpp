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

class TSyntaxAnalyzer::TPrivate
{
public:
	std::unique_ptr<Lexer::ILexer> lexer;
	std::unique_ptr<SyntaxInternal::TClass> base_class;
	std::unique_ptr<TSClass> sem_base_class;
	std::vector<TSClassField*> static_fields;
	std::vector<TSLocalVar*> static_variables;
};

TSyntaxAnalyzer::TSyntaxAnalyzer():_this(std::make_unique<TPrivate>())
{
	_this->lexer.reset(Lexer::ILexer::Create());
}

TSyntaxAnalyzer::~TSyntaxAnalyzer()
{
}

SyntaxApi::IClass* TSyntaxAnalyzer::GetBaseClass()const
{
	return GetBaseClass2();
}
SyntaxInternal::TClass * TSyntaxAnalyzer::GetBaseClass2() const
{
	return _this->base_class.get();
}
TSClass* TSyntaxAnalyzer::GetCompiledBaseClass()const
{
	return _this->sem_base_class.get();
}
Lexer::ILexer* TSyntaxAnalyzer::GetLexer()const
{
	return _this->lexer.get();
}

void TSyntaxAnalyzer::Compile(char* use_source/*, TTime& time*/)
{
	//unsigned long long t = time.GetTime();
	_this->lexer->ParseSource(use_source);
	//printf("Source parsing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
	//t = time.GetTime();
	_this->base_class.reset(new SyntaxInternal::TClass(nullptr));

	_this->base_class->AnalyzeSyntax(_this->lexer.get());
	_this->lexer->GetToken(Lexer::TTokenType::Done);

	_this->sem_base_class.reset(new TSClass(nullptr, _this->base_class.get()));

	_this->sem_base_class->Build();

	TDynArr::DeclareExternalClass(this);
	TStaticArr::DeclareExternalClass(this);
	TString::DeclareExternalClass(this);

	CreateInternalClasses();
	_this->sem_base_class->LinkSignature(TGlobalBuildContext(&_this->static_fields, &_this->static_variables));
	_this->sem_base_class->InitAutoMethods();
	_this->sem_base_class->LinkBody(TGlobalBuildContext(&_this->static_fields, &_this->static_variables));

	std::vector<TSClass*> owners;
	_this->sem_base_class->CalculateSizes(owners);
	_this->sem_base_class->CalculateMethodsSizes();

	//printf("Syntax analyzing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
}

void TSyntaxAnalyzer::CreateInternalClasses()
{
	TNameId name_id = _this->lexer->GetIdFromName("dword");
	SyntaxInternal::TClass* t_syntax = new SyntaxInternal::TClass(_this->base_class.get());
	_this->base_class->AddNested(t_syntax);
	t_syntax->SetName(name_id);
	TSClass* t = new TSClass(_this->sem_base_class.get(), t_syntax);
	t->SetSize(1);
	t->SetSignatureLinked();
	t->SetBodyLinked();
	_this->sem_base_class->AddClass(t);
}

TSMethod* TSyntaxAnalyzer::GetMethod(char* use_method)
{
	_this->lexer->ParseSource(use_method);
	std::unique_ptr<SyntaxInternal::TMethod> method_decl_syntax(new SyntaxInternal::TMethod(_this->base_class.get()));
	method_decl_syntax->AnalyzeSyntax(_this->lexer.get(), false);
	_this->lexer->GetToken(TTokenType::Done);
	std::unique_ptr<TSMethod> method_decl(new TSMethod(_this->sem_base_class->GetNestedByFullName(method_decl_syntax->GetOwner()->GetFullClassName(),1), method_decl_syntax.get()));
	method_decl->Build();
	std::vector<TSMethod*> methods;
	TSMethod* method = nullptr;
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
			for (size_t k = 0; k < method_decl->GetParamsCount(); k++)
			{
				if (!methods[i]->GetParam(k)->IsEqualTo(*method_decl->GetParam(k)))
					goto end_search;
			}
			method = methods[i];
			i = methods.size(); end_search:continue;
		}
	}

	if (method != nullptr)
	{
		if (method_decl->GetSyntax()->IsStatic() != method->GetSyntax()->IsStatic())
			_this->lexer->Error("Метод отличается по статичности!");
		if (method_decl->GetSyntax()->IsExternal() != method->GetSyntax()->IsExternal())
			_this->lexer->Error("Несоответствует классификатор extern!");
		if (method_decl->GetRetClass() != method->GetRetClass()
			|| method_decl->GetSyntax()->IsReturnRef() != method->GetSyntax()->IsReturnRef())
			_this->lexer->Error("Метод возвращает другое значение!");
		return method;
	}
	else
		_this->lexer->Error("Такого метода не существует!");
	return nullptr;
}

TSClassField* TSyntaxAnalyzer::GetStaticField(char* use_var)
{
	_this->lexer->ParseSource(use_var);
	if (_this->lexer->NameId() != _this->base_class->GetName())
		_this->lexer->Error("Ожидалось имя класса!");
	_this->lexer->GetToken();
	TSClassField* result = nullptr;
	TSClass* curr_class = _this->sem_base_class.get();
	while (_this->lexer->Test(TTokenType::Dot))
	{
		_this->lexer->GetToken(TTokenType::Dot);
		_this->lexer->TestToken(TTokenType::Identifier);
		TSClass* t = curr_class->GetNested(_this->lexer->NameId());
		if (t == nullptr)
		{
			result = curr_class->GetField(_this->lexer->NameId(), true);
		}
		else
			curr_class = t;
		_this->lexer->GetToken();
	}
	if (result == nullptr)_this->lexer->Error("Статического члена класса с таким именем не существует!");
	_this->lexer->GetToken(TTokenType::Done);
	return result;
}


