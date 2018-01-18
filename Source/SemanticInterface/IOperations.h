#pragma once

namespace SemanticApi
{
	enum TMethodCallType
	{
		Method,
		ObjectConstructor,
		//ObjectConstructorInitWithAssign,
		Operator,
		//DefaultAssignOperator
	};

	namespace ISOperations
	{
		class ISOperation : public virtual Lexer::ITokenPos
		{
		public:
			virtual void Accept(ISExpressionVisitor*) = 0;
			///<summary>Получить тип возвращаемого подвыражением значения</summary>
			virtual const IExpressionResult* GetFormalParam()const = 0;
		};

		class ISExpression_TMethodCall : public virtual ISOperation
		{
		public:
			virtual const IActualParameters* GetParams()const = 0;
			virtual TMethodCallType GetType()const = 0;
			virtual ISMethod* GetInvoke()const=0;
			virtual ISOperations::ISOperation* GetLeft()const=0;
		};

		class ISExpression_TGetClass : public virtual ISOperation
		{
		public:
		};

		class ISExpression_TCreateTempObject : public virtual ISOperation
		{
		public:
			virtual ISOperations::ISExpression_TempObjectType* GetLeft()const=0;
			virtual ISConstructObject* GetConstructObject()const=0;
		};

		class ISExpression_TempObjectType : public virtual ISOperation
		{
		public:
		};
		class IInt : public virtual ISOperation
		{
		public:
			virtual const ISType* GetType()const = 0;
			virtual int GetValue()const = 0;
		};
		class IFloat : public virtual ISOperation
		{
		public:
			virtual const ISType* GetType()const = 0;
			virtual float GetValue()const = 0;
		};
		class IBool : public virtual ISOperation
		{
		public:
			virtual const ISType* GetType()const = 0;
			virtual bool GetValue()const = 0;
		};
		class IString : public virtual ISOperation
		{
		public:
			virtual const ISType* GetType()const = 0;
			virtual std::string GetValue()const = 0;
		};
		class IEnumValue : public virtual ISOperation
		{
		public:
			virtual ISClass* GetType()const = 0;
			virtual int GetValue()const = 0;
		};
		class IGetMethods : public virtual ISOperation
		{
		public:
			virtual ISOperations::ISOperation* GetLeft()const=0;
		};
		class IGetClassField : public virtual ISOperation
		{
		public:
			virtual ISClassField* GetField()const=0;
			virtual SemanticApi::ISOperations::ISOperation* GetLeft()const=0;
		};
		class IGetParameter : public virtual ISOperation
		{
		public:
			virtual ISParameter* GetParameter()const=0;
		};
		class IGetLocal : public virtual ISOperation
		{
		public:
			virtual ISLocalVar* GetVariable()const=0;
		};

		class IGetThis : public virtual ISOperation
		{
		public:
			virtual ISClass* GetOwner()const = 0;
		};

		class ISExpression : public virtual ISOperation, public virtual ISStatement
		{
		public:
			virtual ISOperations::ISOperation* GetFirstOp()const=0;
		};
	}
}