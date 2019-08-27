#include <string.h>

#include "FormalParam.h"

SemanticApi::ISClass* TStackValue::GetClass()const
{
	return type;
}
bool TStackValue::IsRef()const
{
	return is_ref;
}
TStackValue::TStackValue()
{
	Init();
}

void TStackValue::Init()
{
	actual_size = 0;
	is_ref = false;
	type = nullptr;
	internal_buf = nullptr;
}

TStackValue::TStackValue(TStackValue&& copy_from)
{
	actual_size = copy_from.actual_size;
	is_ref = copy_from.is_ref;
	type = copy_from.type;
	internal_buf = copy_from.internal_buf;

	copy_from.Init();
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

TStackValue::TStackValue(bool is_ref, SemanticApi::ISClass* type)
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
void TStackValue::operator=(TStackValue& right)
{
	assert(internal_buf == nullptr);
	this->internal_buf = right.internal_buf;
	this->actual_size = right.actual_size;
	this->is_ref = right.is_ref;
	this->type = right.type;

	right.Init();
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

TStaticValue::TStaticValue(bool is_ref, SemanticApi::ISClass* type)
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