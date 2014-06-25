#pragma once

#include <string>

struct TString
{
	char* chars;
	int length;
	//bool is_const;
	//TODO is_const ���� ���������� ��� � ������ ������ chars ��������� �� ���������(��� ���������� ��������� ������ ��� ������ � ����������)
	void Init();
	void Copy(TString* copy_from);
	void CopyFromConst(std::string& copy_from);
	void Destr();
	void AssignOp(TString* right);
	bool EqualOp(TString* right);
};