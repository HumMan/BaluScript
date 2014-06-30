#include "ArrayClassMethod.h"

bool TArrayClassMethod::operator == (TArrayClassMethod& right)
{
	return memcmp(this, &right, sizeof(TArrayClassMethod)) == 0;
}