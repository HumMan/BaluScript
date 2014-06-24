
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
