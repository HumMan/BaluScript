#pragma once

namespace SyntaxApi
{

	class ICanBeEnumeration
	{
	public:
		virtual bool IsEnumeration() const = 0;
		virtual std::vector<Lexer::TNameId> GetEnums() const = 0;
		virtual int GetEnumId(Lexer::TNameId use_enum) const = 0;
	};

	class IClass :public virtual Lexer::ITokenPos, public virtual ICanBeEnumeration
	{
	public:
		virtual bool IsTemplate()  const = 0;
		virtual bool IsExternal()  const = 0;
		virtual int GetTemplateParamsCount()  const = 0;
		virtual Lexer::TNameId GetTemplateParam(int i) const = 0;
		virtual IClass* GetNested(Lexer::TNameId name)  const = 0;
		virtual Lexer::TNameId GetName()  const = 0;
		virtual IClass* GetOwner()  const = 0;
		virtual IType* GetParent()  const = 0;
		virtual std::vector<Lexer::TNameId> GetFullClassName()  const = 0;
		virtual int GetFieldsCount()  const = 0;
		virtual IClassField* GetField(int i)  const = 0;
		virtual int GetMethodsCount()  const = 0;
		virtual IOverloadedMethod* GetMethod(int i)  const = 0;
		virtual bool HasDefConstr() const = 0;
		virtual IMethod* GetDefaultConstructor()  const = 0;
		virtual bool HasDestructor() const = 0;
		virtual IMethod* GetDestructor()  const = 0;
		virtual IOverloadedMethod* GetCopyConstr()  const = 0;
		virtual IOverloadedMethod* GetMoveConstr()  const = 0;
		virtual IOverloadedMethod* GetOperator(int i)  const = 0;
		virtual IOverloadedMethod* GetConversion()  const = 0;
		virtual int GetNestedCount() const = 0;
		virtual IClass* GetNested(int i) const = 0;
	};
}