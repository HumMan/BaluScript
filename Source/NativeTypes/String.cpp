#include "String.h"
//
//void TString::Init(){
//	chars = nullptr;
//	length = 0;
//	//is_const=false;
//}
//void TString::Copy(TString* copy_from)
//{
//	chars = copy_from->length != 0 ? new char[copy_from->length + 1] : nullptr;
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
//	if (chars != nullptr)delete chars;
//}
//void TString::AssignOp(TString* right)
//{
//	if (length != right->length)
//	{
//		if (chars != nullptr)delete chars;
//		chars = right->length != 0 ? new char[right->length + 1] : nullptr;
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

#include "../TreeRunner/FormalParam.h"

#include "../syntaxAnalyzer.h"

template<>
void TString::constructor(TMethodRunContext* run_context)
{
	run_context->object->get_as<TString>().Init();
}
template<>
void TString::destructor(TMethodRunContext* run_context)
{
	//TODO не должен вызываться конструктор для ссылочного типа
	//if (object.IsRef())
	//	return;

	TString* obj = ((TString*)run_context->object->get());
	obj->~TString();
}
template<>
void TString::copy_constr(TMethodRunContext* run_context)
{
	TString* obj = ((TString*)run_context->object->get());
	TString* copy_from = (TString*)(*run_context->formal_params)[0].get();
	obj->Init(*copy_from->v);
}
template<>
void TString::assign_op(TMethodRunContext* run_context)
{
	TString* left = ((TString*)(*run_context->formal_params)[0].get());
	TString* right = ((TString*)(*run_context->formal_params)[1].get());

	*(left->v) = *right->v;
}
template<>
void TString::assign_plus_op(TMethodRunContext* run_context)
{
	TString* left = ((TString*)(*run_context->formal_params)[0].get());
	TString* right = ((TString*)(*run_context->formal_params)[1].get());

	*(left->v) += *right->v;
}
template<>
void TString::plus_op(TMethodRunContext* run_context)
{
	TString* left = ((TString*)(*run_context->formal_params)[0].get());
	TString* right = ((TString*)(*run_context->formal_params)[1].get());

	auto temp = (*left->v + *right->v);

	run_context->result->get_as<TString>().Init(temp);
}
template<>
void TString::get_char_op(TMethodRunContext* run_context)
{
	TString* obj = ((TString*)(*run_context->formal_params)[0].get());
	int index = *((int*)(*run_context->formal_params)[1].get());
	run_context->result->SetAsReference(&(*(obj->v))[index]);
}
template<>
void TString::get_length(TMethodRunContext* run_context)
{
	TString* obj = ((TString*)run_context->object->get());
	*(int*)run_context->result->get() = obj->GetLength();
}
template<>
void TString::DeclareExternalClass(TSyntaxAnalyzer* syntax)
{
	syntax->GetLexer()->ParseSource(
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
		"}\n"
		);
	auto cl = SyntaxApi::AnalyzeNestedClass(syntax->GetLexer(), syntax->GetBaseClass());

	//TSClass* scl = new TSClass(syntax->GetCompiledBaseClass(), cl);
	//syntax->GetCompiledBaseClass()->AddClass(scl);
	//scl->Build();

	//scl->SetSize(LexerIntSizeOf(sizeof(TString)) / sizeof(int));
	//scl->SetAutoMethodsInitialized();

	//scl->GetDefConstr()->SetAsExternal(TString::constructor);

	//std::vector<TSMethod*> m;
	//m.clear();
	//scl->GetCopyConstructors(m);
	//m[0]->SetAsExternal(TString::copy_constr);

	//scl->GetDestructor()->SetAsExternal(TString::destructor);

	//m.clear();
	//scl->GetOperators(m, Lexer::TOperator::GetArrayElement);
	//m[0]->SetAsExternal(TString::get_char_op);

	//m.clear();
	//scl->GetOperators(m, Lexer::TOperator::Assign);
	////TODO ввести использование ссылки на временный объект
	//m[0]->SetAsExternal(TString::assign_op);

	//m.clear();
	//scl->GetOperators(m, Lexer::TOperator::PlusA);
	////TODO ввести использование ссылки на временный объект
	//m[0]->SetAsExternal(TString::assign_plus_op);

	//m.clear();
	//scl->GetOperators(m, Lexer::TOperator::Plus);
	////TODO ввести использование ссылки на временный объект
	//m[0]->SetAsExternal(TString::plus_op);

	//m.clear();
	//scl->GetMethods(m, syntax->GetLexer()->GetIdFromName("length"));
	//m[0]->SetAsExternal(TString::get_length);
}
