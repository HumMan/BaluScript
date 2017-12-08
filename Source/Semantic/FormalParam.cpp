#include <string.h>

#include "FormalParam.h"

#include "SClass.h"
#include "SMethod.h"
#include "SStatements.h"

void TExpressionResult::Init()
{
	result_value_type = nullptr;
	is_ref = false;
	type = nullptr;
}
TExpressionResult::TExpressionResult()
{
	Init();
}
TExpressionResult::TExpressionResult(std::vector<TSMethod*> use_methods, bool use_need_push_this)
{
	Init();
	methods = use_methods;
}
TExpressionResult::TExpressionResult(TSClass* use_class, bool use_is_ref)
{
	Init();
	result_value_type = use_class;
	is_ref = use_is_ref;
}
TExpressionResult::TExpressionResult(TSClass* use_type)
{
	Init();
	type = use_type;
}
bool TExpressionResult::IsRef()const
{
	return is_ref;
}
void TExpressionResult::SetIsRef(bool use_is_ref)
{
	is_ref = use_is_ref;
}
bool TExpressionResult::IsMethods()const
{
	return methods.size() != 0;
}
bool TExpressionResult::IsType()const
{
	return type != nullptr;
}
TSClass* TExpressionResult::GetType()const
{
	return type;
}
std::vector<TSMethod*>& TExpressionResult::GetMethods()
{
	return methods;
}
TSClass* TExpressionResult::GetClass()const
{
	return result_value_type;
}
bool TExpressionResult::IsVoid()const
{
	return result_value_type == nullptr && (!IsMethods()) && type == nullptr;//TODO в дальнейшем methods_pointer не должен считаться void
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
	internal_buf = nullptr;
	type = nullptr;
}
TStackValue::TStackValue(const TStackValue& copy_from)
{
	actual_size = copy_from.actual_size;
	is_ref = copy_from.is_ref;
	type = copy_from.type;

	if (copy_from.internal_buf != nullptr)
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
		internal_buf = nullptr;
	}
}

TStackValue::TStackValue(TStackValue&& copy_from)
{
	actual_size = copy_from.actual_size;
	is_ref = copy_from.is_ref;
	type = copy_from.type;
	internal_buf = copy_from.internal_buf;

	copy_from.actual_size = 0;
	copy_from.is_ref = false;
	copy_from.type = nullptr;
	copy_from.internal_buf = nullptr;
}

void TStackValue::SetAsReference(void* use_ref)
{
	assert(is_ref);
	internal_buf = use_ref;
}

void* TStackValue::get()
{
	return internal_buf;
}

int TStackValue::GetSize()
{
	return type->GetSize();
}

TStackValue::TStackValue(bool is_ref, TSClass* type)
{
	assert(type != nullptr);
	this->is_ref = is_ref;
	this->type = type;
	if (is_ref)
	{
		actual_size = 1;
		internal_buf = nullptr;
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
		internal_buf = nullptr;
	}
	actual_size = right.actual_size;
	is_ref = right.is_ref;
	type = right.type;

	if (right.internal_buf != nullptr)
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
		internal_buf = nullptr;
	}
}
TStackValue::~TStackValue()
{
	if (!is_ref)
	{
		if (internal_buf!=nullptr)
			memset(internal_buf, 0xfeefee, actual_size);
		delete internal_buf;
		internal_buf = nullptr;
	}
}

TStaticValue::TStaticValue()
{
	is_initialized = false;
}

TStaticValue::TStaticValue(bool is_ref, TSClass* type)
	:TStackValue(is_ref,type)
{
	is_initialized = false;
}

void TStaticValue::Initialize()
{
	assert(!is_initialized);
	is_initialized = true;
}

bool TStaticValue::IsInitialized()
{
	return is_initialized;
}

void* TStaticValue::get()
{
	//assert(is_initialized);
	return TStackValue::get();
}