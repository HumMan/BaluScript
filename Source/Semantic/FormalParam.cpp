#include "FormalParam.h"

#include "SClass.h"
#include "SMethod.h"
#include "SStatements.h"
#include "../Syntax/Statements.h"

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

void TFormalParamWithConversions::RunConversion(TStackValue &value)
{
	if (ref_to_rvalue)
	{
		if (copy_constr != NULL)
		{
			std::vector<TStackValue> constr_params;
			constr_params.push_back(value);
			TStackValue constr_result;
			TStackValue constructed_object(false, value.GetClass());
			copy_constr->Run(constr_params, constr_result, constructed_object);
			value = constructed_object;
		}
		else
		{
			TStackValue constructed_object(false, value.GetClass());
			memcpy(constructed_object.get(), value.get(), value.GetClass()->GetSize()*sizeof(int));
			value = constructed_object;
		}
	}
	if (conversion != NULL)
	{
		std::vector<TStackValue> conv_params;
		conv_params.push_back(value);
		TStackValue result;
		conversion->Run(conv_params, result, value);
		value = result;
	}
}

TFormalParamWithConversions::TFormalParamWithConversions()
{
	this->copy_constr = NULL;
	this->conversion = NULL;
	this->ref_to_rvalue = false;
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
		ref_to_rvalue = true;
	}
}

TSClass* TStackValue::GetClass()const
{
	return type;
}
bool TStackValue::IsRef()const
{
	return is_ref;
}
TStackValue::TStackValue()
{
	is_ref = false;
	internal_buf = NULL;
	type = NULL;
}
TStackValue::TStackValue(const TStackValue& copy_from)
{
	actual_size = copy_from.actual_size;
	is_ref = copy_from.is_ref;
	type = copy_from.type;

	if (copy_from.internal_buf != NULL)
	{
		if (is_ref)
		{
			internal_buf = copy_from.internal_buf;
		}
		else
		{
			internal_buf = new int[actual_size];
			memcpy(internal_buf, copy_from.internal_buf, actual_size*sizeof(int));
		}
	}
	else
	{
		internal_buf = NULL;
	}
}
void TStackValue::SetAsReference(void* use_ref)
{
	assert(is_ref);
	internal_buf = use_ref;
}
TStackValue::TStackValue(bool is_ref, TSClass* type)
{
	this->is_ref = is_ref;
	this->type = type;
	if (is_ref)
	{
		actual_size = 1;
		internal_buf = NULL;
	}
	else
	{
		actual_size = type->GetSize();
		internal_buf = new int[actual_size];
	}
}
void TStackValue::operator=(const TStackValue& right)
{
	if (!is_ref)
	{
		delete internal_buf;
		internal_buf = NULL;
	}
	actual_size = right.actual_size;
	is_ref = right.is_ref;
	type = right.type;

	if (right.internal_buf != NULL)
	{
		if (is_ref)
		{
			internal_buf = right.internal_buf;
		}
		else
		{
			internal_buf = new int[actual_size];
			memcpy(internal_buf, right.internal_buf, actual_size*sizeof(int));
		}
	}
	else
	{
		internal_buf = NULL;
	}
}
TStackValue::~TStackValue()
{
	if (!is_ref)
	{
		delete internal_buf;
		internal_buf = NULL;
	}
}