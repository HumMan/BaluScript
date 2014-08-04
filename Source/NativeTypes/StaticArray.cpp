#include "StaticArray.h"

#include "Semantic/FormalParam.h"
#include "Semantic/SClass.h"

#include "syntaxAnalyzer.h"

#include "Syntax/Statements.h"
#include "Syntax/Method.h"

void TStaticArr::get_element_op(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TSClass* obj_class = formal_params[0].GetClass();
	TSClass* el = obj_class->fields.begin()->GetClass();
	int size = obj_class->fields.begin()->GetSizeMultiplier();
	int index = (*(int*)formal_params[1].get());
	if (index < 0 || index >= size)
		throw std::string("Index out of range");
	result.SetAsReference(&(((int*)(formal_params[0].get()))[el->GetSize()*index]));
}

void TStaticArr::get_size(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object)
{
	TSClass* obj_class = object.GetClass();
	int size = obj_class->fields.begin()->GetSizeMultiplier();
	*(int*)result.get() = size;
}

#pragma pop_macro("new")

void TStaticArr::DeclareExternalClass(TSyntaxAnalyzer* syntax)
{
	TClass* cl = new TClass(syntax->base_class);
	syntax->lexer.ParseSource(
		"class TStaticArray<T,Size>\n"
		"{\n"
		"	multifield(Size) T value;\n"
		"	operator extern static [](TStaticArray& v,int id):&T;\n"
		"	func extern size:int;\n"
		"}\n"
		);
	cl->AnalyzeSyntax(syntax->lexer);
	syntax->lexer.GetToken(TTokenType::Done);

	TSClass* scl = new TSClass(syntax->sem_base_class, cl);
	syntax->sem_base_class->AddClass(scl);
	scl->Build();
	scl->SetAutoMethodsInitialized();

	std::vector<TSMethod*> m;

	m.clear();
	scl->GetOperators(m, TOperator::GetArrayElement);
	m[0]->SetAsExternal(TStaticArr::get_element_op);

	m.clear();
	scl->GetMethods(m, syntax->lexer.GetIdFromName("size"));
	m[0]->SetAsExternal(TStaticArr::get_size);
}