#pragma once

#include <vector>

class TSClass;
class TSMethod;

class TExpressionResult
{
	TSClass* result_value_type;
	TSClass* type;//указатель на тип (для доступа к статическим членам и функциям)
	bool is_ref;
	std::vector<TSMethod*> methods;//указатель на перегруженые методы
	void Init();
public:
	TExpressionResult();
	TExpressionResult(std::vector<TSMethod*> use_methods, bool use_need_push_this);
	TExpressionResult(TSClass* use_class, bool use_is_ref);
	TExpressionResult(TSClass* use_type);
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

class TStackValue
{
	void* internal_buf;
	bool is_ref;
	TSClass* type;
	int actual_size;
public:
	TSClass* GetClass()const;
	bool IsRef()const;
	TStackValue();
	TStackValue(TStackValue&& copy_from);
	TStackValue(const TStackValue& copy_from);
	TStackValue(bool is_ref, TSClass* type);
	void operator=(const TStackValue& right);
	void SetAsReference(void* use_ref);
	void* get();
	~TStackValue();
};

class TStaticValue : public TStackValue
{
	bool is_initialized;
public:
	TStaticValue();
	TStaticValue(bool is_ref, TSClass* type);
	void Initialize();
	bool IsInitialized();
	void* get();
};