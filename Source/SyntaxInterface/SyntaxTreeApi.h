#pragma once

#include <list>
#include <vector>
#include <memory>

#include "../lexer.h"

#ifdef _MSC_VER
#pragma warning( disable : 4250)  
#endif

namespace SyntaxApi
{
	class IExpressionTreeVisitor;
	class IBytecode;
	class IExpression;
	class IFor;
	class IIf;
	class IWhile;
	class ILocalVar;
	class IReturn;
	class IClass;
	class IClassField;
	class IType_TClassName;
	class IType_TTemplateParameter;
	class IType_TClassName;
	class IStatement;
	class IStatements;
	class IMethod;
	class IOverloadedMethod;
	class IWhile;
	class IStatementVisitor;
	class IParameter;

	BALUSCRIPT_DLL_INTERFACE IClass* Analyze(Lexer::ILexer* source);
	BALUSCRIPT_DLL_INTERFACE IClass* AnalyzeNestedClass(Lexer::ILexer * lexer, IClass* parent);
	BALUSCRIPT_DLL_INTERFACE IMethod* AnalyzeMethodSignature(Lexer::ILexer * lexer, IClass* parent);

	BALUSCRIPT_DLL_INTERFACE void Destroy(IClass*);
	BALUSCRIPT_DLL_INTERFACE void Destroy(IMethod*);
}

#include "../VirtualMachine/Op.h"

#include "IType.h"
#include "IAccesible.h"
#include "IClass.h"
#include "IClassField.h"
#include "IMethod.h"
#include "IOverloadedMethod.h"
#include "IParameter.h"
#include "IStatement.h"
#include "IStatements.h"
#include "IExpression.h"
#include "IReturn.h"
#include "IWhile.h"
#include "IFor.h"
#include "IIf.h"
#include "ILocalVar.h"
#include "IBytecode.h"