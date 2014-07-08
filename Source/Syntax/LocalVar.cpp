#include <assert.h>

#include "LocalVar.h"
#include "Expression.h"
#include "Statements.h"
#include "ClassField.h"

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
		if (is_next_assign) 
		{
			source.GetToken(TTokenType::Identifier);
			source.TestAndGet(TTokenType::Operator,TOperator::Assign);
			curr_var->assign_expr = std::shared_ptr<TExpression>(new TExpression(owner, method, parent,
					curr_var->stmt_id));
			curr_var->assign_expr->AnalyzeSyntax(source);
		} else {
			source.GetToken();
			if (source.TestAndGet(TTokenType::LParenth)) {
				while (!source.Test(TTokenType::LParenth)) {
					TExpression* expr = new TExpression(owner, method, parent,
							curr_var->stmt_id);
					expr->AnalyzeSyntax(source);
					curr_var->params.push_back(std::shared_ptr<TExpression>(expr));
					if (!source.TestAndGet(TTokenType::Comma))
						break;
				}
				source.GetToken(TTokenType::RParenth);
			}
		}
		if ((source.Test(TTokenType::Comma) || source.Test(TTokenType::Semicolon)) && (curr_var != this))
		{
			statements->AddVar(curr_var);
		}
		if (source.Test(TTokenType::Comma))
		{
			curr_var = new TLocalVar(owner, method, statements, -1);
		}
	} while (source.TestAndGet(TTokenType::Comma));
}

TLocalVar::TLocalVar(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id) :
TStatement(TStatementType::VarDecl, use_owner, use_method, use_parent, use_stmt_id),
 type(use_owner), assign_expr(NULL), is_static(false)
{
}

TNameId TLocalVar::GetName(){
	return name;
}
bool TLocalVar::IsStatic(){
	return is_static;
}

void TLocalVar::Accept(TStatementVisitor* visitor)
{
	visitor->Visit(this);
}