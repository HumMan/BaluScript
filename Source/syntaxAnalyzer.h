#pragma once

#include "lexer.h"
#include <vector>
#include <memory>

#include "SyntaxInterface/SyntaxTreeApi.h"

#include "SemanticInterface/SemanticTreeApi.h"

class ISyntaxAnalyzer
{
	virtual SyntaxApi::IClass* GetBaseClass()const = 0;
	virtual SemanticApi::ISClass* GetCompiledBaseClass()const = 0;
	virtual Lexer::ILexer* GetLexer()const = 0;
	virtual SemanticApi::ISMethod* GetMethod(char* use_method) = 0;
	virtual SemanticApi::ISClassField* GetStaticField(char* use_var) = 0;
};

class BALUSCRIPT_DLL_INTERFACE TSyntaxAnalyzer : public ISyntaxAnalyzer
{
	class TPrivate;
	std::unique_ptr<TPrivate> _this;
public:
	SyntaxApi::IClass* GetBaseClass()const;
	SemanticApi::ISClass* GetCompiledBaseClass()const;
	Lexer::ILexer* GetLexer()const;
	TSyntaxAnalyzer();
	~TSyntaxAnalyzer();
	void Compile(char* use_source/*,TTime& time*/);
	void CreateInternalClasses();
	SemanticApi::ISMethod* GetMethod(char* use_method);
	SemanticApi::ISClassField* GetStaticField(char* use_var);

};
