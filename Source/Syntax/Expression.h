#pragma once
#include "Statement.h"
#include "Void.h"
#include "Type.h"

#include <list>

class TVariable;

class TExpression:public TStatement
{
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
	//
	class TOperation: public TTokenPos
	{
	public:
		virtual ~TOperation(){}
	};
	class TBinOp:public TOperation
	{
		TOperation *left,*right;
		TOperator::Enum op;
	public:
		TBinOp(TOperation *use_left,TOperation *use_right,TOperator::Enum use_op):left(use_left),right(use_right),op(use_op){}
		~TBinOp()
		{
			delete left;
			delete right;
		}
	};
	class TUnaryOp:public TOperation
	{
		TOperation *left;
		TOperator::Enum op;
	public:
		TUnaryOp(TOperation *use_left,TOperator::Enum use_op):left(use_left),op(use_op){}
		~TUnaryOp()
		{
			delete left;
		}
	};
	class TIntValue:public TOperation
	{
		int val;
		TType type;
	public:
		TIntValue(int use_val, TNameId int_class_name, TClass* use_owner) :val(use_val), type(int_class_name, use_owner){}
	};
	class TFloatValue:public TOperation
	{
		float val;
		TType type;
	public:
		TFloatValue(float use_val, TNameId float_class_name, TClass* use_owner) :val(use_val), type(float_class_name, use_owner){}
	};
	class TBoolValue:public TOperation
	{
		bool val;
		TType type;
	public:
		TBoolValue(bool use_val, TNameId bool_class_name, TClass* use_owner) :val(use_val), type(bool_class_name, use_owner){}
	};
	class TStringValue:public TOperation
	{
		TNameId val;
		TType type;
	public:
		TStringValue(TNameId use_val,TNameId string_class_name,TClass* use_owner):val(use_val),type(string_class_name,use_owner){}
	};
	class TCharValue:public TOperation
	{
		char val;
		TType type;
	public:
		TCharValue(char use_val, TNameId char_class_name, TClass* use_owner) :val(use_val), type(char_class_name, use_owner){}
	};
	class TGetMemberOp:public TOperation
	{
		TOperation *left;
		TNameId name;
	public:
		TGetMemberOp(TOperation *use_left,TNameId use_member):left(use_left),name(use_member){}
		~TGetMemberOp()
		{
			delete left;
		}
	};
	class TCallParamsOp:public TOperation
	{
		TOperation *left;
		std::vector<TOperation*> param;
		bool is_bracket;
	public:
		TCallParamsOp(){}
		TCallParamsOp(TOperation *use_left,bool use_is_bracket):left(use_left),is_bracket(use_is_bracket)
		{
		}
		void AddParam(TOperation* use_param)
		{
			param.push_back(use_param);
		}
		~TCallParamsOp()
		{
			delete left;
		}
	};
	class TId:public TOperation
	{
	public:
		TNameId name;
		TId(TNameId use_name) :name(use_name){}
	};

	class TThis:public TOperation
	{
	public:
	};

	TOperation* first_op;

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
		:TStatement(TStatementType::Expression, use_owner, use_method, use_parent, use_stmt_id), first_op(NULL){}

	~TExpression()
	{
		if(first_op!=NULL)
			delete first_op;
	}
	TVariable* GetVar(TNameId name);
};