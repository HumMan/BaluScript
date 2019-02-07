#pragma once

namespace SyntaxApi
{

	class ICanBeEnumeration
	{
	public:
		virtual bool IsEnumeration() const = 0;
		virtual std::vector<Lexer::TNameId> GetEnums() const = 0;
		virtual size_t GetEnumId(Lexer::TNameId use_enum) const = 0;
	};

	class IClass :public virtual Lexer::ITokenPos, public virtual ICanBeEnumeration
	{
	public:
		virtual bool IsTemplate()  const = 0;
		virtual bool IsExternal()  const = 0;
		virtual size_t GetTemplateParamsCount()  const = 0;
		virtual Lexer::TNameId GetTemplateParam(int i) const = 0;
		virtual IClass* GetNested(Lexer::TNameId name)  const = 0;
		virtual Lexer::TNameId GetName()  const = 0;
		virtual IClass* GetOwner()  const = 0;
		virtual std::vector<Lexer::TNameId> GetFullClassName()  const = 0;
		virtual size_t GetFieldsCount()  const = 0;
		virtual IClassField* GetField(size_t i)  const = 0;
		virtual size_t GetMethodsCount()  const = 0;
		virtual IOverloadedMethod* GetMethod(int i)  const = 0;
		virtual bool HasDefConstr() const = 0;
		virtual IMethod* GetDefaultConstructor()  const = 0;
		virtual bool HasDestructor() const = 0;
		virtual IMethod* GetDestructor()  const = 0;
		virtual IOverloadedMethod* GetCopyConstr()  const = 0;
		virtual IOverloadedMethod* GetMoveConstr()  const = 0;
		virtual IOverloadedMethod* GetOperator(int i)  const = 0;
		virtual IOverloadedMethod* GetConversion()  const = 0;
		virtual size_t GetNestedCount() const = 0;
		virtual IClass* GetNested(size_t i) const = 0;
	};
}