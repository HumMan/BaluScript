#pragma once

#include "../SyntaxTreeApi.h"

#include "Accessible.h"

namespace SyntaxInternal
{
	class TClass;
	class TStatements;
	class TParameter;
	class TType;

	class TMethod :public Lexer::TTokenPos, public TAccessible, public SyntaxApi::IMethod
	{
	private:
		std::unique_ptr<TType> ret;
		bool ret_ref;
		TClass* owner;
		bool is_static;
		bool is_extern;

		std::vector<std::unique_ptr<TParameter>> parameters;

		std::unique_ptr<TStatements> statements;
		bool has_return;

		bool one_instruction; //метод состоит из одной инструкции(без операторных скобок, напр.: "func M:int return 5;"

		Lexer::TNameId method_name;
		Lexer::TOperator operator_type;
		SyntaxApi::TClassMember member_type;
	public:
		TMethod(TClass* use_owner, SyntaxApi::TClassMember use_member_type = SyntaxApi::TClassMember::Func);
		~TMethod();
		void ParametersDecl(Lexer::ILexer* source);
		void AnalyzeSyntax(Lexer::ILexer* source, bool realization = true);//realization - используется при получении идентификатора метода (т.к. только прототип без тела метода)

		void SetHasReturn(bool use_has_return);
		bool HasReturn()const;
		SyntaxApi::IStatements* GetStatements()const;
		TStatements* GetStatementsT()const;
		Lexer::TNameId GetName()const;
		SyntaxApi::IClass* GetOwner()const;
		Lexer::TOperator GetOperatorType()const;
		SyntaxApi::TClassMember GetMemberType()const;
		TParameter* GetParamT(size_t use_id)const;
		SyntaxApi::IParameter* GetParam(size_t use_id)const;
		size_t GetParamsCount()const;
		bool IsReturnRef()const;
		bool IsStatic()const;
		bool IsExternal()const;
		//bool IsBytecode();
		void AddParam(TParameter* use_param);
		//bool HasParams(std::vector<std::unique_ptr<TParameter>> &use_params)const;
		void CheckForErrors();
		TType* GetRetTypeT()const;
		SyntaxApi::IType* GetRetType()const;
	};
}

