#include "String.h"
#include "DynArray.h"

#include "../TreeRunner/FormalParam.h"

#include "../syntaxAnalyzer.h"

void TString::def_constr()
{
	Init();
}

void TString::destructor()
{
	//TODO не должен вызываться конструктор для ссылочного типа
	//if (object.IsRef())
	//	return;

	delete v;
	v = nullptr;
}

void TString::copy_constr(TString& param0)
{
	Init(*(param0.v));
}

void TString::operator_Assign(TString& left, TString& right)
{
	*(left.v) = *right.v;
}

void TString::operator_PlusA(TString& left, TString& right)
{
	*(left.v) += *right.v;
}

TString TString::operator_Plus(TString& left, TString& right)
{
	auto temp = (*left.v + *right.v);
	TString result;
	result.Init(temp);
	return result;
}

char& TString::operator_GetArrayElement(TString& obj, int index)
{
	return (*(obj.v))[index];
}

int TString::length()
{
	return GetLength();
}

SemanticApi::TExternalClassDecl TString::DeclareExternalClass()
{
	SemanticApi::TExternalClassDecl decl;
	decl.size = LexerIntSizeOf(sizeof(TString)) / sizeof(int);
	decl.source =
		"class extern string\n"
		"{\n"
		"default();\n"
		"copy(string& copy_from);\n"
		"destr();\n"
		"operator static [](string& v,int id):&char;\n"
		"operator static =(string& v,string& l);\n"
		"operator static +=(string& v,string& l);\n"
		"operator static +(string& v,string& l):string;\n"
		"func length:int;\n"
		"}\n";
	return decl;
}
