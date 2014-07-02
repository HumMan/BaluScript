#include "FormalParam.h"

#include "SClass.h"
#include "SMethod.h"
#include "SStatements.h"

void TFormalParam::Init(){
	class_pointer = NULL;
	is_ref = false;
	type = NULL;
}
TFormalParam::TFormalParam(){
	Init();
}
TFormalParam::TFormalParam(std::vector<TSMethod*> use_methods, bool use_need_push_this){
	Init();
	methods = use_methods;
}
TFormalParam::TFormalParam(TSClass* use_class, bool use_is_ref){
	Init();
	class_pointer = use_class;
	is_ref = use_is_ref;
}
TFormalParam::TFormalParam(TSClass* use_type){
	Init();
	type = use_type;
}
bool TFormalParam::IsRef()const{
	return is_ref;
}
void TFormalParam::SetIsRef(bool use_is_ref){
	is_ref = use_is_ref;
}
bool TFormalParam::IsMethods()const{
	return methods.size() != 0;
}
bool TFormalParam::IsType()const{
	return type != NULL;
}
TSClass* TFormalParam::GetType()const{
	return type;
}
std::vector<TSMethod*>& TFormalParam::GetMethods(){
	return methods;
}
TSClass* TFormalParam::GetClass()const{
	return class_pointer;
}
bool TFormalParam::IsVoid()const{
	return class_pointer == NULL && (!IsMethods()) && type == NULL;//TODO в дальнейшем methods_pointer не должен считаться void
}