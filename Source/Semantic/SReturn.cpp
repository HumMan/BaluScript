#include "../Syntax/Return.h"

#include "../Syntax/Void.h"
#include "../Syntax/Method.h"
#include "../syntaxAnalyzer.h"
#include "../Syntax/Statements.h"

TFormalParam TReturn::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	TFormalParam result_result=result.Build(program,local_var_offset);
	if(method->GetRetClass()!=NULL)
	{
		int conv_needed;
		if(result_result.IsVoid())
			Error("После оператора return должно следовать какое-то выражение!");
		if(!IsEqualClasses(result_result,method->GetRetClass(),method->IsReturnRef(),conv_needed))
			Error("Выражение невозможно преобразовать в тип возвращаемого значения!");
		method->BuildFormalParamConversion(program,result_result,method->GetRetClass(),method->IsReturnRef());	
	}else if(result_result.GetClass()!=NULL) Error("Метод не должен ничего возвращать!");
	int locals_size=0;
	result_result.GetOps()+=BuildLocalsAndParamsDestructor(program,locals_size);
	if(method->GetMemberType()==TClassMember::Destr)
	{
		TMethod* auto_destr=owner->GetAutoDestructor();
		if(auto_destr!=NULL)
		{
			program.Push(TOp(TOpcode::PUSH_THIS),result_result.GetOps());
			result_result.GetOps()+=auto_destr->BuildCall(program).GetOps();
		}
	}
	program.Push(TOp(TOpcode::RETURN,
		method->GetParamsSize()+locals_size+!method->IsStatic(),method->GetRetSize()),result_result.GetOps());
	method->SetHasReturn(true);
	return TVoid(result_result.GetOps());
}