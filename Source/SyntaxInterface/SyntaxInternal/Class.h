#pragma once

#include "../SyntaxTreeApi.h"

#include "Accessible.h"

namespace SyntaxInternal
{
	class TClassField;
	class TMethod;

	class TCanBeEnumeration : public virtual SyntaxApi::ICanBeEnumeration
	{
		bool is_enum;
		bool is_enum_initialized;
		std::vector<Lexer::TNameId> enums;
	public:
		TCanBeEnumeration()
		{
			is_enum = false;
			is_enum_initialized = false;
		}
		void SetAsEnumeration()
		{
			assert(!is_enum_initialized);
			is_enum_initialized = true;
			is_enum = true;
		}
		bool IsEnumeration()const
		{
			return is_enum;
		}
		std::vector<Lexer::TNameId> GetEnums()const
		{
			return enums;
		}
		size_t GetEnumId(Lexer::TNameId use_enum)const
		{
			assert(is_enum);
			for (size_t i = 0; i < enums.size(); i++)
				if (enums[i] == use_enum)
					return i;
			return -1;
		}
	};
		

	class TClass :public Lexer::TTokenPos, public TCanBeEnumeration, public SyntaxApi::IClass
	{
	private:
		class TClassInternal;
		std::unique_ptr<TClassInternal> _this;
		void AccessDecl(Lexer::ILexer* source, bool& readonly, SyntaxApi::TTypeOfAccess access);
		void AddNested(TClass* use_class);
	public:
		TClass(TClass* use_owner);
		~TClass();
		void AnalyzeSyntax(Lexer::ILexer* source);
		void SetName(Lexer::TNameId name);

		bool IsTemplate()const;
		bool IsExternal()const;

		size_t GetTemplateParamsCount()const;
		Lexer::TNameId GetTemplateParam(int i)const;

		TClass* GetNested(size_t i)const;
		TClass* GetNested(Lexer::TNameId name)const;
		size_t FindNested(Lexer::TNameId name)const;
		size_t GetNestedCount()const;

		Lexer::TNameId GetName()const;
		TClass* GetOwner()const;
		SyntaxApi::IType* GetParent()const;
		std::vector<Lexer::TNameId> GetFullClassName()const;
		size_t GetFieldsCount()const;
		SyntaxApi::IClassField* GetField(size_t i) const;
		size_t GetMethodsCount()const;
		SyntaxApi::IOverloadedMethod* GetMethod(int i) const;

		bool HasDefConstr()const;
		SyntaxApi::IMethod* GetDefaultConstructor() const;
		bool HasDestructor()const;
		SyntaxApi::IMethod* GetDestructor() const;
		SyntaxApi::IOverloadedMethod* GetCopyConstr() const;
		SyntaxApi::IOverloadedMethod* GetMoveConstr() const;
		SyntaxApi::IOverloadedMethod* GetOperator(int i) const;
		SyntaxApi::IOverloadedMethod* GetConversion() const;

		void AddMethod(TMethod* use_method, Lexer::TNameId name);
		void AddOperator(Lexer::TOperator op, TMethod* use_method);
		void AddConversion(TMethod* method);
		void AddDefaultConstr(TMethod* use_method);
		void AddCopyConstr(TMethod* use_method);
		void AddMoveConstr(TMethod* use_method);
		void AddDestr(TMethod* use_method);
		TClassField* EmplaceField(TClass* use_field_owner);
	};
}