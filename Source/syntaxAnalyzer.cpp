#include "syntaxAnalyzer.h"

#include "NativeTypes/base_types.h"

#include "NativeTypes/DynArray.h"
#include "NativeTypes/StaticArray.h"
#include "NativeTypes/String.h"

#include "TreeRunner/ExpressionRun.h"

#include "NativeTypes/vec.h"
#include "TreeRunner/TreeRunner.h"

#include "../Data/extern_binding.h"

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

ISyntaxAnalyzer* ISyntaxAnalyzer::Create()
{
	return new TSyntaxAnalyzer();
}

void ISyntaxAnalyzer::Destroy(ISyntaxAnalyzer* v)
{
	delete v;
}

TSyntaxAnalyzer::TSyntaxAnalyzer()
	:_this(std::unique_ptr<TPrivate>(new TPrivate()))
{
	_this->lexer.reset(Lexer::ILexer::Create());
	_this->base_class = nullptr;
	_this->sem_base_class = nullptr;
}

TSyntaxAnalyzer::~TSyntaxAnalyzer()
{
	SyntaxApi::Destroy(_this->base_class);
	SemanticApi::SDestroy(_this->sem_base_class);
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

void TSyntaxAnalyzer::Compile(const char* use_source)
{
	std::vector<SemanticApi::TExternalClassDecl> _external_classes;
	std::vector<SemanticApi::TExternalSMethod> _external_bindings;
	Compile(use_source, _external_classes, _external_bindings);
}

void TSyntaxAnalyzer::Compile(const char* use_source, std::vector<SemanticApi::TExternalClassDecl> _external_classes, std::vector<SemanticApi::TExternalSMethod> _external_bindings)
{
	//unsigned long long t = time.GetTime();
	_this->lexer->ParseSource(("class Script{" + std::string(base_types) + use_source + "}").c_str());
	//printf("Source parsing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
	//t = time.GetTime();
	_this->base_class=SyntaxApi::Analyze(_this->lexer.get());

	std::vector<SemanticApi::TExternalClassDecl> external_classes;
	external_classes.push_back(TDynArr::DeclareExternalClass());
	external_classes.push_back(TStaticArr::DeclareExternalClass());
	external_classes.push_back(TString::DeclareExternalClass());

	for(auto& v: _external_classes)
		external_classes.push_back(v);

	auto external_classes_bindings = ns_Script::Register();
	for (auto& v : _external_bindings)
		external_classes_bindings.push_back(v);

	_this->sem_base_class = SemanticApi::SAnalyze(_this->lexer.get(), _this->base_class, external_classes, external_classes_bindings,
		SemanticApi::TGlobalBuildContext(&_this->static_fields, &_this->static_variables));

	//printf("Syntax analyzing = %.3f ms\n", time.TimeDiff(time.GetTime(), t) * 1000);
}

class TMethodPointer
{
	SyntaxApi::IMethod* method;
public:
	TMethodPointer(SyntaxApi::IMethod* method)
	{
		this->method = method;
	}
	~TMethodPointer()
	{
		SyntaxApi::Destroy(method);
	}
	SyntaxApi::IMethod* operator->()
	{
		return method;
	}
	SyntaxApi::IMethod* get()
	{
		return method;
	}
};

class TSMethodPointer
{
	SemanticApi::ISMethod* method;
public:

	TSMethodPointer(SemanticApi::ISMethod* method)
	{
		this->method = method;
	}
	~TSMethodPointer()
	{
		SemanticApi::SDestroyMethodSignature(method);
	}
	SemanticApi::ISMethod* operator->()
	{
		return method;
	}
};

SemanticApi::ISMethod* TSyntaxAnalyzer::GetMethod(const char* use_method)
{
	_this->lexer->ParseSource(use_method);
	TMethodPointer method_decl_syntax(SyntaxApi::AnalyzeMethodSignature(_this->lexer.get(), _this->base_class));

	TSMethodPointer method_decl(SemanticApi::SAnalyzeMethodSignature(_this->lexer.get(), method_decl_syntax.get(), _this->sem_base_class));

	std::vector<SemanticApi::ISMethod*> methods;
	SemanticApi::ISMethod* method = nullptr;
	switch (method_decl->GetMemberType())
	{
	case SyntaxApi::TClassMember::Func:
		method_decl->GetOwner()->GetMethods(methods, method_decl_syntax->GetName());
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
		method_decl->GetOwner()->GetOperators(methods, method_decl->GetOperatorType());
		break;
	case SyntaxApi::TClassMember::Conversion:
		method = method_decl->GetOwner()->GetConversion(method_decl->GetParam(0)->IsRef(), method_decl->GetRetClass());
		break;
	default:assert(false);
	}
	SyntaxApi::TClassMember temp = method_decl->GetMemberType();
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
				if (!methods[i]->GetParam(k)->IIsEqualTo(method_decl->GetParam(k)))
					goto end_search;
			}
			method = methods[i];
			i = methods.size(); end_search:continue;
		}
	}

	if (method != nullptr)
	{
		if (method_decl->IsStatic() != method->IsStatic())
			throw std::runtime_error("Метод отличается по статичности!");
		if (method_decl->IsExternal() != method->IsExternal())
			throw std::runtime_error("Несоответствует классификатор extern!");
		auto c0 = method_decl->GetRetClass();
		auto c1 = method->GetRetClass();
		if ( c0 != c1
			|| method_decl->IsReturnRef() != method->IsReturnRef())
			throw std::runtime_error("Метод возвращает другое значение!");
		return method;
	}
	else
		throw new std::runtime_error("Такого метода не существует!");
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

std::vector<SemanticApi::ISClassField*> TSyntaxAnalyzer::GetStaticFields() const
{
	return _this->static_fields;
}

std::vector<SemanticApi::ISLocalVar*> TSyntaxAnalyzer::GetStaticVariables() const
{
	return _this->static_variables;
}

int TSyntaxAnalyzer::GetBindingOffset()
{
	return ns_Script::BindingOffset;
}

int TSyntaxAnalyzer::GetBindingCount()
{
	return ns_Script::BindingCount;
}


