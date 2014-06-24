
TFormalParam TLocalVar::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	TVector<TMethod*> methods;
	if(owner->GetMethods(methods,name))
		Error("����� �� ����� ���� ������ ����������!");
	if(owner->GetClass(name)!=NULL)
		Error("����� �� ����� ���� ������ ����������!");
	TVariable* t=parent->GetVar(name,stmt_id);
	if(t!=NULL&&t!=this)
	{
		switch(t->GetType())
		{
		case TVariableType::ClassField:
			Error("��������� ���������� ����������� ���� ������!");
		case TVariableType::Parameter:
			Error("��������� ���������� ����������� ��������!");
		case TVariableType::LocalVar:
			Error("��������� ���������� � ����� ������ ��� ����������!");
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

	TVector<TFormalParam> params_result;
	TOpArray params_ops,before_params,after_params;
	for(int i=0;i<=params.GetHigh();i++)
	{
		params_result.Push(params[i]->Build(program,local_var_offset));
		params_ops+=params_result[i].GetOps();
	}
	int conv_need=0;
	TVector<TMethod*> constructors;
	type.GetClass()->GetConstructors(constructors);
	TMethod* constructor=FindMethod(this,constructors,params_result,conv_need);
	if(constructor==NULL&&params_result.GetHigh()>-1)
		Error("����������� � ������ ���������� �� ����������!");

	bool need_testandget=is_static&&(params_result.GetHigh()>-1||assign_expr!=NULL);

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