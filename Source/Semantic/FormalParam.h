#pragma once

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