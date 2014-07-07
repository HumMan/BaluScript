#pragma once

#include "../lexer.h"

#include "../Syntax/Statement.h"

#include "SSyntaxNode.h"

class TSMethod;
class TSStatements;
class TSClass;

class TSStatement :public TSyntaxNode<TStatement>
{
protected:
	TSMethod* method;
	TSStatements* parent;
	TSClass* owner;
public:
	TSStatement(TStatementType::Enum use_stmt_type, TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TStatement* use_syntax) :
		method(use_method), parent(use_parent), owner(use_owner),
		TSyntaxNode(use_syntax){}
	
	virtual ~TSStatement(){}
};

