#pragma once

namespace SemanticApi
{
	enum SpecialClassMethodType
	{
		NotSpecial,
		AutoDefConstr,
		AutoCopyConstr,
		AutoDestructor,
		AutoAssignOperator,

		Default,
		CopyConstr,
		Destructor
	};

	class ISpecialClassMethod
	{
	public:
		virtual SpecialClassMethodType GetType()const = 0;
	};

	/*class ITemplateParameter
	{
	public:
		virtual bool IsValue()const = 0;
		virtual int GetValue()const = 0;
		virtual ISClass* GetType()const = 0;
	};*/

	class TTemplateParameter
	{
	public:
		bool is_value;
		int value;
		ISClass* type;

		//bool IsValue()const;
		//int GetValue()const;
		//SemanticApi::ISClass* GetType()const;

		bool IsValue()const
		{
			return is_value;
		}
		int GetValue()const
		{
			return value;
		}
		SemanticApi::ISClass* GetType()const
		{
			return type;
		}
	};

	class INodeWithTemplates
	{
	public:
		virtual TTemplateParameter GetTemplateParam(int i)const = 0;
	};

	class INodeWithOffset
	{
	public:
		virtual int GetOffset()const = 0;
	};

	class ISMultifield
	{
	public:
		virtual bool HasSizeMultiplier()const=0;
		virtual size_t GetSizeMultiplier()const=0;
	};

	class ISClassField: public virtual ISMultifield, public virtual INodeWithOffset
	{
	public:
		virtual ISClass* GetClass()const=0;
		virtual ISClass* GetOwner()const = 0;
		virtual bool IsStatic()const = 0;
	};

	class ISClass: public virtual ISNodeWithSize, public virtual ISNodeSignatureLinked,
		public virtual ISNodeBodyLinked, public virtual INodeWithTemplates, public virtual ISNodeWithAutoMethods
	{
	public:
		virtual ISMethod* GetDefConstr()const=0;
		virtual ISMethod* GetDestructor()const = 0;
		virtual ISMethod* GetCopyConstr()const = 0;
		virtual ISMethod* GetMoveConstr()const = 0;
		virtual ISMethod* GetAssignOperator()const = 0;

		virtual bool GetOperators(std::vector<ISMethod*> &result, Lexer::TOperator op)const=0;
		virtual ISClassField* GetField(size_t i)const=0;
		virtual bool GetCopyConstructors(std::vector<ISMethod*> &result)const=0;
	};

	class TFormalParameter
	{
		ISClass* type;
		bool is_ref;
	public:
		TFormalParameter()
		{
			type = nullptr;
			is_ref = false;
		}
		TFormalParameter(ISClass* type, bool is_ref)
		{
			assert(type != nullptr);
			this->type = type;
			this->is_ref = is_ref;
		}
		ISClass* GetClass()
		{
			assert(type != nullptr);
			return type;
		}
		bool IsRef()
		{
			assert(type != nullptr);
			return is_ref;
		}
	};

	class ISParameter: public virtual INodeWithOffset
	{
	public:
		virtual TFormalParameter AsFormalParameter()const=0;
		virtual ISClass* GetClass()const=0;
	};

	enum class VariableType
	{
		ClassField,
		Parameter,
		Local,
		This,
		TemplateValue
	};

	class IVariable
	{
	public:
		virtual VariableType GetType()const = 0;
	};

	class ISMethod: public virtual ISpecialClassMethod
	{
	public:
		virtual size_t GetParametersSize()const=0;
		virtual size_t GetReturnSize()const = 0;
		virtual bool IsExternal()const = 0;
		virtual ISClass* GetOwner()const = 0;
		virtual ISClass* GetRetClass()const = 0;
		virtual ISParameter* GetParam(int use_id)const = 0;
		//virtual std::vector<ISParameter*> GetParameters()const = 0;
		virtual IVariable* GetVar(Lexer::TNameId name)const = 0;
		virtual size_t GetParamsCount()const = 0;
		//virtual bool HasParams(const std::vector<ISParameter*> &use_params)const = 0;
		virtual TExternalSMethod GetExternal()const=0;
		virtual bool IsReturnRef()const = 0;
	};

	class ISLocalVar: public virtual INodeWithOffset
	{
	public:
		virtual ISClass* GetClass()const = 0;
		virtual bool IsStatic()const=0;
	};

	class ISFor
	{
	public:
	};

	class IActualParamWithConversion
	{
	public:
		virtual ISOperations::ISOperation* GetExpression()const=0;
		virtual ISMethod* GetCopyConstr()const=0;
		virtual bool IsRefToRValue()const=0;
		virtual ISMethod* GetConverstion()const=0;
	};

	class IActualParameters
	{
	public:
	};

	class ISConstructObject
	{
	public:
		virtual ISOperations::ISExpression_TMethodCall* GetConstructorCall()const=0;
	};

	class ISBytecode
	{
	public:
	};

	class ISIf
	{
	public:
	};

	class ISStatements
	{
	public:
	};

	class ISStatement
	{
	public:
	};

	class ISReturn
	{
	public:
	};

	class ISWhile
	{
	public:
	};

	class ISType
	{
	public:
		virtual ISClass* GetClass()const = 0;
	};

	class IExpressionResult
	{
	public:
		virtual bool IsRef()const=0;
		virtual bool IsMethods()const = 0;
		virtual bool IsType()const = 0;
		virtual ISClass* GetType()const = 0;
		virtual std::vector<ISMethod*>& GetMethods() = 0;
		virtual ISClass* GetClass()const = 0;
		virtual bool IsVoid()const = 0;
	};
}
