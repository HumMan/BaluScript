#pragma once

#include <list>
#include <vector>
#include <memory>

#include "../lexer.h"

#ifdef _MSC_VER
#pragma warning( disable : 4250)  
#endif

namespace SemanticApi
{
	template<class T>
	class ISyntaxNode;

	//BALUSCRIPT_DLL_INTERFACE IClass* Analyze(Lexer::ILexer* source);
	//BALUSCRIPT_DLL_INTERFACE IClass* AnalyzeNestedClass(Lexer::ILexer * lexer, IClass* parent);
	//BALUSCRIPT_DLL_INTERFACE IMethod* AnalyzeNestedMethod(Lexer::ILexer * lexer, IClass* parent);

	//BALUSCRIPT_DLL_INTERFACE void Destroy(IClass*);
	//BALUSCRIPT_DLL_INTERFACE void Destroy(IMethod*);
}

//#include "../VirtualMachine/Op.h"

#include "ISyntaxNode.h"
#include "IMethod.h"