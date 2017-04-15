#pragma once

#include <list>
#include <vector>
#include <memory>

#include "../lexer.h"

#pragma warning( disable : 4250)  

//TODO нужно заменить на интерфейсы api
namespace SyntaxInternal
{
	class TType;
	class TClassField;
	class TOverloadedMethod;
	class TMethod;
	class TStatement;
	class TBytecode;
	class TExpression;
	class TFor;
	class TIf;
	class TLocalVar;
	class TReturn;
	class TStatements;
	class TRetTWhileurn;
}

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