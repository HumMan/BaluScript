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

void TFormalParamWithConversions::RunConversion(std::vector<TStackValue> &stack)
{
	if (copy_constr != NULL)
		copy_constr->Run(stack);
}

TFormalParamWithConversions::TFormalParamWithConversions()
{
	this->copy_constr = NULL;
	this->conversion = NULL;
}
void TFormalParamWithConversions::BuildConvert(TFormalParam from_result, TSClass* param_class, bool param_ref)
{
	result = from_result;

	//если необходимо преобразование типа формального параметра то добавляем его
	if (result.GetClass() != param_class)
	{
		conversion = result.GetClass()->GetConversion(result.IsRef(), param_class);
		if (result.IsRef() && !param_ref)
		{
			if (conversion == NULL)
			{
				copy_constr = result.GetClass()->GetCopyConstr();
				conversion = result.GetClass()->GetConversion(false, param_class);
			}
		}
		assert(conversion != NULL);//ошибка в FindMethod
	}
	//если в стеке находится ссылка, а в качестве параметра требуется значение, то добавляем преобразование
	else if (result.IsRef() && !param_ref)
	{
		copy_constr = result.GetClass()->GetCopyConstr();
	}
}

TSClass* TStackValue::GetClass()
{
	return type;
}
bool TStackValue::IsRef()
{
	return is_ref;
}
TStackValue::TStackValue()
{
	internal_buf = NULL;
	type = NULL;
}
TStackValue::TStackValue(bool is_ref, TSClass* type)
{
	this->is_ref = is_ref;
	this->type = type;
	if (is_ref)
		actual_size = 1;
	else
		actual_size = type->GetSize();
	internal_buf = new int[actual_size];
}
TStackValue::~TStackValue()
{
	delete internal_buf;
}