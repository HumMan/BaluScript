#pragma once

#include <memory>


struct TArrayClassMethod
{
	///<summary>Размер элемента массива и их количество</summary>
	int el_size, el_count;
	//идентификаторы методов и операторов используемых при работе массивов
	int el_def_constr;
	int el_copy_constr;
	int el_destr;
	int el_assign_op[2];//[right_ref]
	int el_equal_op[2][2]; //[left_ref][right_ref]
	bool operator==(TArrayClassMethod& right);
};