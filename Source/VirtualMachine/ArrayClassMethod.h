#pragma once

#include <memory>

struct TArrayClassMethod
{
	int el_size, el_count;
	//идентификаторы методов и операторов используемых при работе массивов
	int el_def_constr;
	int el_copy_constr;
	int el_destr;
	int el_assign_op[2];//[right_ref]
	int el_equal_op[2][2]; //[left_ref][right_ref]
	bool operator==(TArrayClassMethod& right)
	{
		return memcmp(this, &right, sizeof(TArrayClassMethod)) == 0;
	}
};