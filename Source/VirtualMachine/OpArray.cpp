#include "OpArray.h"

TOpArray::TOpArray() :first(NULL), last(NULL){}
bool TOpArray::IsNull()const
{
	assert((first == NULL) == (last == NULL));
	return first == NULL;
}
TOpArray TOpArray::operator+(const TOpArray& use_right)const
{
	if (first == NULL)return use_right;
	if (use_right.first == NULL)return *this;

	last->next = use_right.first;
	use_right.first->prev = last;
	TOpArray result;
	result.first = first;
	result.last = use_right.last;
	return result;
}
void TOpArray::operator+=(const TOpArray& use_right)
{
	assert((first == NULL) == (last == NULL));
	*this = *this + use_right;
}