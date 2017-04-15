#pragma once

namespace SyntaxApi
{
	namespace IOperations
	{
		class IOperation
		{
		public:
			virtual void Accept(IExpressionTreeVisitor* visitor) = 0;
			virtual Lexer::TTokenPos* GetOperationSource() = 0;
		};

		class IBinOp : public virtual IOperation
		{
		public:
			virtual IOperation* GetLeft()const = 0;
			virtual IOperation* GetRight()const = 0;
			virtual Lexer::TOperator GetOp()const = 0;
		};
		class IUnaryOp : public virtual IOperation
		{
		public:
			virtual IOperation* GetLeft()const=0;
			virtual Lexer::TOperator GetOp()const = 0;
		};
		class ICallParamsOp : public virtual IOperation
		{
		public:
			virtual IOperation* GetLeft()const = 0;
			virtual bool IsBracket()const = 0;
			virtual std::vector<IExpression*> GetParam()const = 0;
		};
		class IConstructTempObject : public virtual IOperation
		{
		public:
			virtual IType* GetType()const = 0;
		};
		class ICharValue : public virtual IOperation
		{
		public:
			virtual char GetValue()const = 0;
			virtual IType* GetType() = 0;
		};
		class IFloatValue : public virtual IOperation
		{
		public:
			virtual float GetValue()const = 0;
			virtual IType* GetType() = 0;
		};
		class IGetMemberOp : public virtual IOperation
		{
		public:
			virtual IOperation* GetLeft()const = 0;
			virtual Lexer::TNameId GetName()const = 0;
		};
		class IId : public virtual IOperation
		{
		public:
			virtual Lexer::TNameId GetName()const = 0;
		};
		class IIntValue : public virtual IOperation
		{
		public:
			virtual int GetValue()const = 0;
			virtual IType* GetType() = 0;
		};
		class IStringValue : public virtual IOperation
		{
		public:
			virtual std::string GetValue()const = 0;
			virtual IType* GetType() = 0;
		};
		class IThis : public virtual IOperation
		{
		public:
		};
		class IBoolValue : public virtual IOperation
		{
		public:
			virtual bool GetValue()const = 0;
			virtual IType* GetType() = 0;
		};
	}

	class IExpressionTreeVisitor
	{
	public:
		virtual void Visit(IOperations::IBinOp* op) = 0;
		virtual void Visit(IOperations::IUnaryOp* op) = 0;
		virtual void Visit(IOperations::ICallParamsOp* op) = 0;
		virtual void Visit(IOperations::IConstructTempObject* op) = 0;
		virtual void Visit(IOperations::ICharValue* op) = 0;
		virtual void Visit(IOperations::IFloatValue* op) = 0;
		virtual void Visit(IOperations::IGetMemberOp* op) = 0;
		virtual void Visit(IOperations::IId* op) = 0;
		virtual void Visit(IOperations::IIntValue *op) = 0;
		virtual void Visit(IOperations::IStringValue *op) = 0;
		virtual void Visit(IOperations::IThis *op) = 0;
		virtual void Visit(IOperations::IBoolValue *op) = 0;
	};

	class IExpression : public virtual Lexer::ITokenPos, public virtual IStatement, public virtual IOperations::IOperation
	{
	public:
	};
}