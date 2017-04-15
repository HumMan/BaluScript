#include "StaticArray.h"

#include "../Semantic/FormalParam.h"
#include "../Semantic/SClass.h"

#include "../syntaxAnalyzer.h"

#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"

#include "../Syntax/Class.h"

void TStaticArr::get_element_op(TMethodRunContext run_context)
{
	TSClass* obj_class = (*run_context.formal_params)[0].GetClass();
	TSClass* el = obj_class->GetField(0)->GetClass();
	int size = obj_class->GetField(0)->GetSizeMultiplier();
	int index = (*(int*)(*run_context.formal_params)[1].get());
	if (index < 0 || index >= size)
		throw std::string("Index out of range");
	run_context.result->SetAsReference(&(((int*)((*run_context.formal_params)[0].get()))[el->GetSize()*index]));
}

void TStaticArr::get_size(TMethodRunContext run_context)
{
	TSClass* obj_class = run_context.object->GetClass();
	int size = obj_class->GetField(0)->GetSizeMultiplier();
	*(int*)run_context.result->get() = size;
}

void TStaticArr::DeclareExternalClass(TSyntaxAnalyzer* syntax)
{
	TClass* cl = new TClass(syntax->GetBaseClass());
	syntax->GetBaseClass()->AddNested(cl);
	syntax->GetLexer()->ParseSource(
		"class TStaticArray<T,Size>\n"
		"{\n"
		"	multifield(Size) T value;\n"
		"	operator extern static [](TStaticArray& v,int id):&T;\n"
		"	func extern size:int;\n"
		"}\n"
		);
	cl->AnalyzeSyntax(syntax->GetLexer());
	syntax->GetLexer()->GetToken(Lexer::TTokenType::Done);

	TSClass* scl = new TSClass(syntax->GetCompiledBaseClass(), cl);
	syntax->GetCompiledBaseClass()->AddClass(scl);
	scl->Build();
	scl->SetAutoMethodsInitialized();

	std::vector<TSMethod*> m;

	m.clear();
	scl->GetOperators(m, Lexer::TOperator::GetArrayElement);
	m[0]->SetAsExternal(TStaticArr::get_element_op);

	m.clear();
	scl->GetMethods(m, syntax->GetLexer()->GetIdFromName("size"));
	m[0]->SetAsExternal(TStaticArr::get_size);
}
