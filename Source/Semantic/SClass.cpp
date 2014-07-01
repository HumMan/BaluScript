#include "SClass.h"

#include "SOverloadedMethod.h"
#include "SType.h"

TSClass::TSClass(TSClass* use_owner, TTemplateRealizations* use_templates, TClass* use_syntax_node) :TSyntaxNode(use_syntax_node)
{
	templates = use_templates;
	is_sealed = false;

	auto_def_constr = NULL;
	auto_destr = NULL;

	owner = use_owner;
}

TTemplateRealizations* TSClass::GetTemplates(){
	return templates;
}

TSClass* TSClass::GetNested(TNameId name) {
	for (const std::shared_ptr<TSClass>& nested_class : nested_classes)
		if (nested_class->GetSyntax()->GetName() == name)
			return nested_class.get();
	return NULL;
}

void TSClass::CheckForErrors()
{
	if(owner!=NULL&&owner->GetOwner()!=NULL&&owner->GetOwner()->GetClass(GetSyntax()->GetName()))
		GetSyntax()->Error("Класс с таким именем уже существует!");//TODO как выводить ошибки если объекты были получены не из кода, а созданы вручную - исправить
	//for (const std::unique_ptr<TSClass> nested_class : nested_classes)
	for(int i=0;i<nested_classes.size();i++)
	{
		for(int k=0;k<i;k++)
		{
			if (nested_classes[i]->GetSyntax()->GetName() == nested_classes[k]->GetSyntax()->GetName())
				nested_classes[i]->GetSyntax()->Error("Класс с таким именем уже существует!");
		}
	}
	for (TSClassField& field : fields)
	{
		for (TSClassField& other_field : fields)
		//for(int k=0;k<i;k++)
		{
			if (&field == &other_field)
				break;
			if(owner==NULL&&!field.GetSyntax()->IsStatic())
				GetSyntax()->Error("Базовый класс может содержать только статические поля!");
			if (field.GetSyntax()->GetName() == other_field.GetSyntax()->GetName())
				field.GetSyntax()->Error("Поле класса с таким именем уже существует!");
			//TODO как быть со статическими членами класса
		}
		std::vector<TSMethod*> m;
		if ((owner != NULL&&owner->GetField(GetSyntax()->GetName(), true, false) != NULL) || GetMethods(m, GetSyntax()->GetName()))
			field.GetSyntax()->Error("Член класса с таким имененем уже существует!");
	}
	for (TSOverloadedMethod& method : methods)
	{
		if((owner!=NULL&&owner->GetField(method.GetName(),true,false)!=NULL))
			method.methods[0]->GetSyntax()->Error("Статическое поле класса с таким имененем уже существует!");
		method.CheckForErrors();
		std::vector<TSMethod*> owner_methods;
		if(owner!=NULL&&owner->GetMethods(owner_methods,method.GetName()))
		{
			for(int k=0;k<owner_methods.size();k++)
			{
				TSMethod* temp=method.FindParams(owner_methods[k]);
				if(temp!=NULL)
					temp->GetSyntax()->Error("Статический метод с такими параметрами уже существует!");
			}
		}
	}
	constructors.CheckForErrors();
	conversions.CheckForErrors(true);
	for(int i=0;i<TOperator::End;i++)
	{
		operators[i].CheckForErrors();
	}
	//for(int i=0;i<nested_classes.size();i++)
	//	if(!nested_classes[i]->IsTemplate())
	//		nested_classes[i]->DeclareMethods();
}

TSClass* TSClass::GetClass(TNameId use_name)
{
	if (GetSyntax()->GetName() == use_name)
		return this;
	for (const std::shared_ptr<TSClass>& nested_class : nested_classes)
	{
		if (nested_class->GetSyntax()->GetName() == use_name)
			return nested_class.get();
	}

	//assert(!is_template);
	if (!GetSyntax()->IsTemplate())
	{
		for (TSClass* template_param : template_params)
			if (template_param->GetSyntax()->GetName() == use_name)
				return template_param;
	}
	if (owner != NULL)
		return owner->GetClass(use_name);
	return NULL;
}

TSClassField* TSClass::GetField(TNameId name, bool only_in_this)
{
	TSClassField* result_ns = GetField(name, false, only_in_this);
	TSClassField* result_s = GetField(name, true, only_in_this);
	if (result_ns != NULL)
		return result_ns;
	else
		return result_s;
}

TSClassField* TSClass::GetField(TNameId name, bool is_static, bool only_in_this)
{
	TSClassField* result_parent = NULL;
	if (parent != NULL)
		result_parent = parent->GetField(name, true);
	if (result_parent != NULL)
		return result_parent;
	for (TSClassField& field : fields)
	{
		if (field.GetSyntax()->IsStatic() == is_static && field.GetSyntax()->GetName() == name) {
			return &field;
		}
	}
	if (!only_in_this && is_static && owner != NULL)
		return owner->GetField(name, true, false);
	return NULL;
}

void TSClass::Link()
{
	//проверить поля класса - т.к. классы полей не могут содержать поля текущего класса, он учавствует лишь в параметрах методов и локальных переменных

	//определить присутствие конструктора по умолчанию, деструктора, конструктора копии

	//слинковать методы

	//слинковать тела методов - требуется наличие инф. обо всех методах, conversion, операторах класса
}