#pragma once

#include <list>

#include "SStatement.h"
#include "SType.h"

class TFormalParam;
class TExpression;
class TVariable;

namespace VariableType
{
	enum Enum
	{
		ClassField,
		Static,
		Parameter,
		Local,
		This
	};
}

class TSExpression :public TSStatement
{
	class TOperation;
	TOperation* first_op;
public:
	class TOperation : public TTokenPos
	{
	public:
		virtual TFormalParam Build() = 0;
		virtual TFormalParam GetFormalParameter() = 0;
		virtual ~TOperation(){}
	};
public:
	TSExpression(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TExpression* syntax_node)
		:TSStatement(TStatementType::Expression, use_owner, use_method, use_parent, (TStatement*)syntax_node){}
	void Build();
	TVariable* GetVar(TNameId name);
	TExpression* GetSyntax()
	{
		return (TExpression*)TSStatement::GetSyntax();
	}
};