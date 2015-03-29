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

struct TString
{
	std::string* v;
	TString()
	{
		v = new std::string();
	}
	TString(const TString& r)
	{
		v = new std::string(*r.v);
	}
	char GetChar(int i)
	{
		return(*v)[i];
	}
	int GetLength()
	{
		return v->size();
	}
	~TString()
	{
		delete v;
		v = NULL;
	}
	static void constructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void destructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void copy_constr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void assign_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void get_char_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void get_length(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);

	static void DeclareExternalClass(TSyntaxAnalyzer* syntax);
};