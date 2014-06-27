#pragma once

#include "Parameter.h"
#include "../notOptimizedProgram.h"
#include "ClassField.h"

class TClass;
class TStatements;

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
	TOpArray ops_array;
	std::vector<std::unique_ptr<TParameter>> param;

	int params_size;

	std::unique_ptr<TStatements> statements;
	bool has_return;
	
	bool declared; //настроены типы параметров и возвращаемого значения
	bool build;    //сгенерировано тело метода
	bool one_instruction; //метод состоит из одной инструкции(без операторных скобок, напр.: "func M:int return 5;"

	TNameId method_name;
	TOperator::Enum operator_type;
	TClassMember::Enum member_type;

	TMethod* pre_event;
	TMethod* post_event;

	int is_bytecode;//0 - not_used; 1 - true; 2 - false
	//если 0 то определяется через statements
	
	//TODO
	//bool only_inline;// метод можно только встраивать, но не вызывать

public:
	void InitOwner(TClass* use_owner);
	///<summary>Используется для вызова автоматически созданного конструктора для пользовательского/внешнего метода</summary>
	void SetPreEvent(TMethod* use_event);
	void SetPostEvent(TMethod* use_event);
	void SetAs(TOpArray use_ops, TClass* use_ret_class_pointer, bool use_ret_ref, bool use_is_static, int use_is_bytecode = 0);
	TMethod* GetPreEvent();
	TMethod* GetPostEvent();
	TMethod(TClass* use_owner, TClassMember::Enum use_member_type = TClassMember::Func);

	void ParametersDecl(TLexer& source);
	void AnalyzeSyntax(TLexer& source,bool realization=true);//realization - используется при получении идентификатора метода (т.к. только прототип без тела метода)
	TVariable* GetVar(TNameId name);

	void SetHasReturn(bool use_has_return);
	TNameId GetName();
	TClass* GetOwner()const;
	TOperator::Enum GetOperatorType();
	TClassMember::Enum GetMemberType()const;
	TOpArray GetOps()const;
	TClass* GetRetClass();
	int GetParamsSize();
	int GetRetSize();
	TParameter* GetParam(int use_id);
	int GetParamsCount();
	bool IsReturnRef()const;
	bool IsStatic();
	bool IsExternal();
	bool IsBytecode();
	void AddParam(TParameter* use_param);
	void CalcParamSize();
	bool HasParams(std::vector<std::unique_ptr<TParameter>> &use_params)const;
	void CheckForErrors();
	TOpArray BuildParametersDestructor(TNotOptimizedProgram &program);
	void Build(TNotOptimizedProgram &program);
	void Declare();
	void BuildFormalParamConversion(TNotOptimizedProgram &program,TFormalParam& formal_par, TClass* param_class,bool param_ref);
	void BuildFormalParamsConversion(TNotOptimizedProgram &program, std::vector<TFormalParam> &formal_params);
	TFormalParam BuildCall(TNotOptimizedProgram &program,std::vector<TFormalParam> &formal_params);
	TFormalParam BuildCall(TNotOptimizedProgram &program,TClass* par0,bool par0_ref,TOpArray& par0_ops,TClass* par1,bool par1_ref,TOpArray& par1_ops);
	TFormalParam BuildCall(TNotOptimizedProgram &program,TClass* par0,bool par0_ref,TOpArray& par0_ops);
	TFormalParam BuildCall(TNotOptimizedProgram &program);
};

