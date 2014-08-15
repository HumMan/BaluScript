#pragma once
#include "Statement.h"
#include "Type.h"

#include <list>

class TVariable;

class TExpressionTreeVisitor;

class TOperation
{
public:
	TTokenPos operation_source;
	virtual ~TOperation(){}
	virtual void Accept(TExpressionTreeVisitor* visitor) = 0;
};

class TExpression:public TStatement, public TOperation
{
	friend class TSExpression;
	//TODO доделать обратную польскую нотацию
	struct TPostfixOp
	{
		enum Type
		{
			OP_ID,
			OP_OPERATOR,
			OP_GET_MEMBER,
			OP_CALL_PARAMS,
			OP_GET_ARRAY_ELEM,
			OP_INT,
			OP_FLOAT,
			OP_BOOL,
			OP_LPARENTH,
			OP_RPARENTH,
			OP_LBRACKET,
			OP_RBRACKET
		};
		Type type;
		int operator_type;
		TNameId id;
		int int_val;
		float float_val;
	};
	std::vector<TPostfixOp> stack;
	std::vector<TPostfixOp> out;
	std::vector<int> methods;
	void BuildPostfix();
public:
	
	class TBinOp:public TOperation
	{
		friend class TSemanticTreeBuilder;
		std::unique_ptr<TOperation> left, right;
		TOperator::Enum op;
	public:
		TBinOp(TOperation *use_left,TOperation *use_right,TOperator::Enum use_op):left(use_left),right(use_right),op(use_op){}
		void Accept(TExpressionTreeVisitor* visitor);
	};
	class TUnaryOp:public TOperation
	{
		friend class TSemanticTreeBuilder;
		std::unique_ptr<TOperation> left;
		TOperator::Enum op;
	public:
		TUnaryOp(TOperation *use_left,TOperator::Enum use_op):left(use_left),op(use_op){}
		void Accept(TExpressionTreeVisitor* visitor);
	};
	class TIntValue:public TOperation
	{
		friend class TSemanticTreeBuilder;
		int val;
		TType type;
	public:
		TIntValue(int use_val, TNameId int_class_name, TClass* use_owner) :val(use_val), type(int_class_name, use_owner){}
		void Accept(TExpressionTreeVisitor* visitor);
	};
	class TFloatValue:public TOperation
	{
		friend class TSemanticTreeBuilder;
		float val;
		TType type;
	public:
		TFloatValue(float use_val, TNameId float_class_name, TClass* use_owner) :val(use_val), type(float_class_name, use_owner){}
		void Accept(TExpressionTreeVisitor* visitor);
	};
	class TBoolValue:public TOperation
	{
		friend class TSemanticTreeBuilder;
		bool val;
		TType type;
	public:
		TBoolValue(bool use_val, TNameId bool_class_name, TClass* use_owner) :val(use_val), type(bool_class_name, use_owner){}
		void Accept(TExpressionTreeVisitor* visitor);
	};
	class TStringValue:public TOperation
	{
		friend class TSemanticTreeBuilder;
		TNameId val;
		TType type;
	public:
		TStringValue(TNameId use_val,TNameId string_class_name,TClass* use_owner):val(use_val),type(string_class_name,use_owner){}
		void Accept(TExpressionTreeVisitor* visitor);
	};
	class TCharValue:public TOperation
	{
		friend class TSemanticTreeBuilder;
		char val;
		TType type;
	public:
		TCharValue(char use_val, TNameId char_class_name, TClass* use_owner) :val(use_val), type(char_class_name, use_owner){}
		void Accept(TExpressionTreeVisitor* visitor);
	};
	class TGetMemberOp:public TOperation
	{
		friend class TSemanticTreeBuilder;
		std::unique_ptr<TOperation> left;
		TNameId name;
	public:
		TGetMemberOp(TOperation *use_left,TNameId use_member):left(use_left),name(use_member){}
		void Accept(TExpressionTreeVisitor* visitor);
	};
	class TConstructTempObject :public TOperation
	{
		friend class TSemanticTreeBuilder;
		
	public:
		std::shared_ptr<TType> type;
		TConstructTempObject(){}
		void Accept(TExpressionTreeVisitor* visitor);
	};
	class TCallParamsOp:public TOperation
	{
		friend class TSemanticTreeBuilder;
		std::unique_ptr<TOperation> left;
		std::vector<std::unique_ptr<TExpression>> param;
		bool is_bracket;
	public:
		TCallParamsOp(){}
		void Accept(TExpressionTreeVisitor* visitor);
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
		TNameId name;
		TId(TNameId use_name) :name(use_name){}
		void Accept(TExpressionTreeVisitor* visitor);
	};

	class TThis:public TOperation
	{
	public:
		void Accept(TExpressionTreeVisitor* visitor);
	};

	std::unique_ptr<TOperation> first_op;

	TOperation* ParamsCall(TLexer& source,TOperation* curr_operation);
	TOperation* Factor(TLexer& source);
	TOperation* SpecialPostfix(TLexer& source,TOperation* curr_operation);
	TOperation* Expr(TLexer& source,int curr_prior_level);
	void operator=(const TExpression& use_source);
public:
	void AnalyzeSyntax(TLexer& source);
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
	void Accept(TExpressionTreeVisitor* visitor)
	{
		first_op->Accept(visitor);
	}
	void Accept(TStatementVisitor* visitor);
};

class TExpressionTreeVisitor
{
public:
	virtual void Visit(TExpression::TBinOp* op) = 0;
	virtual void Visit(TExpression::TUnaryOp* op) = 0;
	virtual void Visit(TExpression::TCallParamsOp* op) = 0;
	virtual void Visit(TExpression::TConstructTempObject* op) = 0;
	virtual void Visit(TExpression::TCharValue* op) = 0;
	virtual void Visit(TExpression::TFloatValue* op) = 0;
	virtual void Visit(TExpression::TGetMemberOp* op) = 0;
	virtual void Visit(TExpression::TId* op) = 0;
	virtual void Visit(TExpression::TIntValue *op) = 0;
	virtual void Visit(TExpression::TStringValue *op) = 0;
	virtual void Visit(TExpression::TThis *op) = 0;
	virtual void Visit(TExpression::TBoolValue *op) = 0;
};