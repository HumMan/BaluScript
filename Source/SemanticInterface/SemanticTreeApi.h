#pragma once

#include <list>
#include <vector>
#include <memory>
#include <map>

#include "../lexer.h"
#include "../SyntaxInterface/SyntaxTreeApi.h"

#ifdef _MSC_VER
#pragma warning( disable : 4250)  
#endif

class TMethodRunContext;

namespace SemanticApi
{
	template<class T>
	class ISyntaxNode;

	class ISMethod;
	class ISClass;
	class ISpecialClassMethod;
	class ISParameter;
	class IVariable;
	class ISClassField;
	class ISLocalVar;
	
	typedef void(*TExternalSMethod)(TMethodRunContext* run_context);

	class TExternalClassDecl
	{
	public:
		std::string source;
		TExternalSMethod def_constr,
			copy_constr,
			destr;
		TExternalSMethod operators[(int)Lexer::TOperator::End];
		std::map<std::string, TExternalSMethod> methods;
		size_t size;
	};

	class TGlobalBuildContext
	{
	public:
		std::vector<ISClassField*>* static_fields;
		std::vector<ISLocalVar*>* static_variables;
		TGlobalBuildContext()
		{
			static_fields = nullptr;
			static_variables = nullptr;
		}
		TGlobalBuildContext(std::vector<ISClassField*>* static_fields, std::vector<ISLocalVar*>* static_variables)
		{
			this->static_fields = static_fields;
			this->static_variables = static_variables;
		}
	};

	BALUSCRIPT_DLL_INTERFACE ISClass* SAnalyze(Lexer::ILexer* lexer, SyntaxApi::IClass* base_class, 
		std::vector<TExternalClassDecl> external_classes, TGlobalBuildContext build_context);

	BALUSCRIPT_DLL_INTERFACE void SDestroy(ISClass*);
}

//#include "../VirtualMachine/Op.h"

#include "ISyntaxNode.h"
#include "IMethod.h"
#include "IBuildContext.h"