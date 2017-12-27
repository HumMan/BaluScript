#pragma once

#include "lexer.h"
#include <vector>
#include <memory>

#include "SyntaxInterface/SyntaxTreeApi.h"

class TSClassField;
class TSClass;
class TSMethod;
class TTemplateRealizations;
class TSLocalVar;

class ISyntaxAnalyzer
{
	virtual SyntaxApi::IClass* GetBaseClass()const = 0;
	virtual TSClass* GetCompiledBaseClass()const = 0;
	virtual Lexer::ILexer* GetLexer()const = 0;
	virtual TSMethod* GetMethod(char* use_method) = 0;
	virtual TSClassField* GetStaticField(char* use_var) = 0;
};

class BALUSCRIPT_DLL_INTERFACE TSyntaxAnalyzer : public ISyntaxAnalyzer
{
	class TPrivate;
	std::unique_ptr<TPrivate> _this;
public:
	SyntaxApi::IClass* GetBaseClass()const;
	TSClass* GetCompiledBaseClass()const;
	Lexer::ILexer* GetLexer()const;
	TSyntaxAnalyzer();
	~TSyntaxAnalyzer();
	void Compile(char* use_source/*,TTime& time*/);
	void CreateInternalClasses();
	TSMethod* GetMethod(char* use_method);
	TSClassField* GetStaticField(char* use_var);

};
