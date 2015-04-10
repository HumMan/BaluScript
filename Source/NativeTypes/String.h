#pragma once

#include <baluLib.h>

//struct TString
//{
	//char* chars;
	//int length;
	////bool is_const;
	////TODO is_const флаг означающий что в данный момент chars указывает на константу(для исключения выделения памяти при работе с литералами)
	//void Init();
	//void Copy(TString* copy_from);
	//void CopyFromConst(std::string& copy_from);
	//void Destr();
	//void AssignOp(TString* right);
	//bool EqualOp(TString* right);
//};

class TStaticValue;
class TStackValue;
class TSyntaxAnalyzer;
class TSClass;

template<class T, class pass_as = T>
struct TStringWrapper
{
	typedef T Arg;
	typedef T TypeForGetName;
	typedef pass_as PassInMethodAs;
	T* v;
	void InitBy(T value)
	{
		v = new T(value);
	}
	TStringWrapper()
	{
		v = new T();
	}
	TStringWrapper(const TStringWrapper& r)
	{
		v = new T(*r.v);
	}
	char GetChar(int i)
	{
		return(*v)[i];
	}
	int GetLength()
	{
		return v->size();
	}
	T& GetCppValue()
	{
		return *v;
	}
	~TStringWrapper()
	{
		delete v;
		v = NULL;
	}
	static void constructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void destructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void copy_constr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void assign_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void assign_plus_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void plus_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void get_char_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void get_length(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);

	static void DeclareExternalClass(TSyntaxAnalyzer* syntax);
};

typedef TStringWrapper<std::string> TString;