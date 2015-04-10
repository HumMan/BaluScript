#include "String.h"
//
//void TString::Init(){
//	chars = NULL;
//	length = 0;
//	//is_const=false;
//}
//void TString::Copy(TString* copy_from)
//{
//	chars = copy_from->length != 0 ? new char[copy_from->length + 1] : NULL;
//	length = copy_from->length;
//	if (length > 0)strncpy_s(chars, copy_from->length + 1, copy_from->chars, copy_from->length + 1);
//}
//void TString::CopyFromConst(std::string& copy_from)
//{
//	//is_const=true;
//	length = copy_from.length() + 1;
//	chars = new char[length];
//	strncpy_s(chars, length, copy_from.c_str(), length);
//}
//void TString::Destr()
//{
//	if (chars != NULL)delete chars;
//}
//void TString::AssignOp(TString* right)
//{
//	if (length != right->length)
//	{
//		if (chars != NULL)delete chars;
//		chars = right->length != 0 ? new char[right->length + 1] : NULL;
//		length = right->length;
//	}
//	if (length > 0)
//		strncpy_s(chars, right->length + 1, right->chars, right->length + 1);
//}
//bool TString::EqualOp(TString* right)
//{
//	return strcmp(chars, right->chars) == 0;
//}


#include "String.h"
#include "DynArray.h"

#include "Semantic/FormalParam.h"
#include "Semantic/SClass.h"

#include "syntaxAnalyzer.h"

#include "Syntax/Statements.h"
#include "Syntax/Method.h"

#pragma push_macro("new")
#undef new

void TString::constructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	memset((TString*)object.get(), 0xfeefee, sizeof(TString));
	TString* obj = new ((TString*)object.get()) TString();
}

void TString::destructor(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	//TODO не должен вызываться конструктор для ссылочного типа
	if (object.IsRef())
		return;

	TString* obj = ((TString*)object.get());
	obj->~TString();
}

void TString::copy_constr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TString* obj = ((TString*)object.get());
	TString* copy_from = (TString*)formal_params[0].get();
	new (obj)TString(*copy_from);
}

void TString::assign_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TString* left = ((TString*)formal_params[0].get());
	TString* right = ((TString*)formal_params[1].get());

	*(left->v) = *right->v;
}

void TString::assign_plus_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TString* left = ((TString*)formal_params[0].get());
	TString* right = ((TString*)formal_params[1].get());

	*(left->v) += *right->v;
}

void TString::plus_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TString* left = ((TString*)formal_params[0].get());
	TString* right = ((TString*)formal_params[1].get());

	auto temp = (*left->v + *right->v);

	result.get_as<TString>().InitBy(temp);
}

void TString::get_char_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TString* obj = ((TString*)formal_params[0].get());
	int index = *((int*)formal_params[1].get());
	result.SetAsReference(&(*(obj->v))[index]);
}

void TString::get_length(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TString* obj = ((TString*)object.get());
	*(int*)result.get() = obj->GetLength();
}

#pragma pop_macro("new")

void TString::DeclareExternalClass(TSyntaxAnalyzer* syntax)
{
	TClass* cl = new TClass(syntax->base_class.get());
	syntax->base_class->AddNested(cl);
	syntax->lexer.ParseSource(
		"class extern string\n"
		"{\n"
		"default();\n"
		"copy(string& copy_from);\n"
		"copy(string copy_from);\n"
		"destr();\n"
		"operator static [](string& v,int id):&char;\n"
		"operator static =(string& v,string& l);\n"
		"operator static =(string& v,string l);\n"
		"operator static +=(string& v,string& l);\n"
		"operator static +=(string& v,string l);\n"
		"operator static +(string& v,string& l):string;\n"
		"func length:int;\n"
		"}\n"
		);
	cl->AnalyzeSyntax(syntax->lexer);
	syntax->lexer.GetToken(TTokenType::Done);

	TSClass* scl = new TSClass(syntax->sem_base_class.get(), cl);
	syntax->sem_base_class->AddClass(scl);
	scl->Build();

	scl->SetSize(IntSizeOf(sizeof(TString)) / sizeof(int));
	scl->SetAutoMethodsInitialized();

	scl->GetDefConstr()->SetAsExternal(TString::constructor);

	std::vector<TSMethod*> m;
	m.clear();
	scl->GetCopyConstructors(m);
	m[0]->SetAsExternal(TString::copy_constr);
	m[1]->SetAsExternal(TString::copy_constr);

	scl->GetDestructor()->SetAsExternal(TString::destructor);

	m.clear();
	scl->GetOperators(m, TOperator::GetArrayElement);
	m[0]->SetAsExternal(TString::get_char_op);

	m.clear();
	scl->GetOperators(m, TOperator::Assign);
	//TODO ввести использование ссылки на временный объект
	m[0]->SetAsExternal(TString::assign_op);
	m[1]->SetAsExternal(TString::assign_op);

	m.clear();
	scl->GetOperators(m, TOperator::PlusA);
	//TODO ввести использование ссылки на временный объект
	m[0]->SetAsExternal(TString::assign_plus_op);
	m[1]->SetAsExternal(TString::assign_plus_op);

	m.clear();
	scl->GetOperators(m, TOperator::Plus);
	//TODO ввести использование ссылки на временный объект
	m[0]->SetAsExternal(TString::plus_op);

	m.clear();
	scl->GetMethods(m, syntax->lexer.GetIdFromName("length"));
	m[0]->SetAsExternal(TString::get_length);
}