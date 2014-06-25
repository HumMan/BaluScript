#pragma once

#include "VirtualMachine/Program.h"
#include "VirtualMachine/OpArray.h"

class TMethod;
class TClass;
class TNameId;
class TMethodPrePostEvents;

class TNotOptimizedProgram
{
private:
	//TList<TOp> instructions;
	TAllocator<TListItem<TOp>, 1023> instr_alloc;
	int curr_label;

	std::vector<char*> string_consts;
#ifndef NDEBUG
	std::vector<TOp*> ops;
#endif
	std::vector<TMethod*> methods_table;
	std::vector<TClass*> array_element_classes;

	std::unique_ptr<TMethod> static_vars_init_method, static_vars_destroy_method;

	int CreateStaticVarsInitMethod();
	int CreateStaticVarsDestroyMethod();
	void InitArrayClassMethods(TProgram& optimized);
	void InitPrePostEvents(std::vector<TMethodPrePostEvents>& method_prepost_events, int methods_before_prepost_event);
public:

	TOpArray static_vars_init;
	TOpArray static_vars_destroy;

	int static_vars_size;

	std::vector<std::unique_ptr<TMethod>> internal_methods;

	TNotOptimizedProgram();
	~TNotOptimizedProgram();
	void ListItems();
	void CreateOptimizedProgram(TProgram& optimized, TTime& time);
	int AddStringConst(TNameId string);
	int GetUniqueLabel();
	void Push(TOp use_op, TOpArray &ops_array);
	void PushFront(TOp use_op, TOpArray &ops_array);
	int AddMethodToTable(TMethod* use_method);
	int CreateArrayElementClassId(TClass* use_class);
	int FindMethod(TMethod* use_method);


};