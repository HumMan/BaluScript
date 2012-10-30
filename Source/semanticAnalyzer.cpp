#include "baluScript.h"

TClass*  TType::GetClass(bool use_build_methods,TNotOptimizedProgram* program)
{
	if(use_build_methods) assert(class_pointer==NULL);
	if(class_pointer==NULL)
	{
		if(class_name.class_pointer!=NULL)
			return class_name.class_pointer;
		if(class_name.name.IsNull())
			return NULL;
		class_pointer=class_name.Build(use_build_methods,NULL,owner,*this,program);
	}
	return class_pointer;
}
TClass* TType::TClassName::Build(bool use_build_methods,TClass* use_curr_class,TClass* owner,TTokenPos& source,TNotOptimizedProgram* program)
{
	//TODO можно сделать все это без рекурсии при пом. цикла и массива состояний к тому же если нет шаблонных параметров, то рекурсия не нужна
	if(use_curr_class==NULL)
	{
		use_curr_class=owner->GetClass(name);
		if(use_curr_class==NULL)
			source.Error("Неизвестный тип!");
	}else
	{
		use_curr_class=use_curr_class->GetNested(name);
		if(use_curr_class==NULL)
			source.Error("Вложенного класса с таким именем не существует!");
	}
	if(template_params.GetCount()!=0)
	{
		TTemplateRealizations* templates=owner->GetTemplates();
		if(!use_curr_class->IsTemplate())
			source.Error("Класс не является шаблонным!");
		if(template_params.GetHigh()!=use_curr_class->GetTemplateParamsHigh())
			source.Error("Шаблон имеет другое количество параметров!");
		for(int i=0;i<=template_params.GetHigh();i++)
			template_params[i]->Build(use_build_methods,NULL,owner,source,program);
		TClass* realization=NULL;
		TVectorList<TClass>* temp=NULL;
		temp=templates->FindTemplate(use_curr_class);
		if(temp!=NULL)
		{
			//TODO !!!!!!!!!!  шаблонный класс не имеет доступа к своему шаблону
			for(int k=0;k<=temp->GetHigh();k++)
			{
				if((*temp)[k]->GetTemplateParamsHigh()!=template_params.GetHigh())
					continue;
				bool found=true;
				for(int t=0;t<=(*temp)[k]->GetTemplateParamsHigh();t++)
				{
					if((*temp)[k]->GetTemplateParamClass(t)!=
						template_params[t]->class_pointer)
					{
						found=false;
						break;
					}
				}
				if(found)
				{
					realization=(*temp)[k];
					break;
				}
			}
		}
		if(realization==NULL)
		{
			if(temp==NULL)
			{
				TTemplateRealizations::TTemplateRealization* t=
					templates->templates.Push(new TTemplateRealizations::TTemplateRealization());
				t->template_pointer=use_curr_class;
				temp=&t->realizations;
			}
			//realization=new TClass(owner,templates);
			//temp->Push(realization);
			//*realization=*use_curr_class;
			realization=new TClass(*use_curr_class);
			temp->Push(realization);
			//
			realization->InitOwner(use_curr_class->GetOwner());
			realization->SetIsTemplate(false);
			for(int i=0;i<=template_params.GetHigh();i++)
				realization->SetTemplateParamClass(i,template_params[i]->class_pointer);
			realization->BuildClass();
			if(use_build_methods)
			{
				realization->DeclareMethods();
				realization->CheckForErrors();
				realization->InitAutoMethods(*program);
				realization->BuildMethods(*program);
			}
		}
		use_curr_class=realization;
	}
	class_pointer=use_curr_class;
	class_pointer->BuildClass();
	if(use_build_methods)
	{
		class_pointer->DeclareMethods();
		class_pointer->InitAutoMethods(*program);
		class_pointer->BuildMethods(*program);
	}
	if(member.IsNull())return use_curr_class;
	else return member->Build(use_build_methods,use_curr_class,owner,source,program);
}

void TStatement::TestBoolExpr(TNotOptimizedProgram &program,TFormalParam& compare_result,int go_false_label)
{
	int conv_needed;
	TClass* bool_class=owner->GetClass(source->GetIdFromName("bool"));
	if(!IsEqualClasses(compare_result,bool_class,false,conv_needed))
		Error("Выражение невозможно преобразовать в логический тип!");
	method->BuildFormalParamConversion(program,compare_result,bool_class,false);	
	program.Push(TOp(TOpcode::GOFALSE,go_false_label),compare_result.GetOps());
}

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

TFormalParam TLocalVar::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	TVector<TMethod*> methods;
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
		Error("Коструктора с такими парметрами не существует!");

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


TFormalParam TExpression::TBinOp::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	TVector<TFormalParam> param;
	TVector<TMethod*> bin_operators;

	param.SetCount(2);
	param[0]=left->Build(program,parent);
	param[1]=right->Build(program,parent);

	int left_conv_count=-1,right_conv_count=-1;

	TMethod *bin_operator=NULL,*bin_operator2=NULL;

	if(param[0].IsVoid())
		Error("К левому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");
	if(param[1].IsVoid())
		Error("К правому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");

	TVector<TMethod*> operators;

	param[0].GetClass()->GetOperators(operators,op);
	bin_operator=FindMethod(this,operators,param,left_conv_count);
	if(op>=TOperator::Assign&&op<=TOperator::OrA && !param[0].IsRef())
		Error("Для присваиваниия требуется ссылка, а не значение!");

	if(!(op>=TOperator::Assign&&op<=TOperator::OrA))
	{
		operators.SetCount(0);
		param[1].GetClass()->GetOperators(operators,op);
		bin_operator2=FindMethod(this,operators,param,right_conv_count);
	}

	TFormalParam result;
	if(bin_operator!=NULL&&(bin_operator2==NULL||left_conv_count<right_conv_count))
	{
		ValidateAccess(this,parent->owner,bin_operator);
		result=bin_operator->BuildCall(program,param);
	}
	else if(bin_operator2!=NULL)
	{
		ValidateAccess(this,parent->owner,bin_operator2);
		result=bin_operator2->BuildCall(program,param);
	}
	else Error("Бинарного оператора для данных типов не существует!");
	return param[0].GetOps()+param[1].GetOps()+result;
}

TFormalParam TExpression::TUnaryOp::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	TVector<TFormalParam> param;

	param.SetCount(1);
	param[0]=left->Build(program,parent);

	int conv_count;
	TMethod *unary_operator=NULL;

	if(param[0].GetClass()==NULL)
		Error("К данному операнду нельзя применить унарный оператор (нужен тип отличающийся от void)!");

	TVector<TMethod*> operators;
	param[0].GetClass()->GetOperators(operators,op);

	unary_operator=FindMethod(this,operators,param,conv_count);

	if(unary_operator!=NULL)
	{
		ValidateAccess(this,parent->owner,unary_operator);
		TFormalParam result=unary_operator->BuildCall(program,param);
		return param[0].GetOps()+result;
	}
	else Error("Унарного оператора для данного типа не существует!");
	return TVoid();
}

TFormalParam TExpression::TId::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	TVariable* var=parent->GetVar(name);
	if(var!=NULL)
	{
		if(var->GetType()==TVariableType::ClassField)
		{
			ValidateAccess(this,parent->owner,(TClassField*)var);
			if(!((TClassField*)var)->IsStatic()&&parent->method->IsStatic())
				Error("К нестатическому полю класса нельзя обращаться из статического метода!");
		}
		TOpArray temp;
		if(var->NeedPushThis())
			program.Push(TOp(TOpcode::PUSH_THIS),temp);
		return temp+var->PushRefToStack(program);
	}
	TVector<TMethod*> methods;
	if(parent->method->IsStatic())
		parent->owner->GetMethods(methods,name,true);
	else
		parent->owner->GetMethods(methods,name);
	if(methods.GetHigh()!=-1)
	{
		return TFormalParam(methods,true);
	}else
	{
		if(parent->method->IsStatic())
		{
			methods.SetHigh(-1);
			if(parent->owner->GetMethods(methods,name,false))
			{
				Error("К нестатическому методу класса нельзя обращаться из статического метода!");
			}
		}
	}
	TClass* type=parent->owner->GetClass(name);
	if(type==NULL)
	{
		Error("Неизвестный идентификатор!");
	}
	return TFormalParam(type);
}

TFormalParam TExpression::TThis::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	if(parent->method->IsStatic())
		Error("Ключевое слово 'this' можно использовать только в нестатических методах!");
	TOpArray temp;
	program.Push(TOp(TOpcode::PUSH_THIS),temp);
	return TFormalParam(parent->owner,true,temp);
}

TFormalParam TExpression::TGetMemberOp::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	TFormalParam left_result=left->Build(program,parent);
	if(left_result.IsMethods())
		Error("Оператор доступа к члену класса нельзя применить к методу!");
	if(left_result.IsType())
	{
		if(left_result.GetType()->IsEnum())
		{
			int id=left_result.GetType()->GetEnumId(name);
			//TODO ввести спец функции min max count
			if(id==-1)
				Error("Перечислимого типа с таким именем не существует!");
			else
			{
				program.Push(TOp(TOpcode::PUSH,id),left_result.GetOps());
				return TFormalParam(left_result.GetType(),false,left_result.GetOps());
			}
		}else
		{
			TClassField* static_member=left_result.GetType()->GetField(name,true,true);
			if(static_member!=NULL)
				return static_member->PushRefToStack(program);
			TVector<TMethod*> methods;
			if(left_result.GetType()->GetMethods(methods,name,true))
				return TFormalParam(methods,false);
			TClass* nested=left_result.GetType()->GetNested(name);
			if(nested!=NULL)
				return TFormalParam(nested);
			Error("Статического поля или метода с таким именем не существует!");
		}
	}
	TClassField* member=left_result.GetClass()!=NULL
		?left_result.GetClass()->GetField(name,true)
		:NULL;
	if(member!=NULL)
	{
		if(member->IsStatic())
			Error("Оператор доступа к члену класса нельзя применить к объекту для доступа к статическому члену, \nвместо объекта следует использовать имя класса!");
		ValidateAccess(this,parent->owner,member);
		if(left_result.IsRef())
		{
			TFormalParam result=member->PushRefToStack(program);
			if(member->IsReadOnly())
			{
				TMethod* copy_constr=result.GetClass()->GetCopyConstr();
				program.Push(TOp(TOpcode::RVALUE,result.GetClass()->GetSize(),program.AddMethodToTable(copy_constr))
					,result.GetOps());
				result.SetIsRef(false);
			}
			return left_result.GetOps()+result;
		}
		else 
		{
			program.Push(TOp(TOpcode::PUSH,left_result.GetClass()->GetSize()),left_result.GetOps());
			program.Push(TOp(TOpcode::PUSH,member->GetOffset()),left_result.GetOps());
			program.Push(TOp(TOpcode::PUSH,member->GetClass()->GetSize()),left_result.GetOps());
			program.Push(TOp(TOpcode::GET_MEMBER,
				program.AddMethodToTable(member->GetClass()->GetCopyConstr()),
				program.AddMethodToTable(member->GetClass()->GetDestructor())),
				left_result.GetOps());
			return TFormalParam(member->GetClass(),false,left_result.GetOps());
		}
		//Error("Оператор доступа к члену объекта нельзя использовать для временного объекта!");//TODO
	}else
	{
		if(!left_result.IsRef())
			Error("Вызов метода для временного объекта недопустим!");
		TVector<TMethod*> methods;
		if(!left_result.GetClass()->GetMethods(methods,name,false))
			Error("Члена класса с таким именем не существует!");
		TFormalParam result(methods,false);
		result.GetOps()=left_result.GetOps();
		return result;
	}
	assert(false);
}

TFormalParam TExpression::TCallParamsOp::Build(TNotOptimizedProgram &program,TExpression* parent)
{
	TFormalParam left_result=left->Build(program,parent);
	TVector<TFormalParam> params_result;

	for(int i=0;i<=param.GetHigh();i++)
		params_result.Push(param[i]->Build(program,parent));
	int conv_need=0;
	if(left_result.IsMethods())
	{
		//вызов метода
		if(is_bracket)assert(false);//при вызове метода используются круглые скобки
		TMethod* method=FindMethod(this,left_result.GetMethods(),params_result,conv_need);
		if(method!=NULL)
		{
			ValidateAccess(this,parent->owner,method);
			TOpArray params_ops;
			TFormalParam call_ops=method->BuildCall(program,params_result);
			for(int i=0;i<=param.GetHigh();i++)
				params_ops+=params_result[i].GetOps();
			if(!method->IsStatic()&&left_result.NeedPushThis())
				program.Push(TOp(TOpcode::PUSH_THIS),params_ops);
			return left_result.GetOps()+params_ops+call_ops;
		}
		else Error("Метода с такими параметрами не существует");
	}
	else if(left_result.IsType())
	{
		//if(left_result.GetType()->GetType()==TYPE_ENUM)
		//	Error("Для перечислений нельзя использовать оператор вызова параметров!");
		int conv_need=-1;
		TVector<TMethod*> constructors;
		TClass* constr_class=left_result.GetType();
		constr_class->GetConstructors(constructors);
		TMethod* constructor=FindMethod(this,constructors,params_result,conv_need);
		TOpArray before_params;
		if(constructor==NULL)
			program.PushFront(TOp(TOpcode::PUSH_COUNT,constr_class->GetSize()),before_params);
		if(constructor!=NULL)
		{
			ValidateAccess(this,parent->owner,constructor);
			//if(constructor->GetType()==MT_USER||constructor->GetType()==MT_EXTERNAL)
			//{
			program.Push(TOp(TOpcode::PUSH_STACK_HIGH_REF,constr_class->GetSize()),before_params);
			//}
			TOpArray call_ops=constructor->BuildCall(program,params_result).GetOps();
			TOpArray params_ops;
			for(int i=0;i<=param.GetHigh();i++)
				params_ops+=params_result[i].GetOps();
			return before_params+params_ops+TFormalParam(constr_class,false,call_ops);
		}else if(params_result.GetHigh()>-1)
			Error("Конструктора с такими параметрами не существует!");
	}
	else
	{
		params_result.Insert(left_result,-1);
		TVector<TMethod*> operators;
		left_result.GetClass()->GetOperators(operators,is_bracket?(TOperator::GetArrayElement):(TOperator::ParamsCall));
		TMethod* method=FindMethod(this,operators,params_result,conv_need);
		if(method!=NULL)
		{
			ValidateAccess(this,parent->owner,method);
			TOpArray params_ops;
			TFormalParam call_ops=method->BuildCall(program,params_result);
			for(int i=0;i<=params_result.GetHigh();i++)
				params_ops+=params_result[i].GetOps();
			return params_ops+call_ops;
		}
		else Error("Оператора с такими параметрами не существует!");
	}
	assert(false);return TVoid();
}

TFormalParam TBytecode::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	TOpArray temp;
	for(int i=0;i<=code.GetHigh();i++)
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

TFormalParam TIf::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	TOpArray ops_array;
	TFormalParam bool_result=bool_expr.Build(program,local_var_offset);

	int end_if=program.GetUniqueLabel();
	TestBoolExpr(program,bool_result,end_if);
	ops_array+=bool_result.GetOps();

	ops_array+=statements.Build(program,local_var_offset).GetOps();
	if(else_statements.GetHigh()>=0)
	{
		int end_else=program.GetUniqueLabel();
		program.Push(TOp(TOpcode::GOTO,end_else),ops_array);
		program.Push(TOp(TOpcode::LABEL,end_if),ops_array);
		ops_array+=else_statements.Build(program,local_var_offset).GetOps();
		program.Push(TOp(TOpcode::LABEL,end_else),ops_array);
	}else
		program.Push(TOp(TOpcode::LABEL,end_if),ops_array);
	return TVoid(ops_array);
}

TFormalParam TFor::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	TOpArray ops_array;
	int start_loop=program.GetUniqueLabel();
	program.Push(TOp(TOpcode::LABEL,start_loop),ops_array);
	TFormalParam compare_result=compare.Build(program,local_var_offset);

	int end_loop=program.GetUniqueLabel();
	TestBoolExpr(program,compare_result,end_loop);
	ops_array+=compare_result.GetOps();

	ops_array+=statements.Build(program,local_var_offset).GetOps();
	ops_array+=increment.Build(program,local_var_offset).GetOps();
	program.Push(TOp(TOpcode::GOTO,start_loop),ops_array);
	program.Push(TOp(TOpcode::LABEL,end_loop),ops_array);
	return TVoid(ops_array);
}

TFormalParam TWhile::Build(TNotOptimizedProgram &program,int& local_var_offset)
{
	/*TFormalParam bool_result=bool_expr.Build(ops_array,program,local_var_offset);
	int conv_needed;
	if(!IsEqualClasses(bool_result,owner->GetClassCmp("bool"),false,conv_needed))
	Error("Выражение невозможно преобразовать в логический тип!");
	method->BuildFormalParamConversion(program,bool_result,owner->GetClassCmp("bool"),false);	
	last_op=bool_result.LastOp();
	int end_if=program.GetUniqueLabel();
	program.Push(TOp(TOpcode::GOFALSE,end_if),ops_array);
	statements.Build(ops_array,program,local_var_offset);
	if(else_statements.GetHigh()>=0)
	{
	int end_else=program.GetUniqueLabel();
	program.Push(TOp(TOpcode::GOTO,end_else),ops_array);
	program.Push(TOp(TOpcode::LABEL,end_if),ops_array);
	else_statements.Build(ops_array,program,local_var_offset);
	program.Push(TOp(TOpcode::LABEL,end_else),ops_array);
	}else
	program.Push(TOp(TOpcode::LABEL,end_if),ops_array);*/
	return TVoid();
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



void TMethod::BuildFormalParamConversion(TNotOptimizedProgram &program,TFormalParam& formal_par,  TClass* param_class,bool param_ref)
{	
	assert(!(param_ref&&!formal_par.IsRef()));//ошибка в FindMethod
	//если необходимо преобразование типа формального параметра то добавляем его
	if(formal_par.GetClass()!=param_class)
	{
		TMethod* conversion=formal_par.GetClass()->GetConversion(formal_par.IsRef(),param_class);
		if(formal_par.IsRef()&&!param_ref)
		{
			if(conversion==NULL)
			{
				TMethod* copy_constr=formal_par.GetClass()->GetCopyConstr();
				program.Push(TOp(TOpcode::RVALUE,formal_par.GetClass()->GetSize(),program.AddMethodToTable(copy_constr))
					,formal_par.GetOps());
				formal_par.SetIsRef(false);
				conversion=formal_par.GetClass()->GetConversion(false,param_class);
			}
		}
		assert(conversion!=NULL);//ошибка в FindMethod
		TVector<TFormalParam> conv_method_params;
		conv_method_params.Push(formal_par);
		formal_par=formal_par.GetOps()+conversion->BuildCall(program,conv_method_params);
	}
	//если в стеке находится ссылка, а в качестве параметра требуется значение, то добавляем преобразование
	else if(formal_par.IsRef()&&!param_ref)
	{
		TMethod* copy_constr=formal_par.GetClass()->GetCopyConstr();
		program.Push(TOp(TOpcode::RVALUE,formal_par.GetClass()->GetSize(),program.AddMethodToTable(copy_constr))
			,formal_par.GetOps());
	}
}

void TMethod::BuildFormalParamsConversion(TNotOptimizedProgram &program,TVector<TFormalParam> &formal_params)
{
	assert(formal_params.GetHigh()==param.GetHigh());//ошибка в FindMethod
	for(int i=0;i<=param.GetHigh();i++){
		BuildFormalParamConversion(program,formal_params[i],param[i]->GetClass(),param[i]->IsRef());			
	}
}

TFormalParam TMethod::BuildCall(TNotOptimizedProgram &program,TClass* par0,bool par0_ref,TOpArray& par0_ops,TClass* par1,bool par1_ref,TOpArray& par1_ops)
{
	TVector<TFormalParam> params;
	params.SetHigh(1);
	params[0]=TFormalParam(par0,par0_ref,par0_ops);
	params[1]=TFormalParam(par1,par1_ref,par1_ops);
	return BuildCall(program,params);
}

TFormalParam TMethod::BuildCall(TNotOptimizedProgram &program,TClass* par0,bool par0_ref,TOpArray& par0_ops)
{
	TVector<TFormalParam> params;
	params.SetHigh(0);
	params[0]=TFormalParam(par0,par0_ref,par0_ops);
	return BuildCall(program,params);
}

TFormalParam TMethod::BuildCall(TNotOptimizedProgram &program)
{
	TVector<TFormalParam> params;
	return BuildCall(program,params);
}

TFormalParam TMethod::BuildCall(TNotOptimizedProgram &program,TVector<TFormalParam> &formal_params)
{
	assert(this!=NULL);
	if(member_type!=TClassMember::Conversion)
		BuildFormalParamsConversion(program,formal_params);
	if(is_bytecode==1)
		return TFormalParam(ret.GetClass(),ret_ref,ops_array);
	else 
	{
		TOpArray ops_array;
		if(IsBytecode())
		{
			int not_used=-1;
			ops_array+=statements.Build(program,not_used).GetOps();
		}
		else
			program.Push(TOp(TOpcode::CALL_METHOD,program.AddMethodToTable(this)),ops_array);
		return TFormalParam(ret.GetClass(),ret_ref,ops_array);
	}
}

void TMethod::Declare()
{
	params_size=0;
	for(int i=0;i<=param.GetHigh();i++)
	{
		param[i]->SetOffset(params_size);
		params_size+=param[i]->GetSize();
		param[i]->GetClass()->BuildClass();
	}
	if(ret.GetClass()!=NULL)
		ret.GetClass()->BuildClass();
	CheckForErrors();
}

void TMethod::Build(TNotOptimizedProgram &program)
{
	if(!(is_extern||IsBytecode()))
	{
		int local_var_offset=params_size;
		ops_array+=statements.Build(program,local_var_offset).GetOps();
		if(ret.GetClass()!=NULL)
		{
			if(!has_return)
				Error("Метод должен возвращать значение!");
			//TODO не работает с методами содержащими только байткод
			//program.Push(TOp(TOpcode::METHOD_HAS_NOT_RETURN_A_VALUE),ops_array);
			program.Push(TOp(TOpcode::RETURN,
				GetParamsSize()+!IsStatic(),GetRetSize()),ops_array);
			//TODO добавить информацию о методе
			//TODO лучше добавить проверку на return, но как быть с байткодом по которому нельзя определить возвращает ли он значение или нет
		}
		else{
			ops_array+=BuildParametersDestructor(program);
			program.Push(TOp(TOpcode::RETURN,params_size+!is_static,0),ops_array);
		}
	}
}

TOpArray TMethod::BuildParametersDestructor(TNotOptimizedProgram &program)
{
	TOpArray ops_array;
	for(int i=0;i<=param.GetHigh();i++)
	{
		if(!param[i]->IsRef()&&param[i]->GetClass()->HasDestr())
		{
			program.Push(TOp(TOpcode::PUSH_LOCAL_REF,param[i]->GetOffset()),ops_array);
			ops_array+=param[i]->GetClass()->GetDestructor()->BuildCall(program).GetOps();
		}
	}
	return ops_array;
}

int TMethod::GetRetSize()
{
	if(ret.GetClass()==NULL)return 0;
	return ret_ref?1:ret.GetClass()->GetSize();
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

int TParameter::GetSize(){
	return is_ref?1:type.GetClass()->GetSize();
}

TVariable* TMethod::GetVar(TNameId name)
{
	for(int i=0;i<=param.GetHigh();i++)
		if(param[i]->GetName()==name)return param[i];
	return owner->GetField(name,false);
}

void TMethod::CheckForErrors()
{
	if(owner->GetOwner()==NULL&&!is_static)
		Error("Базовый класс может содержать только статические методы!");
	for(int i=0;i<=param.GetHigh();i++)
	{
		if(!param[i]->GetName().IsNull())
			for(int k=0;k<i;k++)
			{
				if(param[i]->GetName()==param[k]->GetName())
					param[i]->Error("Параметр с таким именем уже существует!");
			}
	}
	if(!method_name.IsNull())
	{
		if(owner->GetClass(method_name)!=NULL)
			Error("Класс не может быть именем метода!");
		if(owner->GetField(method_name,false)!=NULL)
			Error("Член класса с таким именем уже существует!");
		//TODO проверить члены родительского класса и т.д. (полный запрет на перекрытие имен)
	}
	switch(member_type)
	{
	case TResWord::Func:	
		assert(!method_name.IsNull());
		break;
	case TResWord::Constr:	
		if(is_static)Error("Конструктор должен быть не статичным!");
		break;
	case TResWord::Destr:
		if(is_static)Error("Деструктор должен быть не статичным!");
		break;
	case TResWord::Operator:
		if(!is_static)Error("Оператор должен быть статичным!");
		break;
	case TResWord::Conversion:
		if(!is_static)Error("Оператор приведения типа должен быть статичным!");
		break;
	default:assert(false);
	}
	{
		//проверяем правильность указания параметров и возвращаемого значения
		switch(member_type)
		{
		case TClassMember::Func:	
			break;
		case TClassMember::Constr:	
			if(ret.GetClass()!=NULL)Error("Конструктор не должен возвращать значение!");
			break;
		case TClassMember::Destr:	
			if(ret.GetClass()!=NULL)Error("Деструктор не должен возвращать значение!");
			if(param.GetCount()!=0)Error("Деструктор не имеет параметров!");
			break;
		case TClassMember::Operator:
			if(operator_type==TOperator::Not)//унарные операторы
			{
				if(GetParamsHigh()!=0)
					Error("Унарный оператор должен иметь один параметр!");
				if(GetParam(0)->GetClass()!=owner)
					Error("Хотя бы один из параметров оператора должен быть классом для которого он используется!");
			}
			else if(operator_type==TOperator::UnaryMinus)
			{
				if(!IsIn(GetParamsHigh(),0,1))
					Error("У унарного оператора ""-"" должнен быть 1 параметр!");
				if(GetParam(0)->GetClass()!=owner
					&&(GetParamsHigh()==1&&GetParam(1)->GetClass()!=owner))
					Error("Параметром унарного оператора должен быть класс для которого он используется!");
			}
			else if(operator_type==TOperator::ParamsCall||operator_type==TOperator::GetArrayElement)
			{
				if(GetParamsHigh()<1)
					Error("Оператор вызова параметров должен иметь минимум 2 операнда!");
				if(GetParam(0)->GetClass()!=owner)
					Error("Первый параметр оператора вызова должен быть данным классом!");
			}
			else //остальные бинарные операторы
			{
				if((operator_type==TOperator::Equal||operator_type==TOperator::NotEqual)&&ret.GetClass()!=owner->GetClass(source->GetIdFromName("bool")))
					Error("Оператор сравнения должен возвращать логическое значение!");
				if(GetParamsHigh()!=1)
					Error("У бинарного оператора должно быть 2 параметра!");
				if(GetParam(0)->GetClass()!=owner
					&&(GetParamsHigh()==1&&GetParam(1)->GetClass()!=owner))
					Error("Хотя бы один из параметров оператора должен быть классом для которого он используется!");
			}
			break;
		case TClassMember::Conversion:
			if(GetParamsHigh()!=0
				||GetParam(0)->GetClass()!=owner)
				Error("Оператор приведения типа должен иметь один параметр с типом равным классу в котором он находится!");
			break;
		default:assert(false);
		}
	}
}

void ValidateAccess(TTokenPos* field_pos,TClass* source,TClassField* target)
{
	if(target->GetAccess()==TTypeOfAccess::Public)return;
	if(source==target->GetOwner())return;
	if(target->GetAccess()==TTypeOfAccess::Protected&&!source->IsChildOf(target->GetOwner()))
		field_pos->Error("Данное поле класса доступно только из классов наследников (protected)!");
	else if(target->GetAccess()==TTypeOfAccess::Private&&source!=target->GetOwner())
		field_pos->Error("Данное поле класса доступно только из класса в котором оно объявлено (private)!");
}

void ValidateAccess(TTokenPos* field_pos,TClass* source,TMethod* target)
{
	if(target->GetAccess()==TTypeOfAccess::Public)return;
	if(source==target->GetOwner())return;
	if(target->GetAccess()==TTypeOfAccess::Protected&&!source->IsChildOf(target->GetOwner()))
		field_pos->Error("Данный метод доступен только из классов наследников (protected)!");
	else if(target->GetAccess()==TTypeOfAccess::Private&&source!=target->GetOwner())
		field_pos->Error("Данный метод доступен только из класса в котором он объявлен (private)!");
}

bool IsEqualClasses(TFormalParam formal_par,TClass* param_class,bool param_ref,int& need_conv)
//============== На выходе =========================================
//результат - равенство классов или возможность приведения класса
{
	need_conv=0;
	if((!formal_par.IsRef())&&param_ref)return false;		
	if(formal_par.IsMethods()||formal_par.IsType())return false;
	if(param_class!=formal_par.GetClass())
	{
		if(!formal_par.GetClass()->HasConversion(param_class))return false;
		if(param_ref&&!formal_par.GetClass()->IsChildOf(param_class))return false;
		need_conv+=1;	
	}
	if(formal_par.IsRef()&&!param_ref)need_conv+=1;
	return true;
}

TMethod* FindMethod(TTokenPos* source, TVector<TMethod*> &methods_to_call,TVector<TFormalParam> &formal_params,int& conv_needed)
{
	for(int k=0;k<=formal_params.GetHigh();k++){
		if(formal_params[k].IsVoid())
			source->Error("Параметр метода должен иметь тип отличный от void!");
	}
	int i,k;
	int min_conv_method=-1,temp_conv,conv;
	conv_needed=-1;
	for (i=0;i<=methods_to_call.GetHigh();i++)
	{			
		if(formal_params.GetHigh()==-1&&methods_to_call[i]->GetParamsHigh()==-1){
			conv_needed=0;
			return methods_to_call[i];
		}
		if(formal_params.GetHigh()!=methods_to_call[i]->GetParamsHigh())goto end_search;
		temp_conv=0;
		conv=0;
		for(k=0;k<=formal_params.GetHigh();k++){
			TParameter* p=methods_to_call[i]->GetParam(k);
			if(!IsEqualClasses(formal_params[k],p->GetClass(),p->IsRef(),conv))goto end_search;
			else temp_conv+=conv;
		}
		if(temp_conv<conv_needed||conv_needed==-1)
		{
			conv_needed=temp_conv;
			min_conv_method=i;
		}end_search:
		continue;
	}
	if(min_conv_method>=0)
		return methods_to_call[min_conv_method];		
	return NULL;
}

void TClass::CheckForErrors()
{
	if(owner!=NULL&&owner->GetOwner()!=NULL&&owner->GetOwner()->GetClass(name))
		Error("Класс с таким именем уже существует!");
	for(int i=0;i<=nested_class.GetHigh();i++)
	{
		for(int k=0;k<i;k++)
		{
			if(nested_class[i]->GetName()==nested_class[k]->GetName())
				nested_class[i]->Error("Класс с таким именем уже существует!");
		}
	}
	for(int i=0;i<=field.GetHigh();i++)
	{
		for(int k=0;k<i;k++)
		{
			if(owner==NULL&&!field[i]->IsStatic())
				Error("Базовый класс может содержать только статические поля!");
			if(field[i]->GetName()==field[k]->GetName())
				field[i]->Error("Поле класса с таким именем уже существует!");
			//TODO как быть со статическими членами класса
		}
		TVector<TMethod*> m;
		if((owner!=NULL&&owner->GetField(name,true,false)!=NULL)||GetMethods(m,name))
			field[i]->Error("Член класса с таким имененем уже существует!");
	}
	for(int i=0;i<=method.GetHigh();i++)
	{
		if((owner!=NULL&&owner->GetField(method[i]->GetName(),true,false)!=NULL))
			method[i]->method[0]->Error("Статическое поле класса с таким имененем уже существует!");
		method[i]->CheckForErrors();
		TVector<TMethod*> owner_methods;
		if(owner!=NULL&&owner->GetMethods(owner_methods,method[i]->GetName()))
		{
			for(int k=0;k<=owner_methods.GetHigh();k++)
			{
				TMethod* temp=method[i]->FindParams(owner_methods[k]);
				if(temp!=NULL)
					temp->Error("Статический метод с такими параметрами уже существует!");
			}
		}
	}
	constructor.CheckForErrors();
	conversion.CheckForErrors(true);
	for(int i=0;i<TOperator::End;i++)
	{
		operators[i].CheckForErrors();
	}
	for(int i=0;i<=nested_class.GetHigh();i++)
		if(!nested_class[i]->IsTemplate())
			nested_class[i]->DeclareMethods();
}

void TClass::InitAutoMethods(TNotOptimizedProgram &program)
{
	assert(!is_template);
	if(auto_methods_build)return;
	if(parent.GetClass()!=NULL)
		parent.GetClass()->InitAutoMethods(program);
	for(int i=0;i<=field.GetHigh();i++)
	{
		field[i]->GetClass()->InitAutoMethods(program);
		//создаем конструктор для статических полей класса
		if(field[i]->IsStatic())
		{
			field[i]->SetOffset(program.static_vars_size);
			TVector<TMethod*> constructors;
			field[i]->GetClass()->GetConstructors(constructors);
			TVector<TFormalParam> params;
			int conv_need=0;
			{
				TMethod* constructor=FindMethod(this,constructors,params,conv_need);
				if(constructor!=NULL)
				{
					ValidateAccess(field[i],this,constructor);
					program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,program.static_vars_size),program.static_vars_init);
					program.static_vars_init+=constructor->BuildCall(program,params).GetOps();
				}
			}
			{
				TMethod* destructor=field[i]->GetClass()->GetDestructor();
				if(destructor!=NULL)
				{
					ValidateAccess(field[i],this,destructor);
					program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,program.static_vars_size),program.static_vars_destroy);
					program.static_vars_destroy+=destructor->BuildCall(program,params).GetOps();
				}
			}
			program.static_vars_size+=field[i]->GetClass()->GetSize();
		}
	}
	//TODO что делать если будет bytecode пользовательский конструктор и auto_def_constr?
	{//defConstructor
		bool field_has_def_constr=false;
		bool parent_has_def_constr=parent.GetClass()==NULL?false:parent.GetClass()->HasDefConstr();
		for(int i=0;i<=field.GetHigh();i++){
			TMethod* field_def_constr=field[i]->GetClass()->GetDefConstr();
			if(field_def_constr!=NULL&&!field_def_constr->IsBytecode()&&!field[i]->IsStatic()){
				field_has_def_constr=true;
				break;
			}
		}
		if(field_has_def_constr||parent_has_def_constr)
		{
			TOpArray ops;
			if(parent_has_def_constr)
			{
				ValidateAccess(&parent,this,parent.GetClass()->GetDefConstr());
				program.Push(TOp(TOpcode::PUSH_THIS),ops);
				ops+=parent.GetClass()->GetDefConstr()->BuildCall(program).GetOps();
			}
			for(int i=0;i<=field.GetHigh();i++)
			{
				if(!field[i]->IsStatic())
				{
					TClass* field_class=field[i]->GetClass();
					if(field_class->HasDefConstr())
					{
						ValidateAccess(field[i],this,field_class->GetDefConstr());
						program.Push(TOp(TOpcode::PUSH_MEMBER_REF,field[i]->GetOffset()),ops);
						ops+=field_class->GetDefConstr()->BuildCall(program).GetOps();
					}
				}
			}
			program.Push(TOp(TOpcode::RETURN,1,0),ops);
			auto_def_constr = new TMethod(this);
			auto_def_constr->SetAs(ops, NULL, false, false);
			auto_def_constr->CalcParamSize();
			for(int i=0;i<=constructor.method.GetHigh();i++)
				if(constructor.method[i]!=NULL&&!constructor.method[i]->IsBytecode())
					constructor.method[i]->SetPreEvent(auto_def_constr.GetPointer());
		}
	}

	if(GetCopyConstr()==NULL)
	{
		bool field_has_copy_constr=false;
		bool parent_has_copy_constr=parent.GetClass()==NULL?false:parent.GetClass()->HasCopyConstr();
		for(int i=0;i<=field.GetHigh();i++){
			TMethod* field_copy_constr=field[i]->GetClass()->GetCopyConstr();
			if(field_copy_constr!=NULL&&!field_copy_constr->IsBytecode()&&!field[i]->IsStatic()){
				field_has_copy_constr=true;
				break;
			}
		}
		if(field_has_copy_constr||parent_has_copy_constr)
		{
			TOpArray ops;
			//копируем члены не имеющие конструктора копии
			program.Push(TOp(TOpcode::PUSH_THIS),ops);//TODO копирование должно производиться один раз
			program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,0),ops);
			program.Push(TOp(TOpcode::ASSIGN,(char)true,GetSize()),ops);
			if(parent_has_copy_constr)
			{
				ValidateAccess(&parent,this,parent.GetClass()->GetCopyConstr());//TODO в таких местах будет непонятно где ошибка
				program.Push(TOp(TOpcode::PUSH_THIS),ops);
				program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,0),ops);
				ops+=parent.GetClass()->GetCopyConstr()->BuildCall(program,parent.GetClass(),true,ops).GetOps();
			}
			//вызываем конструкторы копий
			for(int i=0;i<=field.GetHigh();i++)
			{
				if(!field[i]->IsStatic())
				{
					TClass* field_class=field[i]->GetClass();
					if(field_class->HasCopyConstr())
					{
						ValidateAccess(field[i],this,field_class->GetCopyConstr());
						program.Push(TOp(TOpcode::PUSH_MEMBER_REF,field[i]->GetOffset()),ops);
						program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,0),ops);
						program.Push(TOp(TOpcode::ADD_OFFSET,field[i]->GetOffset()),ops);
						ops+=field_class->GetCopyConstr()->BuildCall(program,field_class,true,ops).GetOps();
					}
				}
			}
			program.Push(TOp(TOpcode::RETURN,1+1,0),ops);
			TMethod* auto_copy_constr=new TMethod(this);
			auto_copy_constr->SetAs(ops, NULL, false, false);
			TParameter* t = new TParameter(this, auto_copy_constr);
			t->SetAs(true, this);
			auto_copy_constr->AddParam(t);
			auto_copy_constr->CalcParamSize();
			constructor.method.Push(auto_copy_constr);
		}
	}

	{
		bool field_has_destr=false;
		bool parent_has_destr=parent.GetClass()==NULL?false:parent.GetClass()->HasDestr();
		for(int i=0;i<=field.GetHigh();i++){
			TMethod* field_destr=field[i]->GetClass()->GetDestructor();
			if(field_destr!=NULL&&!field_destr->IsBytecode()&&!field[i]->IsStatic()){
				field_has_destr=true;
				break;
			}
		}
		if(field_has_destr||parent_has_destr)
		{
			TOpArray ops;
			if(parent_has_destr)
			{
				ValidateAccess(&parent,this,parent.GetClass()->GetDestructor());
				program.Push(TOp(TOpcode::PUSH_THIS),ops);
				ops+=parent.GetClass()->GetDestructor()->BuildCall(program).GetOps();
			}
			for(int i=0;i<=field.GetHigh();i++)
			{
				if(!field[i]->IsStatic())
				{
					TClass* field_class=field[i]->GetClass();
					if(field_class->HasDestr())
					{
						ValidateAccess(field[i],this,field_class->GetDestructor());
						program.Push(TOp(TOpcode::PUSH_MEMBER_REF,field[i]->GetOffset()),ops);
						ops+=field_class->GetDestructor()->BuildCall(program).GetOps();
					}
				}
			}
			program.Push(TOp(TOpcode::RETURN,1,0),ops);
			auto_destr = new TMethod(this);
			auto_destr->SetAs(ops, NULL, false, false);
			auto_destr->CalcParamSize();
			if(!destructor.IsNull()&&!destructor->IsBytecode())
				destructor->SetPostEvent(auto_destr.GetPointer());
		}
	}

	if(operators[TOperator::Assign].method.GetCount()==0)
	{
		//TODO оператор = (&left,right)
		//TODO у членов проверяется только оператор (&,&) нужно брать в расчет и (&,val)
		bool field_has_assign_op=false;
		TMethod* parent_assign_op=parent.GetClass()!=NULL?parent.GetClass()->GetBinOp(TOperator::Assign,parent.GetClass(),true,parent.GetClass(),true):NULL;
		//if(parent_assign_op!=NULL&&parent_assign_op->GetType()==MT_INTERNAL)parent_assign_op=NULL;
		bool parent_has_assign_op=parent.GetClass()==NULL?false:parent_assign_op!=NULL;
		for(int i=0;i<=field.GetHigh();i++){
			if(!field[i]->IsStatic())
			{
				TClass* field_class=field[i]->GetClass();//TODO проверять не является ли метод bytecode
				TMethod* field_assign_op=field_class->GetBinOp(TOperator::Assign,field_class,true,field_class,true);
				if(field_assign_op!=NULL&&!field_assign_op->IsBytecode()){
					field_has_assign_op=true;break;
				}
			}
		}
		if(field_has_assign_op||parent_has_assign_op)
		{
			TOpArray ops;
			if(parent_has_assign_op)
			{
				ValidateAccess(&parent,this,parent_assign_op);//TODO доделать далее и подправить до
				program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,0),ops);
				program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,1),ops);
				parent_assign_op->BuildCall(program,parent.GetClass(),true,ops,parent.GetClass(),true,ops);
			}
			for(int i=0;i<=field.GetHigh();i++)
			{
				if(!field[i]->IsStatic())
				{
					TClass* field_class=field[i]->GetClass();

					program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,0),ops);
					program.Push(TOp(TOpcode::ADD_OFFSET,field[i]->GetOffset()),ops);
					program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,1),ops);
					program.Push(TOp(TOpcode::ADD_OFFSET,field[i]->GetOffset()),ops);
					TMethod* field_assign_op=field_class->GetBinOp(TOperator::Assign,field_class,true,field_class,true);
					if(field_assign_op!=NULL)
					{
						ValidateAccess(field[i],this,field_assign_op);
						ops+=field_assign_op->BuildCall(program,field_class,true,ops,field_class,true,ops).GetOps();
					}
					else
						program.Push(TOp(TOpcode::ASSIGN,(char)true,field_class->GetSize()),ops);
				}
			}
			program.Push(TOp(TOpcode::RETURN,1+1,0),ops);

			TMethod* method = new TMethod(this);
			method->SetAs(ops, NULL, false, true);
			TParameter* t = new TParameter(this, method);
			t->SetAs(true, this);
			method->AddParam(t);
			t = new TParameter(this, method);
			t->SetAs(true, this);
			method->AddParam(t);
			method->CalcParamSize();
			operators[TOperator::Assign].method.Push(method);
		}else
		{
			TOpArray ops;
			program.Push(TOp(TOpcode::ASSIGN,(char)true,GetSize()),ops);

			//TMethod* op=new TMethod(this);
			//op->SetAs(ops,NULL,false,true,1);
			//operators[TOperator::Assign].method.Push(op);	

			TMethod* op = new TMethod(this);
			op->SetAs(ops, NULL, false, true,1);
			TParameter* t = new TParameter(this, op);
			t->SetAs(true, this);
			op->AddParam(t);
			t = new TParameter(this, op);
			t->SetAs(true, this);
			op->AddParam(t);
			op->CalcParamSize();

			operators[TOperator::Assign].method.Push(op);
		}
	}
	if(is_enum)
	{
		TOpArray ops;
		program.Push(TOp(TOpcode::EQUAL,false,false,GetSize()),ops);

		TMethod* op = new TMethod(this);
		op->SetAs(ops, GetClass(source->GetIdFromName("bool")), false, true,1);
		TParameter* t = new TParameter(this, op);
		t->SetAs(false, this);
		op->AddParam(t);
		t = new TParameter(this, op);
		t->SetAs(false, this);
		op->AddParam(t);
		op->CalcParamSize();

		operators[TOperator::Equal].method.Push(op);
	}
	auto_methods_build=true;
	for(int i=0;i<=nested_class.GetHigh();i++)
		if(!nested_class[i]->IsTemplate())
			nested_class[i]->InitAutoMethods(program);
}

void TSyntaxAnalyzer::Compile(char* use_source,TTime& time)
{
	unsigned long long t=time.GetTime();
	lexer.ParseSource(use_source);
	printf("Source parsing = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();
	base_class=new TClass(NULL,&templates);	
	base_class->InitPos(lexer);
	base_class->CreateInternalClasses();

	base_class->AnalyzeSyntax(lexer);
	printf("Syntax analyzing = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();
	base_class->BuildClass();
	base_class->DeclareMethods();
	base_class->InitAutoMethods(program);
	printf("Build class, declare methods, automethods = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();
	base_class->BuildMethods(program);
	printf("Methods building = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();
	program.Push(TOp(TOpcode::RETURN,0,0),program.static_vars_init);
	program.Push(TOp(TOpcode::RETURN,0,0),program.static_vars_destroy);
}


int TSyntaxAnalyzer::GetMethod(char* use_method)
{
	lexer.ParseSource(use_method);
	TMethod* method_decl=new TMethod(base_class);
	method_decl->AnalyzeSyntax(lexer,false);
	method_decl->Declare();
	TVector<TMethod*> methods;
	TMethod* method=NULL;
	switch(method_decl->GetMemberType())
	{
	case TClassMember::Func:	
		method_decl->GetOwner()->GetMethods(methods,method_decl->GetName());
		break;
	case TClassMember::Constr:	
		method_decl->GetOwner()->GetConstructors(methods);
		break;
	case TClassMember::Destr:	
		method=method_decl->GetOwner()->GetDestructor();
		break;
	case TClassMember::Operator:
		method_decl->GetOwner()->GetOperators(methods,method_decl->GetOperatorType());
		break;
	case TClassMember::Conversion:
		method=method_decl->GetOwner()->GetConversion(method_decl->GetParam(0)->IsRef(),method_decl->GetRetClass());
		break;
	default:assert(false);
	}
	TClassMember::Enum temp=method_decl->GetMemberType();
	if(temp==TClassMember::Func||
			temp==TClassMember::Constr||
			temp==TClassMember::Operator)
	{
		for (int i=0;i<=methods.GetHigh();i++)
		{
			if(method_decl->GetParamsHigh()==-1&&methods[i]->GetParamsHigh()==-1){
				method=methods[i];
				break;
			}
			if(method_decl->GetParamsHigh()!=methods[i]->GetParamsHigh())continue;
			for(int k=0;k<=method_decl->GetParamsHigh();k++)
			{
				if(!methods[i]->GetParam(k)->IsEqualTo(*method_decl->GetParam(k)))
					goto end_search;
			}
			method=methods[i];
			i=methods.GetCount();end_search:continue;
		}
	}

	if(method!=NULL)
	{
		if(method_decl->IsStatic()!=method->IsStatic())
			lexer.Error("Метод отличается по статичности!");
		if(method_decl->IsExternal()!=method->IsExternal())
			lexer.Error("Несоответствует классификатор extern!");
		if(method_decl->GetRetClass()!=method->GetRetClass()
			||method_decl->IsReturnRef()!=method->IsReturnRef())
			lexer.Error("Метод возвращает другое значение!");
		delete method_decl;
		return program.AddMethodToTable(method);
	}
	else
		lexer.Error("Такого метода не существует!");
	delete method_decl;
	return -1;
}

TClassField* TSyntaxAnalyzer::GetStaticField(char* use_var)
{
	lexer.ParseSource(use_var);
	if(lexer.NameId()!=base_class->GetName())
		lexer.Error("Ожидалось имя класса!");
	lexer.GetToken();
	TClassField* result=NULL;
	TClass* curr_class=base_class;
	while(lexer.Test(TTokenType::Dot))
	{
		lexer.GetToken(TTokenType::Dot);
		lexer.TestToken(TTokenType::Identifier);
		TClass* t=curr_class->GetNested(lexer.NameId());
		if(t==NULL)
		{
			result=curr_class->GetField(lexer.NameId(),true);
		}else curr_class=t;
	}
	if(result==NULL)lexer.Error("Статического члена класса с таким именем не существует!");
	//if(!result->IsStatic())lexer->Error("Член класса с таким именем не является статическим!");
	return result;
}
