#include "../Syntax/LocalVar.h"

#include "../Syntax/Class.h"
#include "../Syntax/Statements.h"
#include "../Syntax/Expression.h"
#include "../Syntax/Method.h"

TFormalParam TLocalVar::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	std::vector<TMethod*> methods;
	if(owner->GetMethods(methods,name))
		Error("Метод не может быть именем переменной!");
	if(owner->GetClass(name)!=NULL)
		Error("Класс не может быть именем переменной!");
	TVariable* t=parent->GetVar(name,stmt_id);
	if(t!=NULL&&t!=this)
	{
		switch(t->GetType())
		{
		case TVariableType::ClassField:
			Error("Локальная переменная перекрывает член класса!");
		case TVariableType::Parameter:
			Error("Локальная переменная перекрывает параметр!");
		case TVariableType::LocalVar:
			Error("Локальная переменная с таким именем уже существует!");
		default:assert(false);
		}
	}
	if(is_static)
	{
		offset=program.static_vars_size;
		program.static_vars_size+=type.GetClass(true,&program)->GetSize();
	}else
	{
		offset=local_var_offset;
		local_var_offset+=type.GetClass(true,&program)->GetSize();
	}

	std::vector<TFormalParam> params_result;
	TOpArray params_ops,before_params,after_params;
	for(int i=0;i<params.size();i++)
	{
		params_result.push_back(params[i]->Build(program,local_var_offset));
		params_ops+=params_result[i].GetOps();
	}
	int conv_need=0;
	std::vector<TMethod*> constructors;
	type.GetClass()->GetConstructors(constructors);
	TMethod* constructor=FindMethod(this,constructors,params_result,conv_need);
	if(constructor==NULL&&params_result.size()>0)
		Error("Коструктора с такими парметрами не существует!");

	bool need_testandget=is_static&&(params_result.size()>0||assign_expr!=NULL);

	if((constructor==NULL||!constructor->IsBytecode())&&!is_static)
		program.Push(TOp(TOpcode::PUSH_COUNT,type.GetClass()->GetSize()),before_params);

	if(is_static)
	{
		if(constructor!=NULL)
		{
			ValidateAccess(this,owner,constructor);
			if(need_testandget)
			{
				program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,offset),before_params);
				after_params+=constructor->BuildCall(program,params_result).GetOps();
				/*if(constructor->GetType()==MT_INTERNAL)
				program.Push(TOp(TOpcode::ASSIGN,(char)false,type.GetClass()->GetSize()),ops_array);*/
			}else
			{
				program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,offset),program.static_vars_init);
				program.static_vars_init+=constructor->BuildCall(program,params_result).GetOps();
				/*if(constructor->GetType()==MT_INTERNAL)
				program.Push(TOp(TOpcode::ASSIGN,(char)false,type.GetClass()->GetSize()),program.static_vars_constr_last_op);*/
			}
		}
		TMethod* destructor=type.GetClass()->GetDestructor();
		if(destructor!=NULL)
		{
			ValidateAccess(this,owner,destructor);
			program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,offset),program.static_vars_destroy);
			program.static_vars_destroy+=destructor->BuildCall(program,params_result).GetOps();
		}
	}else if(constructor!=NULL)
	{
		if(!constructor->IsBytecode())
		{
			program.Push(TOp(TOpcode::PUSH_STACK_HIGH_REF,type.GetClass()->GetSize()),before_params);
		}
		ValidateAccess(this,owner,constructor);
		after_params+=constructor->BuildCall(program,params_result).GetOps();
	}

	if(assign_expr!=NULL)
		after_params+=assign_expr->Build(program,local_var_offset).GetOps();
	if(need_testandget)
	{
		int gofalse_label=program.GetUniqueLabel();
		program.Push(TOp(TOpcode::GLOBAL_TESTANDSET,offset,gofalse_label),before_params);
		program.Push(TOp(TOpcode::LABEL,gofalse_label),after_params);
	}
	TOpArray result;
	result+=(before_params);
	result+=(params_ops);
	result+=(after_params);
	return TVoid(result);
}