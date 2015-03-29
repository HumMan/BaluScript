﻿#include "Expression.h"

#include "Statements.h"
#include "ClassField.h"

TOperation* TExpression::ParamsCall(TLexer& source, TOperation* curr_operation)
{
	bool use_brackets = source.Test(TTokenType::LBracket) || source.Test(TTokenType::Operator, TOperator::GetArrayElement);
	TExpression::TCallParamsOp* params_call = new TCallParamsOp(curr_operation, use_brackets);
	params_call->operation_source.InitPos(source);

	if (use_brackets)
	{
		if (!source.TestAndGet(TTokenType::Operator, TOperator::GetArrayElement))
			source.GetToken(TTokenType::LBracket);
		else
			return params_call;
	}
	else
	{
		if (!source.TestAndGet(TTokenType::Operator, TOperator::ParamsCall))
			source.GetToken(TTokenType::LParenth);
		else
			return params_call;
	}
	while (!source.Test(use_brackets ? (TTokenType::RBracket)
		: (TTokenType::RParenth)))
	{
		params_call->AddParam(new TExpression(Expr(source, 0), owner, method, parent));
		if (!source.TestAndGet(TTokenType::Comma))
			break;
	}
	source.GetToken(use_brackets ? (TTokenType::RBracket)
		: (TTokenType::RParenth));
	return params_call;
}

TOperation* TExpression::SpecialPostfix(TLexer& source,
	TOperation* curr_operation) 
{
	if (source.TestAndGet(TTokenType::Dot))
	{
		source.TestToken(TTokenType::Identifier);
		curr_operation = new TGetMemberOp(curr_operation, source.NameId());
		curr_operation->operation_source.InitPos(source);
		source.GetToken();
		return curr_operation;
	}
	else if (source.Test(TTokenType::LBracket) ||
		source.Test(TTokenType::LParenth) || 
		source.Test(TTokenType::Operator, TOperator::ParamsCall) || 
		source.Test(TTokenType::Operator, TOperator::GetArrayElement))
	{
		return ParamsCall(source, curr_operation);
	}
	else
	{
		return curr_operation;
	}
}

TOperation* TExpression::Factor(TLexer& source)
{
	TOperation* curr_operation = NULL;
	TTokenPos token_pos;
	token_pos.InitPos(source);
	if (source.Test(TTokenType::Value))
		//Синтаксис: Value
	{
		switch (source.Token())
		{
		case TValue::Int:
			curr_operation = new TIntValue(source.Int(),
				source.GetIdFromName("int"), owner);
			break;
		case TValue::Float:
			curr_operation = new TFloatValue(source.Float(),
				source.GetIdFromName("float"), owner);
			break;
		case TValue::Bool:
			curr_operation = new TBoolValue(source.Bool(),
				source.GetIdFromName("bool"), owner);
			break;
		case TValue::String:
			curr_operation = new TStringValue(source.StringValue(),
				source.GetIdFromName("string"), owner);
			break;
		case TValue::Char:
			curr_operation = new TCharValue(source.Char(),
				source.GetIdFromName("char"), owner);
			break;
		default:
			assert(false);
		}
		source.GetToken();
	}
	else
	{
		if (source.TestAndGet(TTokenType::ResWord, TResWord::This)) 
		{
			curr_operation = new TThis();
			curr_operation->operation_source = token_pos;
		}
		else if (source.TestAndGet(TTokenType::ResWord, TResWord::New)) 
		{
			TConstructTempObject* temp = new TConstructTempObject();
			temp->type.reset(new TType(owner));
			temp->type->AnalyzeSyntax(source);
			return ParamsCall(source, temp);
		}
		else if (source.Test(TTokenType::Identifier))
			//Синтаксис: Identifier
		{
			curr_operation = new TId(source.NameId());
			curr_operation->operation_source = token_pos;
			source.GetToken();
		}
		else if (source.TestAndGet(TTokenType::LParenth))
			//Синтаксис: Expr
		{
			curr_operation = Expr(source, 0);
			source.GetToken(TTokenType::RParenth);
		}
		if (curr_operation == NULL)
			source.Error("Ошибка в выражении!");
		//Синтаксис: SpecialPostfixOp*
		do
		{
			curr_operation = SpecialPostfix(source, curr_operation);
		} while (source.Test(TTokenType::LParenth) || source.Test(
			TTokenType::Operator, TOperator::ParamsCall) || source.Test(
			TTokenType::LBracket) || source.Test(TTokenType::Dot));
	}
	curr_operation->operation_source = token_pos;
	return curr_operation;
}

const int operators_priority[TOperator::End] = 
{
	/*OP_PLUS*/6,
	/*TOperator::Minus*/6,
	/*OP_MUL*/7,
	/*OP_DIV*/7,
	/*OP_PERCENT*/7,

	/*OP_AND*/2,
	/*OP_OR*/1,
	/*OP_NOT*/8,

	/*OP_ASSIGN*/0,
	/*OP_PLUS_A*/0,
	/*OP_MINUS_A*/0,
	/*OP_MUL_A*/0,
	/*OP_DIV_A*/0,
	/*OP_PERCENT_A*/0,
	/*OP_AND_A*/0,
	/*OP_OR_A*/0,

	/*OP_LESS*/5,
	/*OP_LESS_EQ*/5,
	/*OP_EQUAL*/4,
	/*OP_NOT_EQ*/3,
	/*OP_GREATER*/5,
	/*OP_GREATER_EQ*/5,

	/*ParamsCall*/-1,
	/*GetArrayElement*/-1,
	/*GetByReference*/-1,

	/*TOperator::UnaryMinus*/8

};

TOperation* TExpression::Expr(TLexer& source, int curr_prior_level) {

	const int operators_priority_max = 8;

	//TODO
	//dyn_test2=dyn_test1; если dyn_test1 не объявлен то в ошибке неправильно указывается символ

	TOperation *left;
	TOperator::Enum curr_op, curr_unary_op;
	if (curr_prior_level > operators_priority_max) 
	{
		return Factor(source);
	}
	if (source.Test(TTokenType::Operator) &&
		(source.Token() == TOperator::UnaryMinus || source.Token() == TOperator::Not)
		&& operators_priority[source.Token()] == curr_prior_level)
	{
		TTokenPos temp;
		temp.InitPos(source);
		if (source.Token() == TOperator::Minus)
			curr_unary_op = TOperator::UnaryMinus;
		else
			curr_unary_op = TOperator::Enum(source.Token());
		source.GetToken();
		TOperation* t = new TUnaryOp(Expr(source, curr_prior_level),
			curr_unary_op);
		t->operation_source = temp;
		return t;
	}
	else
		left = Expr(source, curr_prior_level + 1);
	while (source.Test(TTokenType::Operator)
		&& operators_priority[source.Token()] == curr_prior_level)
	{
		TTokenPos temp;
		temp.InitPos(source);
		curr_op = TOperator::Enum(source.Token());
		source.GetToken();
		left = new TBinOp(left, Expr(source, curr_prior_level + 1), curr_op);
		left->operation_source = temp;
	}
	return left;
}

void TExpression::AnalyzeSyntax(TLexer& source)
{
	InitPos(source);
	first_op = std::unique_ptr<TOperation>(Expr(source, 0));
	//BuildPostfix();
}

void TExpression::BuildPostfix()
{
	while (true) 
	{
		TPostfixOp temp;
		TTokenType::Enum token = source->Type();
		switch (token) 
		{
		case TTokenType::Identifier:
			temp.type = TPostfixOp::OP_ID;
			temp.id = source->NameId();
			out.push_back(temp);
			source->GetToken();
			break;
		case TTokenType::Value:
			switch (source->Token())
			{
			case TValue::Int:
				temp.type = TPostfixOp::OP_INT;
				temp.int_val = source->Int();
				break;
			case TValue::Float:
				temp.type = TPostfixOp::OP_FLOAT;
				temp.float_val = source->Float();
				break;
			case TValue::Bool:
				temp.type = TPostfixOp::OP_BOOL;
				temp.int_val = source->Bool();
				break;
			}
			out.push_back(temp);
			source->GetToken();
			break;
		case TTokenType::Operator:
			temp.type = TPostfixOp::OP_OPERATOR;
			temp.operator_type = source->Token();
			stack.push_back(temp);
			{
				int t = stack.size() - 1;
				while (t > 0 && stack[t - 1].type == TPostfixOp::OP_OPERATOR
					&& operators_priority[stack[t - 1].operator_type]
					>= operators_priority[stack[t].operator_type])
				{
					TPostfixOp temp = stack[t - 1]; stack[t - 1] = stack[t]; stack[t] = temp;
					//stack.swap(t - 1, t);
					//TODO достаточно поменять тип оператора
				}
			}
			source->GetToken();
			break;
		default:
			while (stack.size() > 0)
			{
				out.push_back(stack.back());
				stack.pop_back();
			}
			return;
		}
	}
}

void TExpression::TBinOp::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TUnaryOp::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TConstructTempObject::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TCallParamsOp::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TCharValue::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TFloatValue::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TGetMemberOp::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TId::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TIntValue::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TStringValue::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TThis::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::TBoolValue::Accept(TExpressionTreeVisitor* visitor)
{
	visitor->Visit(this);
}
void TExpression::Accept(TStatementVisitor* visitor)
{
	visitor->Visit(this);
}