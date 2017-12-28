#pragma once

namespace SemanticApi
{
	class ISpecialClassMethod
	{

	};

	class ISClass: public virtual ISNodeWithSize, public virtual ISNodeSignatureLinked, public virtual ISNodeBodyLinked, public virtual ISNodeWithAutoMethods
	{

	};

	class ISParameter
	{

	};

	enum class TVariableType
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
		virtual TVariableType GetType()const = 0;
	};

	class ISMethod: public virtual ISpecialClassMethod
	{
	public:
		virtual size_t GetParametersSize()const=0;
		virtual size_t GetReturnSize()const = 0;

		virtual ISClass* GetOwner()const = 0;
		virtual ISClass* GetRetClass()const = 0;
		virtual ISParameter* GetParam(int use_id)const = 0;
		//virtual std::vector<ISParameter*> GetParameters()const = 0;
		virtual IVariable* GetVar(Lexer::TNameId name)const = 0;
		virtual size_t GetParamsCount()const = 0;
		//virtual bool HasParams(const std::vector<ISParameter*> &use_params)const = 0;
	};


}
