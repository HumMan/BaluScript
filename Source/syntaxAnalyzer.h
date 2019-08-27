#pragma once

#include "lexer.h"
#include <vector>
#include <memory>

#include "SyntaxInterface/SyntaxTreeApi.h"

#include "SemanticInterface/SemanticTreeApi.h"

enum RuntimeExceptionId
{
	DynArray_UseAfterFree,
	Ptr_UseAfterFree,
	MethodMustReturnValue
};

class RuntimeException: public std::exception
{
public:
	RuntimeException(RuntimeExceptionId id)
	{
		this->id = id;
	}
	RuntimeExceptionId id;
};

class ISyntaxAnalyzer
{
public:
	virtual ~ISyntaxAnalyzer() {}
	virtual SyntaxApi::IClass* GetBaseClass()const = 0;
	virtual SemanticApi::ISClass* GetCompiledBaseClass()const = 0;
	virtual Lexer::ILexer* GetLexer()const = 0;
	virtual SemanticApi::ISMethod* GetMethod(const char* use_method) = 0;
	virtual SemanticApi::ISClassField* GetStaticField(char* use_var) = 0;
	virtual void Compile(const char* use_source)=0;
	virtual void Compile(const char* use_source,
		std::vector<SemanticApi::TExternalClassDecl> external_classes,
		std::vector<SemanticApi::TExternalSMethod> _external_bindings)=0;

	virtual std::vector<SemanticApi::ISClassField*> GetStaticFields()const=0;
	virtual std::vector<SemanticApi::ISLocalVar*> GetStaticVariables()const=0;

	static BALUSCRIPT_DLL_INTERFACE ISyntaxAnalyzer* Create();
	static BALUSCRIPT_DLL_INTERFACE void Destroy(ISyntaxAnalyzer*);
};

class TSyntaxAnalyzer : public ISyntaxAnalyzer
{
	class TPrivate;
	std::unique_ptr<TPrivate> _this;
public:
	SyntaxApi::IClass* GetBaseClass()const;
	SemanticApi::ISClass* GetCompiledBaseClass()const;
	Lexer::ILexer* GetLexer()const;
	TSyntaxAnalyzer();
	~TSyntaxAnalyzer();
	void Compile(const char* use_source);
	void Compile(const char* use_source, 
		std::vector<SemanticApi::TExternalClassDecl> external_classes,
		std::vector<SemanticApi::TExternalSMethod> _external_bindings);
	SemanticApi::ISMethod* GetMethod(const char* use_method);
	SemanticApi::ISClassField* GetStaticField(char* use_var);
	std::vector<SemanticApi::ISClassField*> GetStaticFields()const;
	std::vector<SemanticApi::ISLocalVar*> GetStaticVariables()const;
	static BALUSCRIPT_DLL_INTERFACE int GetBindingOffset();
	static BALUSCRIPT_DLL_INTERFACE int GetBindingCount();
};
