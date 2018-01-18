#pragma once

#include "../../SyntaxInterface/SyntaxTreeApi.h"

#include "SSyntaxNode.h"

class TSMethod;
class TSStatements;
class TSClass;
class TStackValue;
class TExpressionResult;
class TActualParamWithConversion;

class TSStatement :public TSyntaxNode<SyntaxApi::IStatement>, public virtual SemanticApi::ISStatement
{
private:
	TSMethod* method;
	TSStatements* parent;
	TSClass* owner;
public:
	TSStatements* GetParentStatements()const
	{
		return parent;
	}
	TSStatement(SyntaxApi::TStatementType use_stmt_type, TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IStatement* use_syntax) :
		TSyntaxNode(use_syntax),
		method(use_method), parent(use_parent), owner(use_owner){}

	virtual void Accept(SemanticApi::ISStatementVisitor* visitor) = 0;

	virtual ~TSStatement(){}
	SemanticApi::ISClass* IGetOwner()const;
	SemanticApi::ISMethod* IGetMethod()const;
	TSClass * TSStatement::GetOwner() const;
	TSMethod * TSStatement::GetMethod() const;
	void TestBoolExpr(TExpressionResult& compare_result, std::unique_ptr<TActualParamWithConversion>& conversion);
};

