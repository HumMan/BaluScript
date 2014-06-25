#pragma once

#include <baluLib.h>
#include <memory>
#include <string>
#include "Op.h"
#include "ArrayClassMethod.h"

typedef void(*TExternalMethod)(int*&, int*, int*);// параметры: вершина стека, указатель на первый параметр, указатель на объект

class TProgram
{
public:
	struct TMethod
	{
		bool is_static;
		bool is_external;
		int first_op;
		TExternalMethod extern_method;
		int pre_event;
		int post_event;
		int params_size;
		int return_size;//используетс€ дл€ внешних методов, дл€ USER методов все есть в RETURN
		TMethod():first_op(-1),extern_method(NULL){}
	};
	TVector<TOp> instructions;//TODO в дальнейшем в TVector<int>
	
	TVector<TArrayClassMethod> array_class_methods;
	TVector<std::string> string_consts;
	TVector<TMethod> methods_table;
	int static_vars_init;
	int static_vars_destroy;
	int static_vars_size;
	void SetExternalMethod(int method_id,TExternalMethod external_func)
	{
		if(!methods_table[method_id].is_external)
			throw "ƒанный метод не €вл€етс€ внешним!";
		methods_table[method_id].extern_method=external_func;
	}
};