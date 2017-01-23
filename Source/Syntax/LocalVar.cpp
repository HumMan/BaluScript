#include <assert.h>

#include "LocalVar.h"
#include "Expression.h"
#include "Statements.h"
#include "ClassField.h"

using namespace Lexer;

void TLocalVar::AnalyzeSyntax(Lexer::ILexer* source) {
	InitPos(source);
	type->AnalyzeSyntax(source);
	is_static = source->TestAndGet(TResWord::Static);
	assert(GetParent()->GetType()==TStatementType::Statements);
	TStatements* statements = (TStatements*) GetParent();
	TLocalVar* curr_var = this;
	do {
		if (curr_var != this) {
			curr_var->type = type;
			curr_var->is_static = is_static;
			*(TTokenPos*) curr_var = *(TTokenPos*) this;
		}
		curr_var->name = source->NameId();
		int identifier_token = source->GetCurrentToken();
		source->SetCurrentToken(identifier_token + 1);
		bool is_next_assign = source->Test(
				TOperator::Assign);
		source->SetCurrentToken(identifier_token);
		if (is_next_assign) 
		{
			//source->GetToken(TTokenType::Identifier);
			//source->TestAndGet(TOperator::Assign);
			curr_var->assign_expr.reset(new TExpression(GetOwner(), GetMethod(), GetParent(),
					curr_var->GetStmtId()));
			curr_var->assign_expr->AnalyzeSyntax(source);
		} else {
			source->GetToken();
			if (source->TestAndGet(TTokenType::LParenth)) 
			{
				while (!source->Test(TTokenType::RParenth)) 
				{
					TExpression* expr = new TExpression(GetOwner(), GetMethod(), GetParent(),
							curr_var->GetStmtId());
					expr->AnalyzeSyntax(source);
					curr_var->params.push_back(std::unique_ptr<TExpression>(expr));
					if (!source->TestAndGet(TTokenType::Comma))
						break;
				}
				source->GetToken(TTokenType::RParenth);
			}
		}
		if ((source->Test(TTokenType::Comma) || source->Test(TTokenType::Semicolon)) && (curr_var != this))
		{
			statements->AddVar(curr_var);
			if (curr_var->assign_expr)
				curr_var->assign_expr->SetStmtId(curr_var->GetStmtId());
			if (curr_var->params.size() > 0)
			{
				for (const std::unique_ptr<TExpression>& v : curr_var->params)
					v->SetStmtId(curr_var->GetStmtId());
			}
		}
		if (source->Test(TTokenType::Comma))
		{
			curr_var = new TLocalVar(GetOwner(), GetMethod(), statements, -1);
		}
	} while (source->TestAndGet(TTokenType::Comma));
}

TLocalVar::TLocalVar(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id) :
TStatement(TStatementType::VarDecl, use_owner, use_method, use_parent, use_stmt_id),
type(new TType(use_owner)), is_static(false)
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

TExpression* TLocalVar::GetAssignExpr()const
{
	return assign_expr.get();
}
std::vector<TExpression*> TLocalVar::GetParams()const
{
	std::vector<TExpression*> result;
	result.resize(params.size());
	for (size_t i = 0; i < params.size(); i++)
		result[i] = params[i].get();
	return result;
}