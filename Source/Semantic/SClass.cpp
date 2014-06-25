#include "../Syntax/Class.h"

#include "../syntaxAnalyzer.h"
#include "../Syntax/ClassField.h"
#include "../Syntax/Method.h"
#include "../Syntax/Statements.h"

void TClass::CheckForErrors()
{
	if(owner!=NULL&&owner->GetOwner()!=NULL&&owner->GetOwner()->GetClass(name))
		Error("Класс с таким именем уже существует!");
	//for (const std::unique_ptr<TClass> nested_class : nested_classes)
	for(int i=0;i<nested_classes.size();i++)
	{
		for(int k=0;k<i;k++)
		{
			if(nested_classes[i]->GetName()==nested_classes[k]->GetName())
				nested_classes[i]->Error("Класс с таким именем уже существует!");
		}
	}
	for(int i=0;i<fields.size();i++)
	{
		for(int k=0;k<i;k++)
		{
			if(owner==NULL&&!fields[i]->IsStatic())
				Error("Базовый класс может содержать только статические поля!");
			if(fields[i]->GetName()==fields[k]->GetName())
				fields[i]->Error("Поле класса с таким именем уже существует!");
			//TODO как быть со статическими членами класса
		}
		std::vector<TMethod*> m;
		if((owner!=NULL&&owner->GetField(name,true,false)!=NULL)||GetMethods(m,name))
			fields[i]->Error("Член класса с таким имененем уже существует!");
	}
	for(int i=0;i<methods.size();i++)
	{
		if((owner!=NULL&&owner->GetField(methods[i]->GetName(),true,false)!=NULL))
			methods[i]->methods[0]->Error("Статическое поле класса с таким имененем уже существует!");
		methods[i]->CheckForErrors();
		std::vector<TMethod*> owner_methods;
		if(owner!=NULL&&owner->GetMethods(owner_methods,methods[i]->GetName()))
		{
			for(int k=0;k<owner_methods.size();k++)
			{
				TMethod* temp=methods[i]->FindParams(owner_methods[k]);
				if(temp!=NULL)
					temp->Error("Статический метод с такими параметрами уже существует!");
			}
		}
	}
	constructors.CheckForErrors();
	conversions.CheckForErrors(true);
	for(int i=0;i<TOperator::End;i++)
	{
		operators[i].CheckForErrors();
	}
	for(int i=0;i<nested_classes.size();i++)
		if(!nested_classes[i]->IsTemplate())
			nested_classes[i]->DeclareMethods();
}

void TClass::InitAutoMethods(TNotOptimizedProgram &program)
{
	assert(!is_template);
	if(auto_methods_build)return;
	if(parent.GetClass()!=NULL)
		parent.GetClass()->InitAutoMethods(program);
	for(int i=0;i<fields.size();i++)
	{
		fields[i]->GetClass()->InitAutoMethods(program);
		//создаем конструктор для статических полей класса
		if(fields[i]->IsStatic())
		{
			fields[i]->SetOffset(program.static_vars_size);
			std::vector<TMethod*> constructors;
			fields[i]->GetClass()->GetConstructors(constructors);
			std::vector<TFormalParam> params;
			int conv_need=0;
			{
				TMethod* constructor=FindMethod(this,constructors,params,conv_need);
				if(constructor!=NULL)
				{
					ValidateAccess(fields[i].get(),this,constructor);
					program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,program.static_vars_size),program.static_vars_init);
					program.static_vars_init+=constructor->BuildCall(program,params).GetOps();
				}
			}
			{
				TMethod* destructor=fields[i]->GetClass()->GetDestructor();
				if(destructor!=NULL)
				{
					ValidateAccess(fields[i].get(),this,destructor);
					program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,program.static_vars_size),program.static_vars_destroy);
					program.static_vars_destroy+=destructor->BuildCall(program,params).GetOps();
				}
			}
			program.static_vars_size+=fields[i]->GetClass()->GetSize();
		}
	}
	//TODO что делать если будет bytecode пользовательский конструктор и auto_def_constr?
	{//defConstructor
		bool field_has_def_constr=false;
		bool parent_has_def_constr=parent.GetClass()==NULL?false:parent.GetClass()->HasDefConstr();
		for(int i=0;i<fields.size();i++){
			TMethod* field_def_constr=fields[i]->GetClass()->GetDefConstr();
			if(field_def_constr!=NULL&&!field_def_constr->IsBytecode()&&!fields[i]->IsStatic()){
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
			for(int i=0;i<fields.size();i++)
			{
				if(!fields[i]->IsStatic())
				{
					TClass* field_class=fields[i]->GetClass();
					if(field_class->HasDefConstr())
					{
						ValidateAccess(fields[i].get(),this,field_class->GetDefConstr());
						program.Push(TOp(TOpcode::PUSH_MEMBER_REF,fields[i]->GetOffset()),ops);
						ops+=field_class->GetDefConstr()->BuildCall(program).GetOps();
					}
				}
			}
			program.Push(TOp(TOpcode::RETURN,1,0),ops);
			auto_def_constr = std::unique_ptr<TMethod>(new TMethod(this));
			auto_def_constr->SetAs(ops, NULL, false, false);
			auto_def_constr->CalcParamSize();
			for(int i=0;i<constructors.methods.size();i++)
				if(constructors.methods[i]!=NULL&&!constructors.methods[i]->IsBytecode())
					constructors.methods[i]->SetPreEvent(auto_def_constr.get());
		}
	}

	if(GetCopyConstr()==NULL)
	{
		bool field_has_copy_constr=false;
		bool parent_has_copy_constr=parent.GetClass()==NULL?false:parent.GetClass()->HasCopyConstr();
		for(int i=0;i<fields.size();i++){
			TMethod* field_copy_constr=fields[i]->GetClass()->GetCopyConstr();
			if(field_copy_constr!=NULL&&!field_copy_constr->IsBytecode()&&!fields[i]->IsStatic()){
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
			for(int i=0;i<fields.size();i++)
			{
				if(!fields[i]->IsStatic())
				{
					TClass* field_class=fields[i]->GetClass();
					if(field_class->HasCopyConstr())
					{
						ValidateAccess(fields[i].get(),this,field_class->GetCopyConstr());
						program.Push(TOp(TOpcode::PUSH_MEMBER_REF,fields[i]->GetOffset()),ops);
						program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,0),ops);
						program.Push(TOp(TOpcode::ADD_OFFSET,fields[i]->GetOffset()),ops);
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
			constructors.methods.push_back(std::unique_ptr<TMethod>(auto_copy_constr));
		}
	}

	{
		bool field_has_destr=false;
		bool parent_has_destr=parent.GetClass()==NULL?false:parent.GetClass()->HasDestr();
		for(int i=0;i<fields.size();i++){
			TMethod* field_destr=fields[i]->GetClass()->GetDestructor();
			if(field_destr!=NULL&&!field_destr->IsBytecode()&&!fields[i]->IsStatic()){
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
			for(int i=0;i<fields.size();i++)
			{
				if(!fields[i]->IsStatic())
				{
					TClass* field_class=fields[i]->GetClass();
					if(field_class->HasDestr())
					{
						ValidateAccess(fields[i].get(),this,field_class->GetDestructor());
						program.Push(TOp(TOpcode::PUSH_MEMBER_REF,fields[i]->GetOffset()),ops);
						ops+=field_class->GetDestructor()->BuildCall(program).GetOps();
					}
				}
			}
			program.Push(TOp(TOpcode::RETURN,1,0),ops);
			auto_destr = std::unique_ptr<TMethod>(new TMethod(this));
			auto_destr->SetAs(ops, NULL, false, false);
			auto_destr->CalcParamSize();
			if(destructor&&!destructor->IsBytecode())
				destructor->SetPostEvent(auto_destr.get());
		}
	}

	if(operators[TOperator::Assign].methods.size()==0)
	{
		//TODO оператор = (&left,right)
		//TODO у членов проверяется только оператор (&,&) нужно брать в расчет и (&,val)
		bool field_has_assign_op=false;
		TMethod* parent_assign_op=parent.GetClass()!=NULL?parent.GetClass()->GetBinOp(TOperator::Assign,parent.GetClass(),true,parent.GetClass(),true):NULL;
		//if(parent_assign_op!=NULL&&parent_assign_op->GetType()==MT_INTERNAL)parent_assign_op=NULL;
		bool parent_has_assign_op=parent.GetClass()==NULL?false:parent_assign_op!=NULL;
		for(int i=0;i<fields.size();i++){
			if(!fields[i]->IsStatic())
			{
				TClass* field_class=fields[i]->GetClass();//TODO проверять не является ли метод bytecode
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
			for(int i=0;i<fields.size();i++)
			{
				if(!fields[i]->IsStatic())
				{
					TClass* field_class=fields[i]->GetClass();

					program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,0),ops);
					program.Push(TOp(TOpcode::ADD_OFFSET,fields[i]->GetOffset()),ops);
					program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,1),ops);
					program.Push(TOp(TOpcode::ADD_OFFSET,fields[i]->GetOffset()),ops);
					TMethod* field_assign_op=field_class->GetBinOp(TOperator::Assign,field_class,true,field_class,true);
					if(field_assign_op!=NULL)
					{
						ValidateAccess(fields[i].get(),this,field_assign_op);
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
			operators[TOperator::Assign].methods.push_back(std::unique_ptr<TMethod>(method));
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

			operators[TOperator::Assign].methods.push_back(std::unique_ptr<TMethod>(op));
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

		operators[TOperator::Equal].methods.push_back(std::unique_ptr<TMethod>(op));
	}
	auto_methods_build=true;
	for(int i=0;i<nested_classes.size();i++)
		if(!nested_classes[i]->IsTemplate())
			nested_classes[i]->InitAutoMethods(program);
}