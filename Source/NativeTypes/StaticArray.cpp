#include "StaticArray.h"

#include "../TreeRunner/FormalParam.h"

#include "../syntaxAnalyzer.h"

void* TStaticArr::operator_GetArrayElement(TMethodRunContext* run_context, TStaticArr* object, int index)
{
	SemanticApi::ISClass* obj_class = (*run_context->formal_params)[0].GetClass();
	SemanticApi::ISClass* el = obj_class->GetField(0)->GetClass();
	int size = obj_class->GetField(0)->GetSizeMultiplier();
	if (index < 0 || index >= size)
		throw std::string("Index out of range");
	return &object[el->GetSize()*index];
}

int TStaticArr::size(TMethodRunContext* run_context)
{
	SemanticApi::ISClass* obj_class = run_context->object->GetClass();
	int size = obj_class->GetField(0)->GetSizeMultiplier();
	return size;
}

SemanticApi::TExternalClassDecl TStaticArr::DeclareExternalClass()
{
	SemanticApi::TExternalClassDecl decl;

	decl.source =
		"class TStaticArray<T,Size>\n"
		"{\n"
		"	multifield(Size) T value;\n"
		"	operator extern static [](TStaticArray& v,int id):&T;\n"
		"	func extern size:int;\n"
		"}\n";

	//decl.operators[(int)Lexer::TOperator::GetArrayElement] = TStaticArr::get_element_op;
	//decl.methods.insert(std::make_pair(std::string("size"), TStaticArr::get_size));

	return decl;
}
