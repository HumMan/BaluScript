#include <assert.h>

#include "LocalVar.h"
#include "Expression.h"
#include "Statements.h"

void TLocalVar::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	type.AnalyzeSyntax(source);
	is_static = source.TestAndGet(TTokenType::ResWord, TResWord::Static);
	assert(parent->GetType()==TStatementType::Statements);
	TStatements* statements = (TStatements*) parent;
	TLocalVar* curr_var = this;
	do {
		if (curr_var != this) {
			curr_var->type = type;
			curr_var->is_static = is_static;
			*(TTokenPos*) curr_var = *(TTokenPos*) this;
		}
		curr_var->name = source.NameId();
		int identifier_token = source.GetCurrentToken();
		source.SetCurrentToken(identifier_token + 1);
		bool is_next_assign = source.Test(TTokenType::Operator,
				TOperator::Assign);
		source.SetCurrentToken(identifier_token);
		if (is_next_assign) {
			curr_var->assign_expr = new TExpression(owner, method, parent,
					curr_var->stmt_id);
			curr_var->assign_expr->AnalyzeSyntax(source);
		} else {
			source.GetToken();
			if (source.TestAndGet(TTokenType::LParenth)) {
				while (!source.Test(TTokenType::LParenth)) {
					TExpression* expr = new TExpression(owner, method, parent,
							curr_var->stmt_id);
					expr->AnalyzeSyntax(source);
					curr_var->params.Push(expr);
					if (!source.TestAndGet(TTokenType::Comma))
						break;
				}
				source.GetToken(TTokenType::RParenth);
			}
		}
		if (source.Test(TTokenType::Comma)) {
			curr_var = new TLocalVar(owner, method, statements, -1);
			statements->AddVar(curr_var);
		}
	} while (source.TestAndGet(TTokenType::Comma));
}

TLocalVar::TLocalVar(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id) :
TStatement(TStatementType::VarDecl, use_owner, use_method, use_parent, use_stmt_id),
TVariable(false, TVariableType::LocalVar),
type(use_owner), assign_expr(NULL),
offset(-1), is_static(false)
{
}

TNameId TLocalVar::GetName(){
	return name;
}

TLocalVar::~TLocalVar()
{
	delete assign_expr;
}
TClass* TLocalVar::GetClass(){
	return type.GetClass();
}
int TLocalVar::GetOffset(){
	return offset;
}
bool TLocalVar::IsStatic(){
	return is_static;
}
TFormalParam TLocalVar::PushRefToStack(TNotOptimizedProgram &program)
{
	TOpArray ops_array;
	if (is_static)
		program.Push(TOp(TOpcode::PUSH_GLOBAL_REF, offset), ops_array);
	else
		program.Push(TOp(TOpcode::PUSH_LOCAL_REF, offset), ops_array);
	return TFormalParam(type.GetClass(), true, ops_array);
}
TStatement* TLocalVar::GetCopy()
{
	TLocalVar* copy = new TLocalVar(*this);
	if (assign_expr != NULL)
		copy->assign_expr = new TExpression(*assign_expr);
	return copy;
}
void TLocalVar::InitOwner(TClass* use_owner, TMethod* use_method, TStatements* use_parent)
{
	TStatement::_InitOwner(use_owner, use_method, use_parent);
	type.InitOwner(use_owner);
	if (assign_expr != NULL)assign_expr->InitOwner(use_owner, use_method, use_parent);
	for (int i = 0; i <= params.GetHigh(); i++)
		params[i]->InitOwner(use_owner, use_method, use_parent);
}