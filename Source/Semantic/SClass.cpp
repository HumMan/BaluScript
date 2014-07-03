#include "SClass.h"

#include <assert.h>

#include "SOverloadedMethod.h"
#include "SType.h"

TSClass::TSClass(TSClass* use_owner, TTemplateRealizations* use_templates, TClass* use_syntax_node) :TSyntaxNode(use_syntax_node)
{
	templates = use_templates;
	is_sealed = false;

	auto_def_constr = NULL;
	auto_destr = NULL;

	owner = use_owner;
	linked = false;
}

TSClass* TSClass::GetOwner()
{
	return owner;
}

void TSClass::Build()
{
	for (TClassField& field_syntax : GetSyntax()->fields)
	{
		fields.emplace_back(this, &field_syntax);
	}

	for (TOverloadedMethod& method : GetSyntax()->methods)
	{
		methods.emplace_back(this, &method);
		methods.back().Build();
	}
	constructors = std::shared_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, &GetSyntax()->constructors));
	constructors->Build();

	if (GetSyntax()->destructor)
	{
		destructor = std::shared_ptr<TSMethod>(new TSMethod(this, GetSyntax()->destructor.get()));
		destructor->Build();
	}

	for (int i = 0; i < TOperator::End; i++)
	{
		operators[i] = std::shared_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, &GetSyntax()->operators[i]));
		operators[i]->Build();
	}

	conversions = std::shared_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, &GetSyntax()->conversions));
	conversions->Build();

	for (const std::unique_ptr<TClass>& nested_class : GetSyntax()->nested_classes)
	{
		nested_classes.push_back(std::shared_ptr<TSClass>(new TSClass(this, templates, nested_class.get())));
		nested_classes.back()->Build();
	}
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
	//constructors.CheckForErrors();
	//conversions.CheckForErrors(true);
	//for(int i=0;i<TOperator::End;i++)
	//{
	//	operators[i].CheckForErrors();
	//}
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

	assert(!GetSyntax()->IsTemplate());
	if (linked)
		return;
	linked = true;



	for (TSOverloadedMethod& method : methods)
	{
		method.LinkSignature();
	}
	constructors->LinkSignature();
	if (destructor)
		destructor->LinkSignature();

	for (int i = 0; i < TOperator::End; i++)
		operators[i]->LinkSignature();
	conversions->LinkSignature();

	for (const std::shared_ptr<TSClass>& nested_class : nested_classes)
		if (!nested_class->GetSyntax()->IsTemplate())
			nested_class->Link();

	for (TSOverloadedMethod& method : methods)
	{
		method.LinkBody();
	}
	constructors->LinkBody();
	if (destructor)
		destructor->LinkBody();

	for (int i = 0; i < TOperator::End; i++)
		operators[i]->LinkBody();
	conversions->LinkBody();
}


bool TSClass::GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name) 
{
	assert(linked);
	for (TSOverloadedMethod& ov_method : methods)
	{
		if (ov_method.GetName() == use_method_name)
		{
			for (const std::shared_ptr<TSMethod>& method : ov_method.methods)
				result.push_back(method.get());
		}
	}
	if (owner != NULL)
		owner->GetMethods(result, use_method_name, true);
	if (parent != NULL)
		parent->GetMethods(result, use_method_name);
	return result.size() > 0;
}

bool TSClass::GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name, bool is_static) 
{
	assert(linked);
	for (TSOverloadedMethod& ov_method : methods)
	{
		if (ov_method.GetName() == use_method_name)
		{
			for (const std::shared_ptr<TSMethod>& method : ov_method.methods)
				if (method->GetSyntax()->IsStatic() == is_static)
					result.push_back(method.get());
		}
	}
	if (is_static && owner != NULL)
		owner->GetMethods(result, use_method_name, true);
	if (parent != NULL)
		parent->GetMethods(result, use_method_name, is_static);
	return result.size() > 0;
}

TSMethod* TSClass::GetConversion(bool source_ref, TSClass* target_type) 
{
	assert(linked);
	for (const std::shared_ptr<TSMethod>& conversion : conversions->methods)
	{
		if (conversion->GetRetClass() == target_type
			&& conversion->GetParam(0)->GetSyntax()->IsRef() == source_ref) {
			return conversion.get();
		}
	}
	return NULL;
}


bool TSClass::GetConstructors(std::vector<TSMethod*> &result) 
{
	//assert(methods_declared&&auto_methods_build);
	for (const std::shared_ptr<TSMethod>& constructor : constructors->methods)
	{
		result.push_back(constructor.get());
	}

	if (!constr_override && auto_def_constr)
		result.push_back(auto_def_constr.get());
	return result.size() > 0;
}

TSMethod* TSClass::GetDefConstr()
{
	//assert(methods_declared&&auto_methods_build);
	if (constr_override)
	{
		for (const std::shared_ptr<TSMethod>& constructor : constructors->methods)
			if (constructor->GetParamsCount() == 0)
				return constructor.get();
	}
	else
		return auto_def_constr.get();
	return NULL;
}

TSMethod* TSClass::GetCopyConstr()
{
	//assert(methods_declared);
	for (const std::shared_ptr<TSMethod>& constructor : constructors->methods)
	{
		if (constructor->GetParamsCount() == 1
			&& constructor->GetParam(0)->GetClass() == this
			&& constructor->GetParam(0)->GetSyntax()->IsRef() == true) {
			return constructor.get();
		}
	}
	return NULL;
}

TSMethod* TSClass::GetDestructor()
{
	//assert(methods_declared&&auto_methods_build);
	return (destructor) ? destructor.get() : auto_destr.get();
}

bool TSClass::HasConversion(TSClass* target_type)
{
	return GetConversion(true, target_type)
		|| GetConversion(false, target_type);
}

bool TSClass::IsNestedIn(TSClass* use_parent)
{
	if (parent == NULL)
		return false;
	if (parent == use_parent)
		return true;
	return parent->IsNestedIn(use_parent);
}
bool TSClass::GetOperators(std::vector<TSMethod*> &result, TOperator::Enum op)
{
	//assert(methods_declared&&auto_methods_build);
	operators[op]->GetMethods(result);
	return result.size() > 0;
}