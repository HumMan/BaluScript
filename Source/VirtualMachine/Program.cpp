#include "Program.h"

void TProgram::SetExternalMethod(int method_id, TExternalMethod external_func)
{
	if (!methods_table[method_id].is_external)
		throw "Данный метод не является внешним!";
	methods_table[method_id].extern_method = external_func;
}