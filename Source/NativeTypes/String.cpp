#include "String.h"

void TString::Init(){
	chars = NULL;
	length = 0;
	//is_const=false;
}
void TString::Copy(TString* copy_from)
{
	chars = copy_from->length != 0 ? new char[copy_from->length + 1] : NULL;
	length = copy_from->length;
	if (length > 0)strncpy_s(chars, copy_from->length + 1, copy_from->chars, copy_from->length + 1);
}
void TString::CopyFromConst(std::string& copy_from)
{
	//is_const=true;
	length = copy_from.length() + 1;
	chars = new char[length];
	strncpy_s(chars, length, copy_from.c_str(), length);
}
void TString::Destr()
{
	if (chars != NULL)delete chars;
}
void TString::AssignOp(TString* right)
{
	if (length != right->length)
	{
		if (chars != NULL)delete chars;
		chars = right->length != 0 ? new char[right->length + 1] : NULL;
		length = right->length;
	}
	if (length > 0)
		strncpy_s(chars, right->length + 1, right->chars, right->length + 1);
}
bool TString::EqualOp(TString* right)
{
	return strcmp(chars, right->chars) == 0;
}