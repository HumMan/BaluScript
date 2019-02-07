#pragma once

namespace SemanticApi
{
	enum class SpecialClassMethodType
	{
		NotSpecial,
		AutoDefConstr,
		AutoCopyConstr,
		AutoDestructor,
		AutoAssignOperator,

		DefaultConstr,
		CopyConstr,
		Destructor,
		Operator,
		Conversion
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

	enum class TNodeWithTemplatesType
	{
		Class,
		Template,
		Realization,
		SurrogateTemplateParam,
		Unknown
	};

	class INodeWithTemplates
	{
	public:
		virtual TTemplateParameter GetTemplateParam(int i)const = 0;
		virtual TNodeWithTemplatesType GetType()const = 0;
	};

	class INodeWithOffset
	{
	public:
		virtual int GetOffset()const = 0;
		virtual void SetOffset(int use_offset) = 0;
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

	enum class Filter
	{
		True,
		False,
		NotSet
	};

	class ISClass: public virtual ISNodeWithSize, public virtual ISNodeSignatureLinked,
		public virtual ISNodeBodyLinked, public virtual INodeWithTemplates, public virtual ISNodeWithAutoMethods
	{
	public:

		virtual ISClass * GetClass(Lexer::TNameId use_name)=0;

		virtual ISMethod* GetAutoDefConstr()const=0;
		virtual ISMethod* GetAutoDestr()const=0;

		virtual ISMethod* GetDefConstr()const=0;
		virtual ISMethod* GetDestructor()const = 0;
		virtual ISMethod* GetCopyConstr()const = 0;
		virtual ISMethod* GetAssignOperator()const = 0;

		virtual bool GetOperators(std::vector<ISMethod*> &result, Lexer::TOperator op)const=0;
		virtual ISClassField* GetField(size_t i)const=0;
		virtual std::vector<ISClassField*> GetFields()const = 0;
		virtual bool GetCopyConstructors(std::vector<ISMethod*> &result)const=0;
		virtual ISMethod* GetAutoCopyConstr()const=0;
		virtual ISMethod* GetAutoAssignOperator()const=0;
		virtual void GetMethods(std::vector<ISMethod*> &result)const=0;
		virtual bool GetMethods(std::vector<ISMethod*> &result, Lexer::TNameId use_method_name,
			Filter is_static = Filter::NotSet, bool scan_owner = true)const = 0;
		virtual ISMethod* GetConversion(bool source_ref, ISClass* target_type)const=0;

		virtual ISClass* GetNestedByFullName(std::vector<Lexer::TNameId> full_name, size_t curr_id)=0;

		virtual size_t GetNestedCount()const=0;
		virtual ISClass* GetNested(size_t index)const=0;

		virtual bool IsExternal()const = 0;
		virtual SyntaxApi::IClass* IGetSyntax()const = 0;
		virtual ISClass* GetOwner()const = 0;

		virtual void SetExternal(const std::vector<SemanticApi::TExternalSMethod>& bindings, int& curr_bind) = 0;
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
		virtual bool IsRef()const = 0;
		virtual bool IIsEqualTo(const ISParameter* right)const=0;
		virtual SyntaxApi::IParameter* IGetSyntax()const=0;
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
		virtual VariableType GetVariableType()const = 0;
	};

	class ISMethod: public virtual ISpecialClassMethod
	{
	public:
		virtual size_t GetParametersSize()const=0;
		virtual size_t GetReturnSize()const = 0;
		virtual bool IsExternal()const = 0;
		virtual void SetAsExternal(SemanticApi::TExternalSMethod method)=0;
		virtual ISClass* GetOwner()const = 0;
		virtual ISClass* GetRetClass()const = 0;
		virtual ISParameter* GetParam(size_t use_id)const = 0;
		//virtual std::vector<ISParameter*> GetParameters()const = 0;
		virtual IVariable* GetVar(Lexer::TNameId name)const = 0;
		virtual size_t GetParamsCount()const = 0;
		//virtual bool HasParams(const std::vector<ISParameter*> &use_params)const = 0;
		virtual TExternalSMethod GetExternal()const=0;
		virtual bool IsReturnRef()const = 0;
		virtual ISStatements* GetStatements()const=0;
		virtual SyntaxApi::TClassMember GetMemberType()const=0;
		virtual bool IsStatic()const=0;
		virtual Lexer::TOperator GetOperatorType()const=0;
		virtual SyntaxApi::IMethod* IGetSyntax()const = 0;
	};

	class ISStatement
	{
	public:
		virtual ISClass* IGetOwner()const = 0;
		virtual ISMethod* IGetMethod()const = 0;
		virtual void Accept(ISStatementVisitor* visitor) = 0;
	};

	class ISLocalVar: public virtual INodeWithOffset, public virtual ISStatement
	{
	public:
		virtual ISClass* GetClass()const = 0;
		virtual bool IsStatic()const=0;
		virtual const ISType* GetType()const = 0;
		virtual ISConstructObject* GetConstructObject()const=0;
		virtual ISOperations::ISExpression* GetAssignExpression()const=0;
	};

	class ISFor : public virtual ISStatement
	{
	public:
		virtual ISOperations::ISExpression* GetCompare()const = 0;
		virtual IActualParamWithConversion* GetCompareConversion()const = 0;
		virtual ISStatements* GetStatements()const = 0;
		virtual ISStatements* GetIncrement()const=0;
	};

	class IActualParamWithConversion
	{
	public:
		virtual ISOperations::ISOperation* GetExpression()const=0;
		virtual ISMethod* GetCopyConstr()const=0;
		virtual bool IsRefToRValue()const=0;
		virtual ISMethod* GetConverstion()const=0;
		virtual const IExpressionResult* GetResult()const = 0;
	};

	class IActualParameters
	{
	public:
		virtual std::vector<const IActualParamWithConversion*> GetInput()const=0;
	};

	class ISConstructObject
	{
	public:
		virtual ISOperations::ISExpression_TMethodCall* GetConstructorCall()const=0;
		virtual ISClass* GetObjectType()const = 0;
	};
	
	class ISIf: public virtual ISStatement
	{
	public:
		virtual ISOperations::ISExpression* GetBoolExpr()const=0;
		virtual IActualParamWithConversion* GetBoolExprConversion()const = 0;
		virtual ISStatements* GetStatements()const = 0;
		virtual ISStatements* GetElseStatements()const = 0;
	};

	class TVarDecl
	{
	public:
		int stmt_id;
		SemanticApi::ISLocalVar* pointer;
		TVarDecl() {}
		TVarDecl(int use_stmt_id, SemanticApi::ISLocalVar* use_pointer) :stmt_id(use_stmt_id), pointer(use_pointer) {}
	};

	class ISStatements : public virtual ISStatement
	{
	public:
		virtual std::vector<ISStatement*> GetStatements()const=0;
		virtual std::vector<TVarDecl> GetVarDeclarations()const=0;
	};

	

	class ISBytecode : public virtual ISStatement
	{
	public:
		virtual const std::vector<TOpcode::Enum>& GetBytecode()const = 0;
	};

	class ISReturn : public virtual ISStatement
	{
	public:
		virtual ISOperations::ISExpression* GetResult()const=0;
		virtual IActualParamWithConversion* GetConverstion()const=0;
	};

	class ISWhile : public virtual ISStatement
	{
	public:
		virtual ISOperations::ISExpression* GetCompare()const=0;
		virtual IActualParamWithConversion* GetCompareConversion()const=0;
		virtual ISStatements* GetStatements()const=0;
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
