#include "../Syntax/Statements.h"

#include <assert.h>

#include "../Syntax/Void.h"
#include "../Syntax/Class.h"
#include "../Syntax/Method.h"
#include "../Syntax/LocalVar.h"

TStatements::TStatements(const TStatements& use_source):TStatement(use_source)
{
	curr_local_var_offset=use_source.curr_local_var_offset;
	last_locals_offset=use_source.last_locals_offset;
	statement.SetHigh(use_source.statement.GetHigh());
	for(int i=0;i<=statement.GetHigh();i++)
		statement[i]=use_source.statement[i]->GetCopy();
	var_declarations.SetHigh(use_source.var_declarations.GetHigh());
	for(int i=0;i<=var_declarations.GetHigh();i++)
	{
		var_declarations[i].stmt_id=use_source.var_declarations[i].stmt_id;
		var_declarations[i].pointer=(TLocalVar*)statement[var_declarations[i].stmt_id];
		assert(statement[var_declarations[i].stmt_id]->GetType()==TStatementType::VarDecl);
	}
}

TFormalParam TStatements::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	TOpArray ops_array;
	curr_local_var_offset=local_var_offset;
	last_locals_offset=local_var_offset;
	for(int i=0;i<=statement.GetHigh();i++)
	{
		TFormalParam t=statement[i]->Build(program,curr_local_var_offset);
		ops_array+=t.GetOps();
		if(!t.IsVoid())
			statement[i]->Error("Выражение не должно возвращать значение!");
	}
	BuildCurrLocalsDestructor(ops_array,program,true);
	return TVoid(ops_array);
}


TOpArray TStatements::BuildLocalsAndParamsDestructor(TNotOptimizedProgram &program,int &deallocate_size)
{
	TOpArray ops_array;
	deallocate_size+=BuildCurrLocalsDestructor(ops_array,program,false);
	if(parent!=NULL)ops_array+=parent->BuildLocalsAndParamsDestructor(program,deallocate_size);
	else 
	{
		//if(deallocate_size>0)
		//	program.Push(TOp(TOpcode::POP_COUNT,deallocate_size),ops_array);
		ops_array+=method->BuildParametersDestructor(program);
	}
	return ops_array;
}

int TStatements::BuildCurrLocalsDestructor(TOpArray& ops_array,TNotOptimizedProgram &program,bool deallocate_stack)
{
	for(int i=0;i<=statement.GetHigh();i++)
	{
		if(statement[i]->GetType()==TStatementType::VarDecl)
		{
			TLocalVar* v=((TLocalVar*)(statement[i]));
			if(v->IsStatic()||v->GetClass()->GetDestructor()==NULL)continue;
			program.Push(TOp(TOpcode::PUSH_LOCAL_REF,v->GetOffset()),ops_array);
			ops_array+=v->GetClass()->GetDestructor()->BuildCall(program).GetOps();
		}
	}
	int dealloc_size=curr_local_var_offset-last_locals_offset;
	if(deallocate_stack&&dealloc_size>0)
		program.Push(TOp(TOpcode::POP_COUNT,dealloc_size),ops_array);
	return dealloc_size;
}



TVariable* TStatements::GetVar(TNameId name,int sender_id)
{
	for(int i=0;i<=var_declarations.GetHigh();i++)
	{
		if(var_declarations[i].stmt_id<=sender_id
			&&var_declarations[i].pointer->GetName()==name)
			return var_declarations[i].pointer;
	}
	//for(int i=0;i<=statement.GetHigh();i++)
	//	//TODO из-за этого перебора тормоза при большом количестве операторных скобок + невозможность искать локальную переменную за не линейную скорость
	//{
	//	if(except_this&&statement[i]==sender)break;
	//	if(statement[i]->GetType()==TStatementType::VarDecl&&((TLocalVar*)(statement[i]))->GetName()==name)
	//		return ((TLocalVar*)(statement[i]));
	//	if(statement[i]==sender)break;
	//}
	if(parent!=NULL)return parent->GetVar(name,stmt_id);
	else if(method!=NULL)return  method->GetVar(name);
	else return NULL;
}