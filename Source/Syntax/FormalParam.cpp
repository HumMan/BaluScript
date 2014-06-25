#include "FormalParam.h"

#include "Class.h"

void TFormalParam::Init(){
	class_pointer = NULL;
	is_ref = false;
	need_push_this = false;
	type = NULL;
}
TFormalParam::TFormalParam(){
	Init();
}
TFormalParam::TFormalParam(TVector<TMethod*> use_methods, bool use_need_push_this){
	Init();
	methods = use_methods;
	need_push_this = use_need_push_this;
}
TFormalParam::TFormalParam(TClass* use_class, bool use_is_ref, TOpArray use_ops){
	Init();
	class_pointer = use_class;
	is_ref = use_is_ref;
	ops = use_ops;
}
TFormalParam::TFormalParam(TClass* use_type){
	Init();
	type = use_type;
}
bool TFormalParam::IsRef()const{
	return is_ref;
}
void TFormalParam::SetIsRef(bool use_is_ref){
	is_ref = use_is_ref;
}
bool TFormalParam::NeedPushThis()const{
	return need_push_this;
}
bool TFormalParam::IsMethods()const{
	return methods.GetCount() != 0;
}
bool TFormalParam::IsType()const{
	return type != NULL;
}
TClass* TFormalParam::GetType()const{
	return type;
}
TVector<TMethod*>& TFormalParam::GetMethods(){
	return methods;
}
TClass* TFormalParam::GetClass()const{
	return class_pointer;
}
bool TFormalParam::IsVoid()const{
	return class_pointer == NULL && (!IsMethods()) && type == NULL;//TODO в дальнейшем methods_pointer не должен считаться void
}
TOpArray& TFormalParam::GetOps(){
	return ops;
}
TOpArray TFormalParam::GetOps()const{
	return ops;
}