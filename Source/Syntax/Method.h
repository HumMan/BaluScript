#pragma once

#include "Parameter.h"
#include "ClassField.h"

class TClass;
class TStatements;
class TParameter;

namespace TClassMember
{
	enum Enum
	{
		Func=TResWord::Func,
		Constr,
		Destr,
		Operator,
		Conversion,
		Enumeration
	};
}

class TMethod:public TTokenPos,public TAccessible
{
	friend class TOverloadedMethod;
private:
	TType ret;
	bool ret_ref;
	TClass* owner;
	bool is_static;
	bool is_extern;

	std::vector<std::unique_ptr<TParameter>> param;

	std::unique_ptr<TStatements> statements;
	bool has_return;
	
	bool one_instruction; //метод состоит из одной инструкции(без операторных скобок, напр.: "func M:int return 5;"

	TNameId method_name;
	TOperator::Enum operator_type;
	TClassMember::Enum member_type;


	int is_bytecode;//0 - not_used; 1 - true; 2 - false
	//если 0 то определяется через statements
public:

	TMethod(TClass* use_owner, TClassMember::Enum use_member_type = TClassMember::Func);

	void ParametersDecl(TLexer& source);
	void AnalyzeSyntax(TLexer& source,bool realization=true);//realization - используется при получении идентификатора метода (т.к. только прототип без тела метода)
	TVariable* GetVar(TNameId name);

	void SetHasReturn(bool use_has_return);
	TNameId GetName();
	TClass* GetOwner()const;
	TOperator::Enum GetOperatorType();
	TClassMember::Enum GetMemberType()const;
	int GetParamsSize();
	int GetRetSize();
	TParameter* GetParam(int use_id);
	int GetParamsCount();
	bool IsReturnRef()const;
	bool IsStatic();
	bool IsExternal();
	bool IsBytecode();
	void AddParam(TParameter* use_param);
	bool HasParams(std::vector<std::unique_ptr<TParameter>> &use_params)const;
	void CheckForErrors();
};

