#pragma once

#include <baluLib.h>
#include "Op.h"

struct TOpArray
{
	TListItem<TOp>* first;
	TListItem<TOp>* last;
	TOpArray();
	bool IsNull()const;
	TOpArray operator+(const TOpArray& use_right)const;
	void operator+=(const TOpArray& use_right);
};