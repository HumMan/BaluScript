#pragma once

#include "../VirtualMachine/OpArray.h"
#include <vector>

class TClass;
class TMethod;

class TFormalParam
{
	TClass* class_pointer;
	TClass* type;//указатель на тип (для доступа к статическим членам и функциям)
	bool is_ref;
	TOpArray ops;
	std::vector<TMethod*> methods;//указатель на перегруженые методы
	bool need_push_this;//если вызывются методы класса из самого класса 
	void Init();
public:
	TFormalParam();
	TFormalParam(std::vector<TMethod*> use_methods, bool use_need_push_this);
	TFormalParam(TClass* use_class, bool use_is_ref, TOpArray use_ops);
	TFormalParam(TClass* use_type);
	bool IsRef()const;
	void SetIsRef(bool use_is_ref);
	bool NeedPushThis()const;
	bool IsMethods()const;
	bool IsType()const;
	TClass* GetType()const;
	std::vector<TMethod*>& GetMethods();
	TClass* GetClass()const;
	bool IsVoid()const;
	TOpArray& GetOps();
	TOpArray GetOps()const;
};

TFormalParam operator+(const TOpArray& use_left, const TFormalParam& use_right);
