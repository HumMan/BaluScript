﻿#include "SClass.h"

#include <assert.h>

#include "SOverloadedMethod.h"
#include "SType.h"
#include "SStatements.h"

#include "../semanticAnalyzer.h"

#include "../Syntax/Statements.h"
#include "../Syntax/ClassField.h"
#include "../Syntax/Method.h"

TSClass::TSClass(TSClass* use_owner, TClass* use_syntax_node, TNodeWithTemplates::Type type) :TSyntaxNode(use_syntax_node), parent(this,&use_syntax_node->parent)
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

	owner = use_owner;
}

TSClass* TSClass::GetOwner()
{
	return owner;
}

TSClass* TSClass::GetNestedByFullName(std::vector<TNameId> full_name, int curr_id)
{
	if (curr_id == full_name.size())
		return this;
	TNameId curr = full_name[curr_id];
	TSClass* child = GetNested(full_name[curr_id]);
	if (child!=NULL)
	{
		if (curr_id + 1 == full_name.size())
			return child;
		else
			return child->GetNestedByFullName(full_name, curr_id+1);
	}
	else
		return NULL;
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

	if (GetSyntax()->constr_default)
	{
		default_constructor = std::unique_ptr<TSMethod>(new TSMethod(this, GetSyntax()->constr_default.get()));
		default_constructor->Build();
	}

	copy_constructors = std::unique_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, &GetSyntax()->constr_copy));
	copy_constructors->Build();

	move_constructors = std::unique_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, &GetSyntax()->constr_move));
	move_constructors->Build();

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

bool TSClass::GetTemplateParameter(TNameId name, TNodeWithTemplates::TTemplateParameter& result)
{
	assert(GetType() == TNodeWithTemplates::Realization);
	int par_count = GetTemplateClass()->GetSyntax()->GetTemplateParamsCount();
	int i = 0;
	for (TNameId& n : GetTemplateClass()->GetSyntax()->template_params)
	{
		if (n == name)
		{
			result = GetTemplateParam(i);
			return true;
		}
		i++;
	}
	return false;
	assert(false);
}

bool TSClass::HasTemplateParameter(TNameId name)
{
	assert(GetType() == TNodeWithTemplates::Realization);
	int par_count = GetTemplateClass()->GetSyntax()->GetTemplateParamsCount();
	int i = 0;
	for (TNameId& n : GetTemplateClass()->GetSyntax()->template_params)
	{
		if (n == name)
		{
			return true;
		}
		i++;
	}
	return false;
}

void TSClass::CheckForErrors()
{
	if (owner != NULL&&owner->GetOwner() != NULL&&owner->GetOwner()->GetClass(GetSyntax()->GetName()))
		GetSyntax()->Error("Класс с таким именем уже существует!");//TODO как выводить ошибки если объекты были получены не из кода, а созданы вручную - исправить
	//for (const std::unique_ptr<TSClass> nested_class : nested_classes)
	for (size_t i = 0; i < nested_classes.size(); i++)
	{
		for (size_t k = 0; k < i; k++)
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
			if (owner == NULL&&!field.GetSyntax()->IsStatic())
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
		if ((owner != NULL&&owner->GetField(method.GetName(), true, false) != NULL))
			method.methods[0]->GetSyntax()->Error("Статическое поле класса с таким имененем уже существует!");
		method.CheckForErrors();
		std::vector<TSMethod*> owner_methods;
		if (owner != NULL&&owner->GetMethods(owner_methods, method.GetName()))
		{
			for (size_t k = 0; k < owner_methods.size(); k++)
			{
				TSMethod* temp = method.FindParams(owner_methods[k]);
				if (temp != NULL)
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

	//если реализация класса, то не забываем проверить не является ли идентификатор одним из параметров класса
	if (GetType() == TNodeWithTemplates::Realization)
	{
		std::vector<TNodeWithTemplates::TTemplateParameter> template_params = GetTemplateParams();
		for (size_t i = 0; i < template_params.size(); i++)
			if (!template_params[i].is_value)
				if (GetTemplateClass()->GetSyntax()->template_params[i] == use_name)
					return template_params[i].type;
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
	if (parent.GetClass() != NULL)
		result_parent = parent.GetClass()->GetField(name, true);
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

void TSClass::LinkSignature(TGlobalBuildContext build_context)
{
	if (!IsSignatureLinked())
		SetSignatureLinked();
	else
		return;


	parent.LinkSignature(build_context);
	//определить присутствие конструктора по умолчанию, деструктора, конструктора копии

	for (TSClassField& field : fields)
	{
		field.LinkSignature(build_context);
		if (field.GetSyntax()->IsStatic())
			build_context.static_fields->push_back(&field);
	}

	//слинковать сигнатуры методов
	for (TSOverloadedMethod& method : methods)
	{
		method.LinkSignature(build_context);
	}

	if (default_constructor)
		default_constructor->LinkSignature(build_context);
	if (copy_constructors)
		copy_constructors->LinkSignature(build_context);
	if (move_constructors)
		move_constructors->LinkSignature(build_context);
	if (destructor)
		destructor->LinkSignature(build_context);

	for (int i = 0; i < TOperator::End; i++)
		operators[i]->LinkSignature(build_context);
	conversions->LinkSignature(build_context);

	for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
		if (!nested_class->GetSyntax()->IsTemplate())
			nested_class->LinkSignature(build_context);
}

void TSClass::LinkBody(TGlobalBuildContext build_context)
{
	if (!IsBodyLinked())
		SetBodyLinked();
	else
		return;

	CheckForErrors();

	parent.LinkBody(build_context);

	for (TSClassField& field : fields)
	{
		field.LinkBody(build_context);
	}

	//слинковать тела методов - требуется наличие инф. обо всех методах, conversion, операторах класса
	for (TSOverloadedMethod& method : methods)
	{
		method.LinkBody(build_context);
	}
	if (default_constructor)
		default_constructor->LinkBody(build_context);
	if (copy_constructors)
		copy_constructors->LinkBody(build_context);
	if (move_constructors)
		move_constructors->LinkBody(build_context);
	if (destructor)
		destructor->LinkBody(build_context);

	for (int i = 0; i < TOperator::End; i++)
		if (operators[i])
			operators[i]->LinkBody(build_context);
	conversions->LinkBody(build_context);

	for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
		if (!nested_class->GetSyntax()->IsTemplate())
			nested_class->LinkBody(build_context);
}


bool TSClass::GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name)
{
	//assert(IsSignatureLinked());
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
	if (parent.GetClass() != NULL)
		parent.GetClass()->GetMethods(result, use_method_name);
	return result.size() > 0;
}

bool TSClass::GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name, bool is_static)
{
	//assert(IsSignatureLinked());
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
	if (parent.GetClass() != NULL)
		parent.GetClass()->GetMethods(result, use_method_name, is_static);
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

bool TSClass::GetCopyConstructors(std::vector<TSMethod*> &result)
{
	assert(IsAutoMethodsInitialized());
	for (const std::unique_ptr<TSMethod>& constructor : copy_constructors->methods)
	{
		result.push_back(constructor.get());
	}
	if (auto_copy_constr)
		result.push_back(auto_copy_constr.get());
	return result.size() > 0;
}


bool TSClass::GetMoveConstructors(std::vector<TSMethod*> &result)
{
	assert(IsAutoMethodsInitialized());
	for (const std::unique_ptr<TSMethod>& constructor : move_constructors->methods)
	{
		result.push_back(constructor.get());
	}

	if (auto_move_constr)
		result.push_back(auto_move_constr.get());
	return result.size() > 0;
}

TSMethod* TSClass::GetDefConstr()
{
	assert(IsAutoMethodsInitialized());
	if (default_constructor)
		return default_constructor.get();
	if (auto_def_constr)
		return auto_def_constr.get();
	return NULL;
}

TSMethod* TSClass::GetCopyConstr()
{
	assert(IsAutoMethodsInitialized());
	if (copy_constructors)
	{
		for (const std::unique_ptr<TSMethod>& constructor : copy_constructors->methods)
		{
			if (constructor->GetParamsCount() == 1
				&& constructor->GetParam(0)->GetClass() == this
				&& constructor->GetParam(0)->GetSyntax()->IsRef() == true) {
				return constructor.get();
			}
		}
	}
	if (auto_copy_constr)
		return auto_copy_constr.get();
	return NULL;
}

TSMethod* TSClass::GetMoveConstr()
{
	assert(IsAutoMethodsInitialized());
	if (move_constructors)
	{
		for (const std::unique_ptr<TSMethod>& constructor : move_constructors->methods)
		{
			if (constructor->GetParamsCount() == 1
				&& constructor->GetParam(0)->GetClass() == this
				&& constructor->GetParam(0)->GetSyntax()->IsRef() == true) {
				return constructor.get();
			}
		}
	}
	if (auto_move_constr)
		return auto_copy_constr.get();
	return NULL;
}

TSMethod* TSClass::GetAssignOperator()
{
	assert(IsAutoMethodsInitialized());
	if (operators[TOperator::Assign])
	{
		for (const std::unique_ptr<TSMethod>& assign_op : operators[TOperator::Assign]->methods)
		{
			if (assign_op->GetParamsCount() == 2
				&& assign_op->GetParam(0)->GetClass() == this
				&& assign_op->GetParam(0)->GetSyntax()->IsRef() == true
				&& assign_op->GetParam(1)->GetClass() == this
				&& assign_op->GetParam(1)->GetSyntax()->IsRef() == true) {
				return assign_op.get();
			}
		}
	}
	if (auto_assign_operator)
		return auto_assign_operator.get();
	return NULL;
}

TSMethod* TSClass::GetDestructor()
{
	assert(IsAutoMethodsInitialized());
	if (destructor)
		return destructor.get();
	if (auto_destr)
		return auto_destr.get();
	return NULL;
}

bool TSClass::HasConversion(TSClass* target_type)
{
	return GetConversion(true, target_type)
		|| GetConversion(false, target_type);
}

bool TSClass::IsNestedIn(TSClass* use_parent)
{
	if (parent.GetClass() == NULL)
		return false;
	if (parent.GetClass() == use_parent)
		return true;
	return parent.GetClass()->IsNestedIn(use_parent);
}
bool TSClass::GetOperators(std::vector<TSMethod*> &result, TOperator::Enum op)
{
	assert(IsAutoMethodsInitialized());
	operators[op]->GetMethods(result);
	if (result.size() == 0 && op == TOperator::Assign)
		if (auto_assign_operator)
			result.push_back(auto_assign_operator.get());
	return result.size() > 0;
}

void TSClass::AddClass(TSClass* use_class)
{
	nested_classes.push_back(std::unique_ptr<TSClass>(use_class));
}

void TSClass::CopyExternalMethodBindingsFrom(TSClass* source)
{
	//слинковать сигнатуры методов
	auto i = methods.begin();
	auto k = source->methods.begin();
	while( i != methods.end())
	{
		i->CopyExternalMethodBindingsFrom(&(*k));
		i++;
		k++;
	}
	if (default_constructor)
		default_constructor->CopyExternalMethodBindingsFrom(source->default_constructor.get());
	if (copy_constructors)
		copy_constructors->CopyExternalMethodBindingsFrom(source->copy_constructors.get());
	if (move_constructors)
		move_constructors->CopyExternalMethodBindingsFrom(source->move_constructors.get());
	if (destructor)
		destructor->CopyExternalMethodBindingsFrom(source->destructor.get());

	for (int i = 0; i < TOperator::End; i++)
		operators[i]->CopyExternalMethodBindingsFrom(source->operators[i].get());
	conversions->CopyExternalMethodBindingsFrom(source->conversions.get());

	//assert(nested_classes.size() == 0); //внешние классы с вложенными классами не допускаются
	//TODO разобраться
}

void TSClass::CalculateSizes(std::vector<TSClass*> &owners)
{
	if (GetSyntax()->IsEnumeration())
	{
		if (!IsSizeInitialized())
		{
			SetSize(1);
		}
	}
	else
	{
		if (GetType() != TNodeWithTemplates::Template)
		{
			if (!IsSizeInitialized())
			{
				int class_size = 0;
				if (std::find(owners.begin(), owners.end(), this) != owners.end())
				{
					//Error(" ласс не может содержать поле с собственным типом(кроме дин. массивов)!");//TODO см. initautomethods дл¤ дин массивов
					GetSyntax()->Error("Класс не может содержать поле собственного типа!");
				}
				else {
					owners.push_back(this);
					if (parent.GetClass() != NULL)
					{
						TSClass* parent_class = parent.GetClass();
						parent_class->CalculateSizes(owners);
						class_size += parent_class->GetSize();

						do {
							//TODO в методы по умолчанию
							/*
							//добавляем приведение в родительские классы
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
							if (field.GetSyntax()->HasSizeMultiplierId())
							{
								TNameId multiplier_id = field.GetSyntax()->GetFactorId();
								int multiplier = this->FindTemplateIntParameter(multiplier_id);
								field.SetSizeMultiplier(multiplier);
							}
							else if (field.GetSyntax()->HasSizeMultiplier())
							{
								int multiplier = field.GetSyntax()->GetSizeMultiplier();
								field.SetSizeMultiplier(multiplier);
							}

							if (field.HasSizeMultiplier())
								class_size += field.GetClass()->GetSize()*field.GetSizeMultiplier();
							else
								class_size += field.GetClass()->GetSize();
						}
					}
				}
				owners.pop_back();
				SetSize(class_size);
			}
			for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
				nested_class->CalculateSizes(owners);
		}
		if (GetType() == TNodeWithTemplates::Template)
			for (const std::unique_ptr<TSClass>& realization : GetRealizations())
				realization->CalculateSizes(owners);
	}
}

void TSClass::CalculateMethodsSizes()
{
	if (GetType() != TNodeWithTemplates::Template)
	{

		for (TSOverloadedMethod& method : methods)
		{
			method.CalculateParametersOffsets();
		}
		if (default_constructor)
			default_constructor->CalculateParametersOffsets();
		if (copy_constructors)
			copy_constructors->CalculateParametersOffsets();
		if (move_constructors)
			move_constructors->CalculateParametersOffsets();
		if (destructor)
			destructor->CalculateParametersOffsets();

		for (int i = 0; i < TOperator::End; i++)
			if (operators[i])
				operators[i]->CalculateParametersOffsets();
		if (conversions)
			conversions->CalculateParametersOffsets();

		for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
			nested_class->CalculateMethodsSizes();
	}
	if (GetType() == TNodeWithTemplates::Template)
		for (const std::unique_ptr<TSClass>& realization : GetRealizations())
			realization->CalculateMethodsSizes();
}

void TSClass::InitAutoMethods()
{
	if (GetType() == TNodeWithTemplates::Template)
		return;
	if (IsAutoMethodsInitialized())
		return;

	for (TSClassField& field : fields)
	{
		field.GetClass()->InitAutoMethods();
	}

	bool has_def_constr = false;
	bool has_copy_constr = false;
	bool has_destr = false;
	bool has_assign_op = false;

	for (TSClassField& field : fields)
	{
		TSClass* field_class = field.GetClass();
		if (field_class->GetDefConstr() != NULL)
			has_def_constr = true;
		if (field_class->GetCopyConstr() != NULL)
			has_copy_constr = true;
		if (field_class->GetDestructor() != NULL)
			has_destr = true;
		if (field_class->GetAssignOperator() != NULL)
			has_assign_op = true;
	}

	//TODO проверка наследуемых классов на наличие конструктора, деструктора и т.д.

	assert(!auto_def_constr);
	assert(!auto_copy_constr);
	assert(!auto_destr);
	assert(!auto_assign_operator);

	if (has_def_constr)
	{
		auto_def_constr.reset(new TSMethod(this, TSpecialClassMethod::AutoDefConstr));
	}
	//if (has_copy_constr)
	{
		auto_copy_constr.reset(new TSMethod(this, TSpecialClassMethod::AutoCopyConstr));
		TSParameter* p = new TSParameter(this, auto_copy_constr.get(), this, true);
		auto_copy_constr->AddParameter(p);
		
	}
	if (has_destr)
	{
		auto_destr.reset(new TSMethod(this, TSpecialClassMethod::AutoDestructor));
	}


	//if (has_assign_op)
	{
		auto_assign_operator.reset(new TSMethod(this, TSpecialClassMethod::AutoAssignOperator));
		TSParameter* p = new TSParameter(this, auto_assign_operator.get(), this, true);
		auto_assign_operator->AddParameter(p);
		p = new TSParameter(this, auto_assign_operator.get(), this, true);
		auto_assign_operator->AddParameter(p);
	}

	SetAutoMethodsInitialized();

	for (const std::unique_ptr<TSClass>& nested_class : nested_classes)
		nested_class->InitAutoMethods();

	if (GetType() == TNodeWithTemplates::Template)
		for (const std::unique_ptr<TSClass>& realization : GetRealizations())
			realization->InitAutoMethods();
}

void TSClass::RunAutoDefConstr(std::vector<TStaticValue> &static_fields, TStackValue& object)
{
	assert(IsAutoMethodsInitialized());
	assert(auto_def_constr);

	//bool field_has_def_constr = false;
	//bool parent_has_def_constr = parent.GetClass() == NULL ? false : parent.GetClass()->HasDefConstr();

	std::vector<TStackValue> formal_params;
	TStackValue result;

	for (TSClassField& field : fields)
	{
		assert(field.GetClass()->IsAutoMethodsInitialized());
		TSMethod* field_def_constr = field.GetClass()->GetDefConstr();
		if (field_def_constr != NULL&& !field.GetSyntax()->IsStatic())
		{
			TSClass* field_class = field.GetClass();

			ValidateAccess(field.GetSyntax(), this, field_def_constr);
			TStackValue field_object(true, field_class);
			if (field.HasSizeMultiplier())
			{
				for (int i = 0; i < field.GetSizeMultiplier(); i++)
				{
					field_object.SetAsReference(&(((int*)object.get())[field.GetOffset()+i*field.GetClass()->GetSize()]));
					field_def_constr->Run(TMethodRunContext(&static_fields, &formal_params, &result, &field_object));
				}
			}
			else
			{
				field_object.SetAsReference(&(((int*)object.get())[field.GetOffset()]));
				field_def_constr->Run(TMethodRunContext(&static_fields, &formal_params, &result, &field_object));
			}
		}
	}
}

void TSClass::RunAutoDestr(std::vector<TStaticValue> &static_fields, TStackValue& object)
{
	assert(IsAutoMethodsInitialized());
	assert(auto_destr);

	std::vector<TStackValue> formal_params;
	TStackValue result;

	for (TSClassField& field : fields)
	{
		assert(field.GetClass()->IsAutoMethodsInitialized());
		TSMethod* field_destr = field.GetClass()->GetDestructor();
		if (field_destr != NULL&& !field.GetSyntax()->IsStatic())
		{
			TSClass* field_class = field.GetClass();

			ValidateAccess(field.GetSyntax(), this, field_destr);
			TStackValue field_object(true, field_class);
			if (field.HasSizeMultiplier())
			{
				for (int i = 0; i < field.GetSizeMultiplier(); i++)
				{
					field_object.SetAsReference(&(((int*)object.get())[field.GetOffset() + i*field.GetClass()->GetSize()]));
					field_destr->Run(TMethodRunContext(&static_fields, &formal_params, &result, &field_object));
				}
			}
			else
			{
				field_object.SetAsReference(&(((int*)object.get())[field.GetOffset()]));
				field_destr->Run(TMethodRunContext(&static_fields, &formal_params, &result, &field_object));
			}
		}
	}
}

void TSClass::RunAutoCopyConstr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& object)
{
	assert(IsAutoMethodsInitialized());
	assert(auto_copy_constr);

	//конструктор копии должен принимать один аргумент (кроме ссылки на объект) с тем же типом что и данный класс
	assert(formal_params.size() == 1);
	assert(formal_params[0].GetClass() == this);
	
	TStackValue result;

	if (fields.size() > 0)
	{

		for (TSClassField& field : fields)
		{
			assert(field.GetClass()->IsAutoMethodsInitialized());
			TSMethod* field_copy_constr = field.GetClass()->GetCopyConstr();
			if (!field.GetSyntax()->IsStatic())
			{
				TSClass* field_class = field.GetClass();
				//если у поля имеется конструктор копии - вызываем его
				if (field_copy_constr != NULL)
				{
					ValidateAccess(field.GetSyntax(), this, field_copy_constr);
					TStackValue field_object(true, field_class);
					if (field.HasSizeMultiplier())
					{
						for (int i = 0; i < field.GetSizeMultiplier(); i++)
						{
							//настраиваем указатель this - инициализируемый объект
							field_object.SetAsReference(&(((int*)object.get())[field.GetOffset() + i*field.GetClass()->GetSize()]));
							std::vector<TStackValue> field_formal_params;

							//передаем в качестве параметра ссылку на копируемый объект
							field_formal_params.push_back(TStackValue(true, field_class));
							field_formal_params.back().SetAsReference(&((int*)formal_params[0].get())[field.GetOffset() + i*field.GetClass()->GetSize()]);
							field_copy_constr->Run(TMethodRunContext(&static_fields, &field_formal_params, &result, &field_object));
						}
					}
					else
					{
						//настраиваем указатель this - инициализируемый объект
						field_object.SetAsReference(&(((int*)object.get())[field.GetOffset()]));
						std::vector<TStackValue> field_formal_params;

						//передаем в качестве параметра ссылку на копируемый объект
						field_formal_params.push_back(TStackValue(true, field_class));
						field_formal_params.back().SetAsReference(&((int*)formal_params[0].get())[field.GetOffset()]);
						field_copy_constr->Run(TMethodRunContext(&static_fields, &field_formal_params, &result, &field_object));
					}
				}
			}
		}
	}
	else
	{
		memcpy((int*)object.get(), (int*)formal_params[0].get(), object.GetSize()*sizeof(int));
	}
}


void TSClass::RunAutoAssign(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params)
{
	assert(IsAutoMethodsInitialized());
	assert(auto_assign_operator);

	//оператор присваиваиня должен принимать два аргумента с тем же типом что и данный класс по ссылке
	assert(formal_params.size() == 2);
	assert(formal_params[0].GetClass() == this);
	assert(formal_params[1].GetClass() == this);

	if (fields.size() > 0)
	{
		TStackValue result;
		for (TSClassField& field : fields)
		{
			assert(field.GetClass()->IsAutoMethodsInitialized());
			TSMethod* field_assign_op = field.GetClass()->GetAssignOperator();
			if (!field.GetSyntax()->IsStatic())
			{
				TSClass* field_class = field.GetClass();
				//если у поля имеется конструктор копии - вызываем его
				if (field_assign_op != NULL)
				{
					ValidateAccess(field.GetSyntax(), this, field_assign_op);

					if (field.HasSizeMultiplier())
					{
						for (int i = 0; i < field.GetSizeMultiplier(); i++)
						{
							std::vector<TStackValue> field_formal_params;
							//передаем в качестве параметра ссылку на копируемый объект
							field_formal_params.push_back(TStackValue(true, field_class));
							field_formal_params.back().SetAsReference(&((int*)formal_params[0].get())[field.GetOffset() + i*field.GetClass()->GetSize()]);

							field_formal_params.push_back(TStackValue(true, field_class));
							field_formal_params.back().SetAsReference(&((int*)formal_params[1].get())[field.GetOffset() + i*field.GetClass()->GetSize()]);

							field_assign_op->Run(TMethodRunContext(&static_fields, &field_formal_params, &result, nullptr));
						}
					}
					else
					{
						std::vector<TStackValue> field_formal_params;

						//передаем в качестве параметра ссылку на копируемый объект
						field_formal_params.push_back(TStackValue(true, field_class));
						field_formal_params.back().SetAsReference(&((int*)formal_params[0].get())[field.GetOffset()]);

						field_formal_params.push_back(TStackValue(true, field_class));
						field_formal_params.back().SetAsReference(&((int*)formal_params[1].get())[field.GetOffset()]);

						field_assign_op->Run(TMethodRunContext(&static_fields, &field_formal_params, &result, nullptr));
					}
				}
			}
		}
	}
	else
	{
		memcpy((int*)formal_params[0].get(), (int*)formal_params[1].get(), formal_params[0].GetClass()->GetSize()*sizeof(int));
	}
}