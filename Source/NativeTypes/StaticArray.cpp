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

	TSClass* scl = new TSClass(syntax->sem_base_class, cl);
	syntax->sem_base_class->AddClass(scl);
	scl->Build();

	std::vector<TSMethod*> m;

	m.clear();
	scl->GetOperators(m, TOperator::GetArrayElement);
	m[0]->SetAsExternal(TStaticArr::get_element_op);

	m.clear();
	scl->GetMethods(m, syntax->lexer.GetIdFromName("size"));
	m[0]->SetAsExternal(TStaticArr::get_size);
}





//
//int* TStaticArr::Get(int i)
//{
//	assert(i < COUNT);
//	return &data[i*methods->el_size];
//}
//int& TStaticArr::operator[](int id)
//{
//	assert(id < COUNT);
//	return *(int*)&data[id*_INTSIZEOF(int) / 4];
//}
//void TStaticArr::DefConstr(TVirtualMachine* machine, int use_methods_id)
//{
//	methods = &machine->GetArrayClassMethod(use_methods_id);
//	int el_size = methods->el_size;
//	int high = methods->el_count - 1;
//	int el_def_constr = methods->el_def_constr;
//	if (el_def_constr != -1)
//		machine->ArrayElementsDefConstr(&data[0], el_size, high + 1, el_def_constr);
//}
//void TStaticArr::RCopyConstr(TVirtualMachine* machine, int* &sp)
//{
//	TStaticArr* copy_from = (TStaticArr*)*sp;
//	methods = copy_from->methods;
//	int el_size = methods->el_size;
//	int high = methods->el_count - 1;
//
//	int el_copy_constr = methods->el_copy_constr;
//	if (el_copy_constr != -1)
//		machine->ArrayElementsRCopyConstr(&data[0], &copy_from->data[0], el_size, high + 1, el_copy_constr);
//	else if (high > -1)memcpy(&data[0], &copy_from->data[0], (high + 1)*el_size * 4);
//}
//void TStaticArr::Destr(TVirtualMachine* machine)
//{
//	int el_size = methods->el_size;
//	int high = methods->el_count - 1;
//	int el_destr = methods->el_destr;
//	if (el_destr != -1)
//		machine->ArrayElementsDestr(&data[0], el_size, high + 1, el_destr);
//}
//void TStaticArr::AssignOp(TVirtualMachine* machine, TStaticArr* right)
//{
//	int el_size = methods->el_size;
//	int high = methods->el_count - 1;
//	int el_assign_op = methods->el_assign_op[true];
//	if (el_assign_op != -1)
//		machine->ArrayElementsRRAssignOp(&data[0], &right->data[0], el_size, high + 1, el_assign_op);
//	else if (high > -1) memcpy(&data[0], &right->data[0], (high + 1)*el_size * 4);
//}
//bool TStaticArr::EqualOp(TVirtualMachine* machine, TStaticArr* right)
//{
//	bool result = true;
//	int el_size = methods->el_size;
//	int high = methods->el_count - 1;
//	int el_equal_op = methods->el_equal_op[true][true];//TODO выбор оптимального оператора из доступных
//	if (el_equal_op != -1)
//		result = machine->ArrayElementsRREqualOp(&data[0], &right->data[0], el_size, high + 1, el_equal_op);
//	else if (high > -1) result = (memcmp(&data[0], &right->data[0], (high + 1)*el_size * 4) == 0);
//	else result = true;
//	return result;
//}