﻿#include "SBytecode.h"

#include "FormalParam.h"
#include "SMethod.h"
#include "SClass.h"

#include "../virtualMachine.h"

#include "../Syntax/Statements.h"
#include "../Syntax/Bytecode.h"
#include "../Syntax/Method.h"

TSBytecode::TSBytecode(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TBytecode* use_syntax)
	:TSStatement(TStatementType::Bytecode,use_owner,use_method,use_parent,(TStatement*)use_syntax)
{

}

void TSBytecode::Build(TGlobalBuildContext build_context)
{
	auto code = GetSyntax()->GetBytecode();
	for (size_t i = 0; i<code.size(); i++)
	{
		if (code[i].f[0] == TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID)
		{
			TSClass* temp = GetOwner()->GetClass(code[i].id[0]);
			if (temp == NULL)GetSyntax()->Error("Неизвестный идентификатор!");
			//GetSyntax()->code[i].op.v1 = program.CreateArrayElementClassId(temp);
			array_element_classes.push_back(temp);
		}//TODO сделать нормально без повторений
		if (code[i].f[1] == TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID)
		{
			TSClass* temp = GetOwner()->GetClass(code[i].id[1]);
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

void TSBytecode::Run(TStatementRunContext run_context)
{
	int stack[255];
	int* sp = stack;
	PackToStack(*run_context.formal_params, sp);

	auto object = (run_context.object!=nullptr)?(int*)run_context.object->get():nullptr;
	auto code = GetSyntax()->GetBytecode();
	for (const TBytecodeOp& op : code)
	{
		
		if (
			TVirtualMachine::ExecuteIntOps(op.op, sp, object) ||
			TVirtualMachine::ExecuteFloatOps(op.op, sp, object) ||
			TVirtualMachine::ExecuteBoolOps(op.op, sp, object) ||
			TVirtualMachine::ExecuteBaseOps(op.op, sp, object) ||
			TVirtualMachine::ExecuteVec2Ops(op.op, sp, object) ||
			TVirtualMachine::ExecuteVec2iOps(op.op, sp, object))
		{

		}
		else
		{
			throw;//GetSyntax()->Error("Неизвестная команда!");
		}
	}
	if (GetMethod()->GetRetClass() != NULL)
	{
		*run_context.result = TStackValue(GetMethod()->GetSyntax()->IsReturnRef(), GetMethod()->GetRetClass());
		if (GetMethod()->GetSyntax()->IsReturnRef())
		{
			run_context.result->SetAsReference(*(void**)stack);
		}
		else
		{
			memcpy(run_context.result->get(), stack, GetMethod()->GetReturnSize()*sizeof(int));
		}
	}
}
