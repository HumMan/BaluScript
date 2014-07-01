#include "Statement.h"

#include "Statements.h"
#include "ClassField.h"

void TStatement::SetStmtId(int use_id)
{
	stmt_id = use_id;
}
TStatement::TStatement(TStatementType::Enum use_stmt_type, TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id) :
method(use_method), parent(use_parent), owner(use_owner),
stmt_id(use_stmt_id), stmt_type(use_stmt_type)
{
}

TStatementType::Enum TStatement::GetType(){
	return stmt_type;
}