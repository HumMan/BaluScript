#pragma once

#include <baluLib.h>
#include "../VirtualMachine/Program.h"
#include "../VirtualMachine.h"


class TStaticValue;
class TStackValue;
class TSyntaxAnalyzer;
class TSClass;

struct TStaticArr
{
	static void get_element_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
	static void get_size(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);

	static void DeclareExternalClass(TSyntaxAnalyzer* syntax);
};

//struct TArrayClassMethod;
//struct TStaticArr
//	{
//	int COUNT;
//	public:
//		TStaticArr()
//		{
//			COUNT = 1;
//		}
//		TStaticArr(int COUNT)
//		{
//			this->COUNT = COUNT;
//		}
//		TArrayClassMethod* methods;
//		int data[1/*COUNT*/*_INTSIZEOF(int)/4];//размер не имеет значения т.к. TStaticArr никогда не создается
//		int* Get(int i);
//		int& operator[](int id);
//		void DefConstr(TVirtualMachine* machine,int use_methods_id);
//		void RCopyConstr(TVirtualMachine* machine, int* &sp);
//		void Destr(TVirtualMachine* machine);
//		void AssignOp(TVirtualMachine* machine, TStaticArr* right);
//		bool EqualOp(TVirtualMachine* machine, TStaticArr* right);
//	};