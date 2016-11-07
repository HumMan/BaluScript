#pragma once

#include "Parameter.h"
#include "ClassField.h"

class TClass;
class TStatements;
class TParameter;
class TType;

enum class TClassMember
{
	Func = (short)Lexer::TResWord::Func,
	DefaultConstr,
	CopyConstr,
	MoveConstr,
	Destr,
	Operator,
	Conversion
};

class TMethod:public Lexer::TTokenPos,public TAccessible
{
	friend class TOverloadedMethod;
	friend class TSMethod;
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
	TClassMember member_type;
public:
	TMethod(TClass* use_owner, TClassMember use_member_type = TClassMember::Func);
	~TMethod();
	void ParametersDecl(Lexer::ILexer* source);
	void AnalyzeSyntax(Lexer::ILexer* source,bool realization=true);//realization - используется при получении идентификатора метода (т.к. только прототип без тела метода)

	void SetHasReturn(bool use_has_return);
	Lexer::TNameId GetName();
	TClass* GetOwner()const;
	Lexer::TOperator GetOperatorType();
	TClassMember GetMemberType()const;
	int GetParamsSize();
	TParameter* GetParam(int use_id);
	int GetParamsCount();
	bool IsReturnRef()const;
	bool IsStatic();
	bool IsExternal();
	//bool IsBytecode();
	void AddParam(TParameter* use_param);
	bool HasParams(std::vector<std::unique_ptr<TParameter>> &use_params)const;
	void CheckForErrors();
	TType* GetRetType()
	{
		return ret.get();
	}
};

