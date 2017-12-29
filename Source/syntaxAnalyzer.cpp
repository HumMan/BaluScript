#include "syntaxAnalyzer.h"

#include "NativeTypes/DynArray.h"
#include "NativeTypes/StaticArray.h"
#include "NativeTypes/String.h"

using namespace Lexer;

class TSyntaxAnalyzer::TPrivate
{
public:
	std::unique_ptr<Lexer::ILexer> lexer;
	SyntaxApi::IClass* base_class;
	SemanticApi::ISClass* sem_base_class;
	std::vector<SemanticApi::ISClassField*> static_fields;
	std::vector<SemanticApi::ISLocalVar*> static_variables;
};

TSyntaxAnalyzer::TSyntaxAnalyzer():_this(std::make_unique<TPrivate>())
{
	_this->lexer.reset(Lexer::ILexer::Create());
}

TSyntaxAnalyzer::~TSyntaxAnalyzer()
{
	SyntaxApi::Destroy(_this->base_class);
}

SyntaxApi::IClass * TSyntaxAnalyzer::GetBaseClass() const
{
	return _this->base_class;
}

SemanticApi::ISClass* TSyntaxAnalyzer::GetCompiledBaseClass()const
{
	return _this->sem_base_class;
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
	_this->base_class=SyntaxApi::Analyze(_this->lexer.get());

	std::vector<SemanticApi::TExternalClassDecl> external_classes;
	external_classes.push_back(TDynArr::DeclareExternalClass());
	external_classes.push_back(TStaticArr::DeclareExternalClass());
	external_classes.push_back(TString::DeclareExternalClass());

	_this->sem_base_class = SemanticApi::SAnalyze(_this->lexer.get(), _this->base_class, external_classes,
		SemanticApi::TGlobalBuildContext(&_this->static_fields, &_this->static_variables));

	//printf("Syntax analyzing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
}

SemanticApi::ISMethod* TSyntaxAnalyzer::GetMethod(char* use_method)
{
	//TODO
		/*_this->lexer->ParseSource(use_method);
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
	return nullptr;*/
	return nullptr;
}

SemanticApi::ISClassField* TSyntaxAnalyzer::GetStaticField(char* use_var)
{
	//TODO
	/*_this->lexer->ParseSource(use_var);
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
	return result;*/
	return nullptr;
}


