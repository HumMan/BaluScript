#pragma once

#include <baluLib.h>
#include "../VirtualMachine/Program.h"
#include "../VirtualMachine.h"

class TArrayClassMethod;
struct TDynArr
{
	BaluLib::TVector<int> v;
	int el_size;
	TArrayClassMethod* methods;
	int* Get(int i);
	int GetHigh();
	void DefConstr(TVirtualMachine* machine, int use_methods_id);
	void CopyConstr(TVirtualMachine* machine, TDynArr* copy_from);
	void Destr(TVirtualMachine* machine);
	void AssignOp(TVirtualMachine* machine, TDynArr* right);
	bool EqualOp(TVirtualMachine* machine, TDynArr* right);
	void SetHigh(TVirtualMachine* machine, int new_high, bool call_constr_destr = true);
	int* IncHigh(TVirtualMachine* machine, int count = 1);
};