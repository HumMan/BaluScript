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

void TString::copy_constr(TString* param0)
{
	Init(*(param0->v));
}

TString TString::operator_Assign(TString* left, TString* right)
{
	*(left->v) = *right->v;
	return *left;
}

TString TString::operator_PlusA(TString* left, TString* right)
{
	*(left->v) += *right->v;
	return *left;
}

TString TString::operator_Plus(TString* left, TString* right)
{
	auto temp = (*left->v + *right->v);
	TString result;
	result.Init(temp);
	return result;
}

char& TString::operator_GetArrayElement(TString* obj, int index)
{
	return (*(obj->v))[index];
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

	//decl.def_constr = TString::constructor;
	//decl.copy_constr = TString::copy_constr;
	//decl.destr = TString::destructor;

	//decl.operators[(int)Lexer::TOperator::GetArrayElement] = TString::get_char_op;
	//decl.operators[(int)Lexer::TOperator::Assign] = TString::assign_op;

	//decl.operators[(int)Lexer::TOperator::PlusA] = TString::assign_plus_op;
	//decl.operators[(int)Lexer::TOperator::Plus] = TString::plus_op;

	//decl.methods.insert(std::make_pair(std::string("length"), TString::get_length));

	return decl;
}
