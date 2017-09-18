#include <stdlib.h>

#include "OpArray.h"

TOpArray::TOpArray() :first(nullptr), last(nullptr){}
bool TOpArray::IsNull()const
{
	assert((first == nullptr) == (last == nullptr));
	return first == nullptr;
}
TOpArray TOpArray::operator+(const TOpArray& use_right)const
{
	if (first == nullptr)return use_right;
	if (use_right.first == nullptr)return *this;

	last->next = use_right.first;
	use_right.first->prev = last;
	TOpArray result;
	result.first = first;
	result.last = use_right.last;
	return result;
}
void TOpArray::operator+=(const TOpArray& use_right)
{
	assert((first == nullptr) == (last == nullptr));
	*this = *this + use_right;
}