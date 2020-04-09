#pragma once

#include <vector>
#include <assert.h>

#include "../../Include/common.h"

#include "../SemanticInterface/SemanticTreeApi.h"

class TSClass;

class BALUSCRIPT_DLL_INTERFACE TStackValue
{
	void* internal_buf;
	bool is_ref;
	SemanticApi::ISClass* type;
	int actual_size;

	void Init();

public:
	SemanticApi::ISClass* GetClass()const;
	bool IsRef()const;
	TStackValue();
	TStackValue(TStackValue&& copy_from);
	TStackValue(bool is_ref, SemanticApi::ISClass* type);
	TStackValue& operator=(TStackValue&& right);
	void SetAsReference(void* use_ref);
	void* get();
	int GetSize();
	template<class T>
	T& get_as()
	{
		int result_size = LexerIntSizeOf(sizeof(T));
		assert(GetSize() == (result_size / sizeof(int)));
		return *reinterpret_cast<T*>(internal_buf);
	}
	~TStackValue();

	TStackValue(const TStackValue&) = delete; //запретить копирование
	void operator=(const TStackValue&) = delete; //запретить присваивание
};

class TStaticValue : public TStackValue
{
	bool is_initialized;
public:
	TStaticValue();
	TStaticValue(bool is_ref, SemanticApi::ISClass* type);
	void Initialize();
	bool IsInitialized();
	void* get();
};
