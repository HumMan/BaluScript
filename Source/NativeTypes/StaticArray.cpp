#include "StaticArray.h"
#include "../VirtualMachine/ArrayClassMethod.h"

		int* TStaticArr::Get(int i)
		{
			assert(i<COUNT);
			return &data[i*methods->el_size];
		}
		int& TStaticArr::operator[](int id)
		{
			assert(id<COUNT);
			return *(int*)&data[id*_INTSIZEOF(int)/4];
		}
		void TStaticArr::DefConstr(TVirtualMachine* machine,int use_methods_id)
		{
			methods = &machine->GetArrayClassMethod(use_methods_id);
			int el_size=methods->el_size;
			int high=methods->el_count-1;
			int el_def_constr=methods->el_def_constr;
			if(el_def_constr!=-1)
				machine->ArrayElementsDefConstr(&data[0],el_size,high+1,el_def_constr);
		}
		void TStaticArr::RCopyConstr(TVirtualMachine* machine, int* &sp)
		{
			TStaticArr* copy_from=(TStaticArr*)*sp;
			methods=copy_from->methods;
			int el_size=methods->el_size;
			int high=methods->el_count-1;

			int el_copy_constr=methods->el_copy_constr;
			if(el_copy_constr!=-1)
				machine->ArrayElementsRCopyConstr(&data[0],&copy_from->data[0],el_size,high+1,el_copy_constr);
			else if(high>-1)memcpy(&data[0],&copy_from->data[0],(high+1)*el_size*4);
		}
		void TStaticArr::Destr(TVirtualMachine* machine)
		{
			int el_size=methods->el_size;
			int high=methods->el_count-1;
			int el_destr=methods->el_destr;
			if(el_destr!=-1)
				machine->ArrayElementsDestr(&data[0],el_size,high+1,el_destr);
		}
		void TStaticArr::AssignOp(TVirtualMachine* machine, TStaticArr* right)
		{
			int el_size=methods->el_size;
			int high=methods->el_count-1;
			int el_assign_op=methods->el_assign_op[true];
			if(el_assign_op!=-1)
				machine->ArrayElementsRRAssignOp(&data[0],&right->data[0],el_size,high+1,el_assign_op);
			else if(high>-1) memcpy(&data[0],&right->data[0],(high+1)*el_size*4);
		}
		bool TStaticArr::EqualOp(TVirtualMachine* machine, TStaticArr* right)
		{
			bool result=true;
			int el_size=methods->el_size;
			int high=methods->el_count-1;
			int el_equal_op=methods->el_equal_op[true][true];//TODO выбор оптимального оператора из доступных
			if(el_equal_op!=-1)
				result=machine->ArrayElementsRREqualOp(&data[0],&right->data[0],el_size,high+1,el_equal_op);
			else if(high>-1) result=(memcmp(&data[0],&right->data[0],(high+1)*el_size*4)==0);
			else result=true;
			return result;
		}