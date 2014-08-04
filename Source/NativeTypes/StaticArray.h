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