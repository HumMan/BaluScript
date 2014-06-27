#pragma once

#include "Variable.h"
#include "../lexer.h"
#include "Class.h"

class TParameter:public TVariable,public TTokenPos
{
	TType type;
	bool is_ref;
	TNameId name;
	TClass* owner;
	TMethod* method;
	int offset;
public:
	TParameter(TClass* use_owner,TMethod* use_method):TVariable(false,TVariableType::Parameter)
		,type(use_owner),is_ref(false)
		,owner(use_owner),method(use_method),offset(-1)
	{
	}
	virtual ~TParameter(){}
	void InitOwner(TClass* use_owner,TMethod* use_method)
	{
		owner=use_owner;
		method=use_method;
		type.InitOwner(use_owner);
	}
	bool IsEqualTo(const TParameter& right)const
	{
		return type.IsEqualTo(right.type)&&is_ref==right.is_ref;
	}
	void SetAs(bool use_is_ref,TClass* use_class_pointer)
	{
		is_ref=use_is_ref;
		type.SetAs(use_class_pointer);
	}
	void AnalyzeSyntax(TLexer& source);
	TNameId GetName()const{
		return name;
	}
	TClass* GetClass(){
		return type.GetClass();
	}
	TNameId GetClassName()const;
	void SetOffset(int use_offset){
		offset=use_offset;
	}
	int GetSize();
	int GetOffset(){
		return offset;
	}
	bool IsRef()const{
		return is_ref;
	}
	TFormalParam PushRefToStack(TNotOptimizedProgram &program)
	{
		TOpArray ops_array;
		if(is_ref)
			program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,offset),ops_array);
		else
			program.Push(TOp(TOpcode::PUSH_LOCAL_REF,offset),ops_array);
		return TFormalParam(type.GetClass(),true,ops_array);
	}
};