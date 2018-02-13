﻿#pragma once

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
class ISExpressionVisitor;

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
	class IExpressionResult;
	class ISType;
	class ISConstructObject;
	class ISStatement;
	class ISStatements;
	class IActualParamWithConversion;
	class ISStatementVisitor;

	namespace ISOperations
	{
		class ISOperation;
		class ISExpression_TMethodCall;
		class ISExpression_TempObjectType;
		class ISExpression;
	}
	
	
	typedef void(*TExternalSMethod)(TMethodRunContext* run_context);

	class TExternalClassDecl
	{
	public:
		TExternalClassDecl()
		{
			def_constr = nullptr;
			copy_constr = nullptr;
			destr = nullptr;
			size = -1;
			for (int i = 0; i < (int)Lexer::TOperator::End; i++)
				operators[i] = nullptr;
		}
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

	BALUSCRIPT_DLL_INTERFACE ISMethod * SAnalyzeMethodSignature(Lexer::ILexer* lexer, SyntaxApi::IMethod* method_decl_syntax, ISClass* sem_base_class);
	BALUSCRIPT_DLL_INTERFACE void SDestroyMethodSignature(ISMethod * method);
}

//#include "../VirtualMachine/Op.h"

#include "ISyntaxNode.h"
#include "IMethod.h"
#include "IBuildContext.h"
#include "IOperations.h"
#include "IExpressionRunner.h"
#include "IStatementRunner.h"