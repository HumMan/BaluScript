#pragma once
#include "Statement.h"
#include "Type.h"

#include <list>

class TVariable;
class TExpressionTreeVisitor;
class TSOperation;

class TOperation
{
public:
	Lexer::TTokenPos operation_source;
	virtual ~TOperation(){}
	virtual TSOperation* Accept(TExpressionTreeVisitor* visitor) = 0;
};

class TExpression:public TStatement, public TOperation
{
	friend class TSExpression;
public:
	
	class TBinOp:public TOperation
	{
		friend class TSemanticTreeBuilder;
		std::unique_ptr<TOperation> left, right;
		Lexer::TOperator op;
	public:
		TBinOp(TOperation *use_left, TOperation *use_right, Lexer::TOperator use_op) :left(use_left), right(use_right), op(use_op){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};
	class TUnaryOp:public TOperation
	{
		friend class TSemanticTreeBuilder;
		std::unique_ptr<TOperation> left;
		Lexer::TOperator op;
	public:
		TUnaryOp(TOperation *use_left, Lexer::TOperator use_op) :left(use_left), op(use_op){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};
	class TIntValue:public TOperation
	{
		friend class TSemanticTreeBuilder;
		int val;
		TType type;
	public:
		TIntValue(int use_val, Lexer::TNameId int_class_name, TClass* use_owner) :val(use_val), type(int_class_name, use_owner){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};
	class TFloatValue:public TOperation
	{
		friend class TSemanticTreeBuilder;
		float val;
		TType type;
	public:
		TFloatValue(float use_val, Lexer::TNameId float_class_name, TClass* use_owner) :val(use_val), type(float_class_name, use_owner){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};
	class TBoolValue:public TOperation
	{
		friend class TSemanticTreeBuilder;
		bool val;
		TType type;
	public:
		TBoolValue(bool use_val, Lexer::TNameId bool_class_name, TClass* use_owner) :val(use_val), type(bool_class_name, use_owner){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};
	class TStringValue:public TOperation
	{
		friend class TSemanticTreeBuilder;
		std::string val;
		TType type;
	public:
		TStringValue(std::string use_val, Lexer::TNameId string_class_name, TClass* use_owner) :val(use_val), type(string_class_name, use_owner){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};
	class TCharValue:public TOperation
	{
		friend class TSemanticTreeBuilder;
		char val;
		TType type;
	public:
		TCharValue(char use_val, Lexer::TNameId char_class_name, TClass* use_owner) :val(use_val), type(char_class_name, use_owner){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};
	class TGetMemberOp:public TOperation
	{
		friend class TSemanticTreeBuilder;
		std::unique_ptr<TOperation> left;
		Lexer::TNameId name;
	public:
		TGetMemberOp(TOperation *use_left, Lexer::TNameId use_member) :left(use_left), name(use_member){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};
	class TConstructTempObject :public TOperation
	{
		friend class TSemanticTreeBuilder;
		
	public:
		std::shared_ptr<TType> type;
		TConstructTempObject(){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};
	class TCallParamsOp:public TOperation
	{
		friend class TSemanticTreeBuilder;
		std::unique_ptr<TOperation> left;
		std::vector<std::unique_ptr<TExpression>> param;
		bool is_bracket;
	public:
		TCallParamsOp(){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
		TCallParamsOp(TOperation *use_left,bool use_is_bracket):left(use_left),is_bracket(use_is_bracket)
		{
		}
		void AddParam(TExpression* use_param)
		{
			param.push_back(std::unique_ptr<TExpression>(use_param));
		}
	};
	class TId:public TOperation
	{
	public:
		Lexer::TNameId name;
		TId(Lexer::TNameId use_name) :name(use_name){}
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};

	class TThis:public TOperation
	{
	public:
		TSOperation* Accept(TExpressionTreeVisitor* visitor);
	};

	std::unique_ptr<TOperation> first_op;

	TOperation* ParamsCall(Lexer::ILexer* source,TOperation* curr_operation);
	TOperation* Factor(Lexer::ILexer* source);
	TOperation* SpecialPostfix(Lexer::ILexer* source,TOperation* curr_operation);
	TOperation* Expr(Lexer::ILexer* source,int curr_prior_level);
	void operator=(const TExpression& use_source);
public:
	void AnalyzeSyntax(Lexer::ILexer* source);
	TExpression(const TExpression &use_source) :TStatement(use_source)
	{
	}

	TExpression(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
		:TStatement(TStatementType::Expression, use_owner, use_method, use_parent, use_stmt_id)
	{
	}

	TExpression(TOperation* use_op, TClass* use_owner, TMethod* use_method, TStatements* use_parent)
		:TStatement(TStatementType::Expression, use_owner, use_method, use_parent, -1), first_op(use_op)
	{
	}

	~TExpression()
	{
	}
	TSOperation* Accept(TExpressionTreeVisitor* visitor)
	{
		return first_op->Accept(visitor);
	}
	void Accept(TStatementVisitor* visitor);
};

class TExpressionTreeVisitor
{
public:
	virtual TSOperation* Visit(TExpression::TBinOp* op) = 0;
	virtual TSOperation* Visit(TExpression::TUnaryOp* op) = 0;
	virtual TSOperation* Visit(TExpression::TCallParamsOp* op) = 0;
	virtual TSOperation* Visit(TExpression::TConstructTempObject* op) = 0;
	virtual TSOperation* Visit(TExpression::TCharValue* op) = 0;
	virtual TSOperation* Visit(TExpression::TFloatValue* op) = 0;
	virtual TSOperation* Visit(TExpression::TGetMemberOp* op) = 0;
	virtual TSOperation* Visit(TExpression::TId* op) = 0;
	virtual TSOperation* Visit(TExpression::TIntValue *op) = 0;
	virtual TSOperation* Visit(TExpression::TStringValue *op) = 0;
	virtual TSOperation* Visit(TExpression::TThis *op) = 0;
	virtual TSOperation* Visit(TExpression::TBoolValue *op) = 0;
};