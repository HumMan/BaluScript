#pragma once

#include "../SyntaxTreeApi.h"

#include "Statement.h"
#include "Type.h"

namespace SyntaxInternal
{
	class TOperation : public virtual SyntaxApi::IOperations::IOperation
	{
	public:
		Lexer::TTokenPos operation_source;
		Lexer::TTokenPos* GetOperationSource()
		{
			return &operation_source;
		}
		virtual ~TOperation(){}
	};

	class TExpression :public TStatement, public TOperation, public SyntaxApi::IExpression
	{
	public:

		class TBinOp :public TOperation, public SyntaxApi::IOperations::IBinOp
		{
			std::unique_ptr<TOperation> left, right;
			Lexer::TOperator op;
		public:
			TOperation* GetLeft()const
			{
				return left.get();
			}
			TOperation* GetRight()const
			{
				return right.get();
			}
			Lexer::TOperator GetOp()const
			{
				return op;
			}
			TBinOp(TOperation *use_left, TOperation *use_right, Lexer::TOperator use_op) :left(use_left), right(use_right), op(use_op){}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};
		class TUnaryOp :public TOperation, public SyntaxApi::IOperations::IUnaryOp
		{
			std::unique_ptr<TOperation> left;
			Lexer::TOperator op;
		public:
			TOperation* GetLeft()const
			{
				return left.get();
			}
			Lexer::TOperator GetOp()const
			{
				return op;
			}
			TUnaryOp(TOperation *use_left, Lexer::TOperator use_op) :left(use_left), op(use_op){}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};
		class TIntValue :public TOperation, public SyntaxApi::IOperations::IIntValue
		{
			int val;
			TType type;
		public:
			int GetValue()const
			{
				return val;
			}
			TType* GetType()
			{
				return &type;
			}
			TIntValue(int use_val, Lexer::TNameId int_class_name, TClass* use_owner) :val(use_val), type(int_class_name, use_owner){}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};
		class TFloatValue :public TOperation, public SyntaxApi::IOperations::IFloatValue
		{
			float val;
			TType type;
		public:
			float GetValue()const
			{
				return val;
			}
			TType* GetType()
			{
				return &type;
			}
			TFloatValue(float use_val, Lexer::TNameId float_class_name, TClass* use_owner) :val(use_val), type(float_class_name, use_owner){}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};
		class TBoolValue :public TOperation, public SyntaxApi::IOperations::IBoolValue
		{
			bool val;
			TType type;
		public:
			bool GetValue()const
			{
				return val;
			}
			TType* GetType()
			{
				return &type;
			}
			TBoolValue(bool use_val, Lexer::TNameId bool_class_name, TClass* use_owner) :val(use_val), type(bool_class_name, use_owner){}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};
		class TStringValue :public TOperation, public SyntaxApi::IOperations::IStringValue
		{
			std::string val;
			TType type;
		public:
			std::string GetValue()const
			{
				return val;
			}
			TType* GetType()
			{
				return &type;
			}
			TStringValue(std::string use_val, Lexer::TNameId string_class_name, TClass* use_owner) :val(use_val), type(string_class_name, use_owner){}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};
		class TCharValue :public TOperation, public SyntaxApi::IOperations::ICharValue
		{
			char val;
			TType type;
		public:
			char GetValue()const
			{
				return val;
			}
			TType* GetType()
			{
				return &type;
			}
			TCharValue(char use_val, Lexer::TNameId char_class_name, TClass* use_owner) :val(use_val), type(char_class_name, use_owner){}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};
		class TGetMemberOp :public TOperation, public SyntaxApi::IOperations::IGetMemberOp
		{
			std::unique_ptr<TOperation> left;
			Lexer::TNameId name;
		public:
			TOperation* GetLeft()const
			{
				return left.get();
			}
			Lexer::TNameId GetName()const
			{
				return name;
			}
			TGetMemberOp(TOperation *use_left, Lexer::TNameId use_member) :left(use_left), name(use_member){}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};
		class TConstructTempObject :public TOperation, public SyntaxApi::IOperations::IConstructTempObject
		{
			std::unique_ptr<TType> type;
		public:
			TType* GetType()const
			{
				return type.get();
			}
			TConstructTempObject(){}
			TConstructTempObject(TType* type)
			{
				this->type.reset(type);
			}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};
		class TCallParamsOp :public TOperation, public SyntaxApi::IOperations::ICallParamsOp
		{
			std::unique_ptr<TOperation> left;
			std::vector<std::unique_ptr<TExpression>> param;
			bool is_bracket;
		public:
			TOperation* GetLeft()const
			{
				return left.get();
			}
			bool IsBracket()const
			{
				return is_bracket;
			}
			std::vector<TExpression*> GetParamT()const
			{
				std::vector<TExpression*> result;
				for (const std::unique_ptr<TExpression>& v : param)
					result.push_back(v.get());
				return result;
			}
			std::vector<IExpression*> GetParam()const
			{
				std::vector<IExpression*> result;
				for (const std::unique_ptr<TExpression>& v : param)
					result.push_back(v.get());
				return result;
			}
			TCallParamsOp(){}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
			TCallParamsOp(TOperation *use_left, bool use_is_bracket) :left(use_left), is_bracket(use_is_bracket)
			{
			}
			void AddParam(TExpression* use_param)
			{
				param.push_back(std::unique_ptr<TExpression>(use_param));
			}
		};
		class TId :public TOperation, public SyntaxApi::IOperations::IId
		{
			Lexer::TNameId name;
		public:
			Lexer::TNameId GetName()const
			{
				return name;
			}
			TId(Lexer::TNameId use_name) :name(use_name){}
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};

		class TThis :public TOperation, public SyntaxApi::IOperations::IThis
		{
		public:
			void Accept(SyntaxApi::IExpressionTreeVisitor* visitor);
		};

		std::unique_ptr<TOperation> first_op;

		TOperation* ParamsCall(Lexer::ILexer* source, TOperation* curr_operation);
		TOperation* Factor(Lexer::ILexer* source);
		TOperation* SpecialPostfix(Lexer::ILexer* source, TOperation* curr_operation);
		TOperation* Expr(Lexer::ILexer* source, int curr_prior_level);
		void operator=(const TExpression& use_source);
	public:
		bool IsEmpty();
		void AnalyzeSyntax(Lexer::ILexer* source);
		TExpression(const TExpression &use_source) :TStatement(use_source)
		{
		}

		TExpression(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
			:TStatement(SyntaxApi::TStatementType::Expression, use_owner, use_method, use_parent, use_stmt_id)
		{
		}

		TExpression(TOperation* use_op, TClass* use_owner, TMethod* use_method, TStatements* use_parent)
			:TStatement(SyntaxApi::TStatementType::Expression, use_owner, use_method, use_parent, -1), first_op(use_op)
		{
		}

		~TExpression()
		{
		}
		void Accept(SyntaxApi::IExpressionTreeVisitor* visitor)
		{
			return first_op->Accept(visitor);
		}
		void Accept(SyntaxApi::IStatementVisitor* visitor);
	};

	
}