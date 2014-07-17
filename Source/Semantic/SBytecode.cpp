#include "SBytecode.h"
#include "../virtualMachine.h"
#include "../Syntax/Bytecode.h"
#include "FormalParam.h"
#include "SMethod.h"
#include "../Syntax/Statements.h"

TSBytecode::TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TBytecode* use_syntax)
	:TSStatement(TStatementType::Bytecode,use_owner,use_method,use_parent,(TStatement*)use_syntax)
{

}

void TSBytecode::Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	for (int i = 0; i<GetSyntax()->code.size(); i++)
	{
		if (GetSyntax()->code[i].f[0] == TBytecode::TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID)
		{
			TSClass* temp = owner->GetClass(GetSyntax()->code[i].id[0]);
			if (temp == NULL)GetSyntax()->Error("Неизвестный идентификатор!");
			//GetSyntax()->code[i].op.v1 = program.CreateArrayElementClassId(temp);
			array_element_classes.push_back(temp);
		}//TODO сделать нормально без повторений
		if (GetSyntax()->code[i].f[1] == TBytecode::TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID)
		{
			TSClass* temp = owner->GetClass(GetSyntax()->code[i].id[1]);
			if (temp == NULL)GetSyntax()->Error("Неизвестный идентификатор!");
			//code[i].op.v2 = program.CreateArrayElementClassId(temp);
			array_element_classes.push_back(temp);
		}//TODO сделать нормально без повторений
	}
}

void PackToStack(std::vector<TStackValue> &formal_params, int* &sp)
{
	int i = 0;
	for (TStackValue& v : formal_params)
	{
		if (v.IsRef())
		{
			*(void**)sp = v.get();
			(*(int**)&sp) ++;
		}
		else
		{
			memcpy(sp, v.get(), v.GetClass()->GetSize()*sizeof(int));
			*(int**)&sp += v.GetClass()->GetSize();
		}
		if (i == (formal_params.size() - 1))
			(*(int**)&sp)--;
		i++;
	}
}

void TSBytecode::Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables)
{
	int stack[255];
	int* sp = stack;
	PackToStack(formal_params, sp);

	for (TBytecode::TBytecodeOp& op : GetSyntax()->code)
	{
		
		if (
			TVirtualMachine::ExecuteIntOps(&op.op, sp, (int*)object.get()) ||
			TVirtualMachine::ExecuteFloatOps(&op.op, sp, (int*)object.get()) ||
			TVirtualMachine::ExecuteBoolOps(&op.op, sp, (int*)object.get()) ||
			TVirtualMachine::ExecuteBaseOps(&op.op, sp, (int*)object.get()))
		{

		}
		else
		{
			throw;//GetSyntax()->Error("Неизвестная команда!");
		}
	}
	if (method->GetRetClass() != NULL)
	{
		result = TStackValue(method->GetSyntax()->IsReturnRef(), method->GetRetClass());
		if (method->GetSyntax()->IsReturnRef())
		{
			result.SetAsReference(*(void**)stack);
		}
		else
		{
			memcpy(result.get(), stack, method->GetReturnSize()*sizeof(int));
		}
	}
}
