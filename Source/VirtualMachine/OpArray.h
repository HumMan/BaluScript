#pragma once

#include "Op.h"

template<class T>
class TListItem
{
public:
	T value;
	TListItem<T> *prev;
	TListItem<T> *next;
	TListItem<T>() : prev(nullptr), next(nullptr){}
};

class TOpArray
{
public:
	TListItem<TOp>* first;
	TListItem<TOp>* last;
	TOpArray();
	bool IsNull()const;
	TOpArray operator+(const TOpArray& use_right)const;
	void operator+=(const TOpArray& use_right);
};