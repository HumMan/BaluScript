#include "../Syntax/Bytecode.h"

#include "../Syntax/Void.h"
#include "../Syntax/Class.h"
#include "../Syntax/Method.h"
#include "../Syntax/Statements.h"

TFormalParam TBytecode::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	TOpArray temp;
	for(int i=0;i<code.size();i++)
	{
		if(code[i].f[0]==TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID)
		{
			TClass* temp=owner->GetClass(code[i].id[0]);
			if(temp==NULL)Error("Неизвестный идентификатор!");
			code[i].op.v1=program.CreateArrayElementClassId(temp);
		}//TODO сделать нормально без повторений
		if(code[i].f[1]==TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID)
		{
			TClass* temp=owner->GetClass(code[i].id[1]);
			if(temp==NULL)Error("Неизвестный идентификатор!");
			code[i].op.v2=program.CreateArrayElementClassId(temp);
		}//TODO сделать нормально без повторений
		program.Push(code[i].op,temp);
	}
	method->SetHasReturn(true);//TODO нельзя определить возвращает ли код значение поэтому считаем что да(определить можно по записям в стек)
	return TVoid(temp);
}