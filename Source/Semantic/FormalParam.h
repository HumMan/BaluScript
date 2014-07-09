﻿#pragma once

#include <vector>

class TSClass;
class TSMethod;

class TFormalParam
{
	TSClass* class_pointer;
	TSClass* type;//указатель на тип (для доступа к статическим членам и функциям)
	bool is_ref;
	std::vector<TSMethod*> methods;//указатель на перегруженые методы
	void Init();
public:
	TFormalParam();
	TFormalParam(std::vector<TSMethod*> use_methods, bool use_need_push_this);
	TFormalParam(TSClass* use_class, bool use_is_ref);
	TFormalParam(TSClass* use_type);
	bool IsRef()const;
	void SetIsRef(bool use_is_ref);
	bool NeedPushThis()const;
	bool IsMethods()const;
	bool IsType()const;
	TSClass* GetType()const;
	std::vector<TSMethod*>& GetMethods();
	TSClass* GetClass()const;
	bool IsVoid()const;
};

class TVoid :public TFormalParam
{
public:
	TVoid() :TFormalParam(NULL, false){}
};

struct TFormalParamWithConversions
{
	TFormalParam result;
	TSMethod* copy_constr;//используется если необходимо преобразование из lvalue в rvalue
	TSMethod* conversion;//используется если необходимо преобразование параметра
	TFormalParamWithConversions();
	void BuildConvert(TFormalParam from_result, TSClass* param_class, bool param_ref);
	void RunConversion(std::vector<TStackValue> &stack);
};

class TStackValue
{
	void* internal_buf;
	bool is_ref;
	TSClass* type;
	int actual_size;
public:
	TSClass* GetClass();
	bool IsRef();
	TStackValue();
	TStackValue(bool is_ref, TSClass* type);
	~TStackValue();
};