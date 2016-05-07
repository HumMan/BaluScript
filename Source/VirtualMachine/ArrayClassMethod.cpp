#include "ArrayClassMethod.h"

#include <stdio.h>
#include <string.h>

bool TArrayClassMethod::operator == (TArrayClassMethod& right)
{
	return memcmp(this, &right, sizeof(TArrayClassMethod)) == 0;
}
