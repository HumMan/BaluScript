#include "SClass.h"

#include <assert.h>

#include "SOverloadedMethod.h"
#include "SType.h"
#include "SStatements.h"

#include "../Syntax/Statements.h"

TSClass::TSClass(TSClass* use_owner, TClass* use_syntax_node, TNodeWithTemplates::Type type) :TSyntaxNode(use_syntax_node)
{
	if (type == TNodeWithTemplates::Unknown)
	{
		if (use_syntax_node->IsTemplate())
			SetType(TNodeWithTemplates::Template);
		else
			SetType(TNodeWithTemplates::Class);
	}
	else
		SetType(type);
	is_sealed = false;

	//auto_def_constr = NULL;
	//auto_destr = NULL;
	parent = NULL;
	owner = use_owner;
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
	constructors = std::unique_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, &GetSyntax()->constructors));
	constructors->Build();

	if (GetSyntax()->destructor)
	{
		destructor = std::unique_ptr<TSMethod>(new TSMethod(this, GetSyntax()->destructor.get()));
		destructor->Build();
	}

	for (int i = 0; i < TOperator::End; i++)
	{
		operators[i] = std::unique_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, &GetSyntax()->operators[i]));
		operators[i]->Build();
	}

	conversions = std::unique_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, &GetSyntax()->conversions));
	conversions->Build();

	for (const std::unique_ptr<TClass>& nested_class : GetSyntax()->nested_classes)
	{
		nested_classes.push_back(std::unique_ptr<TSClass>(new TSClass(this, nested_class.get())));
		nested_classes.back()->Build();
	}
}

TSClass* TSClass::GetNested(TNameId name) {
	for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
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
	//мы должны возвращать шаблонный класс, а не его реализацию
	if (GetType() == TNodeWithTemplates::Class || GetType() == TNodeWithTemplates::Template)
	{
		if (GetSyntax()->GetName() == use_name)
			return this;
		for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
		{
			if (nested_class->GetSyntax()->GetName() == use_name)
				return nested_class.get();
		}
	}
	
	if (GetType() == TNodeWithTemplates::Realization)
	{
		std::vector<TSClass*> template_params = GetTemplateParams();
		for (int i = 0; i < template_params.size(); i++)
			if (GetTemplateClass()->GetSyntax()->template_params[i] == use_name)
				return template_params[i];
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

void TSClass::LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	if (!IsSignatureLinked())
		SetSignatureLinked();
	else
		return;

	//определить присутствие конструктора по умолчанию, деструктора, конструктора копии
	
	for (TSClassField& field : fields)
	{
		field.LinkSignature(static_fields, static_variables);
		if (field.GetSyntax()->IsStatic())
			static_fields->push_back(&fields.back());
	}

	//слинковать сигнатуры методов
	for (TSOverloadedMethod& method : methods)
	{
		method.LinkSignature(static_fields, static_variables);
	}
	constructors->LinkSignature(static_fields, static_variables);
	if (destructor)
		destructor->LinkSignature(static_fields, static_variables);

	for (int i = 0; i < TOperator::End; i++)
		operators[i]->LinkSignature(static_fields, static_variables);
	conversions->LinkSignature(static_fields, static_variables);

	for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
		if (!nested_class->GetSyntax()->IsTemplate())
			nested_class->LinkSignature(static_fields, static_variables);
}

void TSClass::LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	if (!IsBodyLinked())
		SetBodyLinked();
	else
		return;

	CheckForErrors();

	for (TSClassField& field : fields)
	{
		field.LinkBody(static_fields, static_variables);
	}

	//слинковать тела методов - требуется наличие инф. обо всех методах, conversion, операторах класса
	for (TSOverloadedMethod& method : methods)
	{
		method.LinkBody(static_fields, static_variables);
	}
	if (constructors)
		constructors->LinkBody(static_fields, static_variables);
	if (destructor)
		destructor->LinkBody(static_fields, static_variables);

	for (int i = 0; i < TOperator::End; i++)
		if (operators[i])
			operators[i]->LinkBody(static_fields, static_variables);
	conversions->LinkBody(static_fields, static_variables);

	for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
		if (!nested_class->GetSyntax()->IsTemplate())
			nested_class->LinkBody(static_fields, static_variables);
}


bool TSClass::GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name) 
{
	assert(IsSignatureLinked());
	for (TSOverloadedMethod& ov_method : methods)
	{
		if (ov_method.GetName() == use_method_name)
		{
			for (const std::unique_ptr<TSMethod>& method : ov_method.methods)
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
	assert(IsSignatureLinked());
	for (TSOverloadedMethod& ov_method : methods)
	{
		if (ov_method.GetName() == use_method_name)
		{
			for (const std::unique_ptr<TSMethod>& method : ov_method.methods)
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
	assert(IsSignatureLinked());
	for (const std::unique_ptr<TSMethod>& conversion : conversions->methods)
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
	for (const std::unique_ptr<TSMethod>& constructor : constructors->methods)
	{
		result.push_back(constructor.get());
	}

	//if (!constr_override && auto_def_constr)
	//	result.push_back(auto_def_constr.get());
	return result.size() > 0;
}

TSMethod* TSClass::GetDefConstr()
{
	//assert(methods_declared&&auto_methods_build);
	//if (constr_override)
	{
		for (const std::unique_ptr<TSMethod>& constructor : constructors->methods)
			if (constructor->GetParamsCount() == 0)
				return constructor.get();
	}
	//else
	//	return auto_def_constr.get();
	return NULL;
}

TSMethod* TSClass::GetCopyConstr()
{
	//assert(methods_declared);
	for (const std::unique_ptr<TSMethod>& constructor : constructors->methods)
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
	//return (destructor) ? destructor.get() : auto_destr.get();
	return destructor.get();
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

void TSClass::CreateInternalClasses() 
{
	TClass* t_syntax = new TClass(GetSyntax());
	t_syntax->name = GetSyntax()->source->GetIdFromName("dword");
	TSClass* t = new TSClass(this,t_syntax);
	t->SetSize(1);
	t->SetSignatureLinked();
	t->SetBodyLinked();
	nested_classes.push_back(std::unique_ptr<TSClass>(t));
}

void TSClass::CalculateSizes(std::vector<TSClass*> &owners) 
{
	if (GetType() != TNodeWithTemplates::Template)
	{
		if (IsSizeInitialized())
			return;
		int class_size = 0;
		if (std::find(owners.begin(), owners.end(), this) != owners.end())
		{
			//Error(" ласс не может содержать поле с собственным типом(кроме дин. массивов)!");//TODO см. initautomethods дл¤ дин массивов
			GetSyntax()->Error("Класс не может содержать поле собственного типа!");
		}
		else {
			owners.push_back(this);
			if (parent != NULL)
			{
				TSClass* parent_class = parent;
				parent_class->CalculateSizes(owners);
				class_size = parent_class->GetSize();

				do {
					//TODO в методы по умолчанию
					/*
					//добавл¤ем приведение в родительские классы
					TMethod* temp = new TMethod(this, TClassMember::Conversion);
					temp->SetAs(TOpArray(), parent_class, true, true, 1);

					TParameter* t = new TParameter(this, temp);
					t->SetAs(true, this);
					temp->AddParam(t);
					temp->CalcParamSize();
					AddConversion(temp);*/

					parent_class = parent_class->GetParent();
				} while (parent_class != NULL);
			}
			for (TSClassField& field : fields)
			{
				field.GetClass()->CalculateSizes(owners);
				if (!field.GetSyntax()->IsStatic())
				{
					field.SetOffset(class_size);
					class_size += field.GetClass()->GetSize();
				}
			}
		}
		owners.pop_back();
		SetSize(class_size);

		for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
			nested_class->CalculateSizes(owners);
	}
	if (GetType() == TNodeWithTemplates::Template)
		for (TSClass* realization : GetRealizations())
			realization->CalculateSizes(owners);
}

void TSClass::CalculateMethodsSizes()
{
	if (GetType() == TNodeWithTemplates::Template)
		return;

	for (TSOverloadedMethod& method : methods)
	{
		method.CalculateParametersOffsets();
	}
	if (constructors)
		constructors->CalculateParametersOffsets();
	if (destructor)
		destructor->CalculateParametersOffsets();

	for (int i = 0; i < TOperator::End; i++)
		if (operators[i])
			operators[i]->CalculateParametersOffsets();
	if (conversions)
		conversions->CalculateParametersOffsets();

	for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
		nested_class->CalculateMethodsSizes();

	if (GetType() == TNodeWithTemplates::Template)
		for (TSClass* realization : GetRealizations())
			realization->CalculateMethodsSizes();
}