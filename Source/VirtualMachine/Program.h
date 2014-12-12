#pragma once

#include <baluLib.h>
#include <memory>
#include <string>
#include "Op.h"
#include "ArrayClassMethod.h"

#include <vector>

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
		int return_size;//используется для внешних методов, для USER методов все есть в RETURN
		TMethod():first_op(-1),extern_method(NULL){}
	};
	std::vector<TOp> instructions;//TODO в дальнейшем в TVector<int>
	
	std::vector<TArrayClassMethod> array_class_methods;
	std::vector<std::string> string_consts;
	std::vector<TMethod> methods_table;
	int static_vars_init;
	int static_vars_destroy;
	int static_vars_size;
	void SetExternalMethod(int method_id, TExternalMethod external_func);
};