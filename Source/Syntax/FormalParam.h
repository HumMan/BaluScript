#pragma once

#include <baluLib.h>
#include "../VirtualMachine/OpArray.h"

class TClass;
class TMethod;

class TFormalParam
{
	TClass* class_pointer;
	TClass* type;//��������� �� ��� (��� ������� � ����������� ������ � ��������)
	bool is_ref;
	TOpArray ops;
	TVector<TMethod*> methods;//��������� �� ������������ ������
	bool need_push_this;//���� ��������� ������ ������ �� ������ ������ 
	void Init();
public:
	TFormalParam();
	TFormalParam(TVector<TMethod*> use_methods, bool use_need_push_this);
	TFormalParam(TClass* use_class, bool use_is_ref, TOpArray use_ops);
	TFormalParam(TClass* use_type);
	bool IsRef()const;
	void SetIsRef(bool use_is_ref);
	bool NeedPushThis()const;
	bool IsMethods()const;
	bool IsType()const;
	TClass* GetType()const;
	TVector<TMethod*>& GetMethods();
	TClass* GetClass()const;
	bool IsVoid()const;
	TOpArray& GetOps();
	TOpArray GetOps()const;
};

inline static TFormalParam operator+(const TOpArray& use_left,const TFormalParam& use_right)
{
	TFormalParam result(use_right);
	result.GetOps()=use_left+use_right.GetOps();
	return result;
}
