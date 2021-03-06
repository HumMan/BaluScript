﻿#include "SClass.h"

#include <assert.h>
#include <algorithm>
#include <string.h>

#include "SOverloadedMethod.h"
#include "SType.h"
#include "SStatements.h"

#include "SCommon.h"

class TSClass::TPrivate
{
public:
	TPrivate()
	{
	}
	std::vector<std::unique_ptr<TSClassField>> fields;
	std::list<TSOverloadedMethod> methods;

	std::unique_ptr<TSMethod> default_constructor;
	std::unique_ptr<TSOverloadedMethod> copy_constructors;
	///<summary>Пользовательский деструктор (автоматический деструктор, если существует, будет добавлен как PostEvent)</summary>
	std::unique_ptr<TSMethod> destructor;
	std::unique_ptr<TSOverloadedMethod> operators[(short)Lexer::TOperator::End];
	std::unique_ptr<TSOverloadedMethod> conversions;

	std::vector<std::unique_ptr<TSClass>> nested_classes;

	std::unique_ptr<TSMethod> auto_def_constr;
	std::unique_ptr<TSMethod> auto_copy_constr;
	///<summary>Автоматически созданный оператор присваивания</summary>
	std::unique_ptr<TSMethod> auto_assign_operator;
	///<summary>Автоматически созданный деструктор</summary>
	std::unique_ptr<TSMethod> auto_destr;
	///<summary>Класс в пределах которого объявлен данный класс</summary>
	TSClass* owner;

	///<summary>Суррогатные классы для обозначения параметров шаблона в шаблоне при связывании типов</summary>
	std::vector<std::unique_ptr<TSClass>> surrogate_template_params;
};

TSClass::TSClass(TSClass* use_owner, SyntaxApi::IClass* use_syntax_node, SemanticApi::TNodeWithTemplatesType type)
	:TSyntaxNode(use_syntax_node), _this(std::unique_ptr<TPrivate>(new TPrivate()))
{
	if (type == SemanticApi::TNodeWithTemplatesType::Unknown)
	{
		if (use_syntax_node->IsTemplate())
			SetType(SemanticApi::TNodeWithTemplatesType::Template);
		else
			SetType(SemanticApi::TNodeWithTemplatesType::Class);
	}
	else
		SetType(type);

	_this->owner = use_owner;
}

TSClass::TSClass(TSClass * use_owner)
	:TSyntaxNode(nullptr), _this(std::unique_ptr<TPrivate>(new TPrivate()))
{
	SetType(SemanticApi::TNodeWithTemplatesType::SurrogateTemplateParam);
}

TSClass* TSClass::GetOwner()const
{
	return _this->owner;
}

TSClass* TSClass::GetNestedByFullName(std::vector<Lexer::TNameId> full_name, size_t curr_id)
{
	if (curr_id == full_name.size())
		return this;
	Lexer::TNameId curr = full_name[curr_id];
	TSClass* child = GetNested(full_name[curr_id]);
	if (child!=nullptr)
	{
		if (curr_id + 1 == full_name.size())
			return child;
		else
			return child->GetNestedByFullName(full_name, curr_id+1);
	}
	else
		return nullptr;
}

void TSClass::Build()
{
	for (size_t i = 0; i < GetSyntax()->GetFieldsCount(); i++)
	{
		_this->fields.emplace_back(new TSClassField(this, GetSyntax()->GetField(i)));
	}

	for (size_t i = 0; i < GetSyntax()->GetMethodsCount(); i++)
	{
		_this->methods.emplace_back(this, GetSyntax()->GetMethod(i));
		_this->methods.back().Build();
	}

	if (GetSyntax()->HasDefConstr())
	{
		SyntaxApi::IMethod* constr_syntax = GetSyntax()->GetDefaultConstructor();
		_this->default_constructor = std::unique_ptr<TSMethod>(new TSMethod(this, constr_syntax));
		_this->default_constructor->Build();
	}

	_this->copy_constructors = std::unique_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, GetSyntax()->GetCopyConstr()));
	_this->copy_constructors->Build();

	if (GetSyntax()->HasDestructor())
	{
		_this->destructor = std::unique_ptr<TSMethod>(new TSMethod(this, GetSyntax()->GetDestructor()));
		_this->destructor->Build();
	}

	for (size_t i = 0; i < (short)Lexer::TOperator::End; i++)
	{
		_this->operators[i] = std::unique_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, GetSyntax()->GetOperator(i)));
		_this->operators[i]->Build();
	}

	_this->conversions = std::unique_ptr<TSOverloadedMethod>(new TSOverloadedMethod(this, GetSyntax()->GetConversion()));
	_this->conversions->Build();

	for (size_t i = 0; i < GetSyntax()->GetNestedCount(); i++)
	{
		auto nested_class = GetSyntax()->GetNested(i);
		_this->nested_classes.push_back(std::unique_ptr<TSClass>(new TSClass(this, nested_class)));
		_this->nested_classes.back()->Build();
	}
}

TSClass* TSClass::GetNested(Lexer::TNameId name)const
{
	for (const std::unique_ptr<TSClass>& nested_class : _this->nested_classes)
		if (nested_class->GetSyntax()->GetName() == name)
			return nested_class.get();
	return nullptr;
}

size_t TSClass::GetNestedCount() const
{
	return _this->nested_classes.size();
}

SemanticApi::ISClass * TSClass::GetNested(size_t index) const
{
	return _this->nested_classes[index].get();
}

bool TSClass::GetTemplateParameter(Lexer::TNameId name, SemanticApi::TTemplateParameter& result)const
{
	assert(GetType() == SemanticApi::TNodeWithTemplatesType::Realization);
	SyntaxApi::IClass* template_class_syntax = GetTemplateClass()->GetSyntax();
	size_t par_count = template_class_syntax->GetTemplateParamsCount();
	for (size_t i = 0; i < par_count; i++)
	{
		Lexer::TNameId n = template_class_syntax->GetTemplateParam(i);
		if (n == name)
		{
			result = GetTemplateParam(i);
			return true;
		}
	}
	return false;
}

bool TSClass::HasTemplateParameter(Lexer::TNameId name)const
{
	SemanticApi::TTemplateParameter result;
	return GetTemplateParameter(name, result);
}

void TSClass::CheckForErrors()
{
	if (_this->owner != nullptr&&_this->owner->GetOwner() != nullptr&&_this->owner->GetOwner()->GetClass(GetSyntax()->GetName()))
		GetSyntax()->Error("Класс с таким именем уже существует!");//TODO как выводить ошибки если объекты были получены не из кода, а созданы вручную - исправить
	//for (const std::unique_ptr<TSClass> nested_class : nested_classes)
	for (size_t i = 0; i < _this->nested_classes.size(); i++)
	{
		for (size_t k = 0; k < i; k++)
		{
			if (_this->nested_classes[i]->GetSyntax()->GetName() == _this->nested_classes[k]->GetSyntax()->GetName())
				_this->nested_classes[i]->GetSyntax()->Error("Класс с таким именем уже существует!");
		}
	}
	for (std::unique_ptr<TSClassField>& field : _this->fields)
	{
		for (std::unique_ptr<TSClassField>& other_field : _this->fields)
			//for(size_t k=0;k<i;k++)
		{
			if (&field == &other_field)
				break;
			if (_this->owner == nullptr&&!field->GetSyntax()->IsStatic())
				GetSyntax()->Error("Базовый класс может содержать только статические поля!");
			if (field->GetSyntax()->GetName() == other_field->GetSyntax()->GetName())
				field->GetSyntax()->Error("Поле класса с таким именем уже существует!");
			//TODO как быть со статическими членами класса
		}
		std::vector<SemanticApi::ISMethod*> m;
		if ((_this->owner != nullptr&&_this->owner->GetField(GetSyntax()->GetName(), true, false) != nullptr) || GetMethods(m, GetSyntax()->GetName()))
			field->GetSyntax()->Error("Член класса с таким имененем уже существует!");
	}
	for (TSOverloadedMethod& method : _this->methods)
	{
		if ((_this->owner != nullptr&&_this->owner->GetField(method.GetName(), true, false) != nullptr))
			method.GetMethod(0)->GetSyntax()->Error("Статическое поле класса с таким имененем уже существует!");
		method.CheckForErrors();
		std::vector<SemanticApi::ISMethod*> owner_methods;
		if (_this->owner != nullptr&&_this->owner->GetMethods(owner_methods, method.GetName()))
		{
			for (size_t k = 0; k < owner_methods.size(); k++)
			{
				SemanticApi::ISMethod* temp = method.FindParams(owner_methods[k]);
				if (temp != nullptr)
					dynamic_cast<TSMethod*>(temp)->GetSyntax()->Error("Статический метод с такими параметрами уже существует!");
			}
		}
	}

	if (IsExternal() && GetNestedCount()>0)
	{
		GetSyntax()->Error("External класс не может содержать вложенные классы!");
	}
}

TSClass* TSClass::GetClass(Lexer::TNameId use_name)
{
	//мы должны возвращать шаблонный класс, а не его реализацию
	if (GetType() == SemanticApi::TNodeWithTemplatesType::Class || GetType() == SemanticApi::TNodeWithTemplatesType::Template)
	{
		if (GetSyntax()->GetName() == use_name)
			return this;
		for (const std::unique_ptr<TSClass>& nested_class : _this->nested_classes)
		{
			if (nested_class->GetSyntax()->GetName() == use_name)
				return nested_class.get();
		}
	}

	//если реализация класса, то не забываем проверить не является ли идентификатор одним из параметров класса
	if (GetType() == SemanticApi::TNodeWithTemplatesType::Realization)
	{
		std::vector<SemanticApi::TTemplateParameter> template_params = GetTemplateParams();
		for (size_t i = 0; i < template_params.size(); i++)
			if (!template_params[i].is_value)
				if (GetTemplateClass()->GetSyntax()->GetTemplateParam(i) == use_name)
					return dynamic_cast<TSClass*>(template_params[i].type);
	}

	if (GetType() == SemanticApi::TNodeWithTemplatesType::Template)
	{
		auto count = GetSyntax()->GetTemplateParamsCount();
		for (size_t i = 0; i < count; i++)
				if (GetSyntax()->GetTemplateParam(i) == use_name)
				{
					_this->surrogate_template_params.push_back(std::unique_ptr<TSClass>(new TSClass(this)));
					return _this->surrogate_template_params.back().get();
				}
	}

	if (_this->owner != nullptr)
		return _this->owner->GetClass(use_name);
	return nullptr;
}

TSClassField* TSClass::GetField(Lexer::TNameId name, bool only_in_this)
{
	TSClassField* result_ns = GetField(name, false, only_in_this);
	TSClassField* result_s = GetField(name, true, only_in_this);
	if (result_ns != nullptr)
		return result_ns;
	else
		return result_s;
}
SemanticApi::ISClassField* TSClass::GetField(size_t i)const
{
	return _this->fields[i].get();
}
size_t TSClass::GetFieldsCount() const
{
	return _this->fields.size();
}
TSClassField* TSClass::GetField(Lexer::TNameId name, bool is_static, bool only_in_this)
{
	TSClassField* result_parent = nullptr;
	for (std::unique_ptr<TSClassField>& field : _this->fields)
	{
		if (field->GetSyntax()->IsStatic() == is_static && field->GetSyntax()->GetName() == name) {
			return field.get();
		}
	}
	if (!only_in_this && is_static && _this->owner != nullptr)
		return _this->owner->GetField(name, true, false);
	return nullptr;
}

void TSClass::LinkSignature(SemanticApi::TGlobalBuildContext build_context)
{
	if (!IsSignatureLinked())
		SetSignatureLinked();
	else
		return;

	//определить присутствие конструктора по умолчанию, деструктора, конструктора копии

	for (std::unique_ptr<TSClassField>& field : _this->fields)
	{
		field->LinkSignature(build_context);
		if (field->GetSyntax()->IsStatic())
			build_context.static_fields->push_back(field.get());
	}

	//слинковать сигнатуры методов
	for (TSOverloadedMethod& method : _this->methods)
	{
		method.LinkSignature(build_context);
	}

	if (_this->default_constructor)
		_this->default_constructor->LinkSignature(build_context);
	if (_this->copy_constructors)
		_this->copy_constructors->LinkSignature(build_context);
	if (_this->destructor)
		_this->destructor->LinkSignature(build_context);

	for (size_t i = 0; i < (short)Lexer::TOperator::End; i++)
		_this->operators[i]->LinkSignature(build_context);
	_this->conversions->LinkSignature(build_context);

	for (const std::unique_ptr<TSClass>& nested_class : _this->nested_classes)
		if (!nested_class->GetSyntax()->IsTemplate())
			nested_class->LinkSignature(build_context);
}

void TSClass::LinkBody(SemanticApi::TGlobalBuildContext build_context)
{
	if (!IsBodyLinked())
		SetBodyLinked();
	else
		return;

	CheckForErrors();

	for (std::unique_ptr<TSClassField>& field : _this->fields)
	{
		field->LinkBody(build_context);
	}

	//слинковать тела методов - требуется наличие инф. обо всех методах, conversion, операторах класса
	for (TSOverloadedMethod& method : _this->methods)
	{
		method.LinkBody(build_context);
	}
	if (_this->default_constructor)
		_this->default_constructor->LinkBody(build_context);
	if (_this->copy_constructors)
		_this->copy_constructors->LinkBody(build_context);
	if (_this->destructor)
		_this->destructor->LinkBody(build_context);

	for (size_t i = 0; i < (short)Lexer::TOperator::End; i++)
		if (_this->operators[i])
			_this->operators[i]->LinkBody(build_context);
	_this->conversions->LinkBody(build_context);

	for (const std::unique_ptr<TSClass>& nested_class : _this->nested_classes)
		if (!nested_class->GetSyntax()->IsTemplate())
			nested_class->LinkBody(build_context);
}

void TSClass::GetMethods(std::vector<SemanticApi::ISMethod*> &result)const
{
	for (TSOverloadedMethod& ov_method : _this->methods)
	{
		for (size_t i = 0; i < ov_method.GetMethodsCount(); i++)
			result.push_back(ov_method.GetMethod(i));
	}
}

bool TSClass::GetMethods(std::vector<SemanticApi::ISMethod*> &result, Lexer::TNameId use_method_name, SemanticApi::Filter is_static, bool scan_owner)const
{
	//assert(IsSignatureLinked());
	for (TSOverloadedMethod& ov_method : _this->methods)
	{
		if (ov_method.GetName() == use_method_name)
		{
			for (size_t i = 0; i < ov_method.GetMethodsCount(); i++)
			{
				auto method = ov_method.GetMethod(i);
				if (is_static == SemanticApi::Filter::NotSet)
					result.push_back(method);
				else
				{
					if(method->GetSyntax()->IsStatic() == (is_static == SemanticApi::Filter::True))
						result.push_back(method);
				}
			}
		}
	}
	if (scan_owner &&
		(is_static==SemanticApi::Filter::True || is_static == SemanticApi::Filter::NotSet) &&
		_this->owner != nullptr)
		_this->owner->GetMethods(result, use_method_name, SemanticApi::Filter::True);

	return result.size() > 0;
}

SemanticApi::ISMethod* TSClass::GetConversion(bool source_ref, SemanticApi::ISClass* target_type)const
{
	assert(IsSignatureLinked());
	for (size_t i = 0; i < _this->conversions->GetMethodsCount(); i++)
	{
		auto conversion = _this->conversions->GetMethod(i);
		if (conversion->GetRetClass() == target_type
			&& conversion->GetParam(0)->GetSyntax()->IsRef() == source_ref) {
			return conversion;
		}
	}
	return nullptr;
}

bool TSClass::GetCopyConstructors(std::vector<SemanticApi::ISMethod*> &result)const
{
	assert(IsAutoMethodsInitialized());
	if (!_this->copy_constructors)
		return false;
	for (size_t i = 0; i < _this->copy_constructors->GetMethodsCount(); i++)
	{
		auto constructor = _this->copy_constructors->GetMethod(i);
		result.push_back(constructor);
	}
	if (_this->auto_copy_constr)
		result.push_back(_this->auto_copy_constr.get());
	return result.size() > 0;
}

SemanticApi::ISMethod* TSClass::GetDefConstr()const
{
	assert(IsAutoMethodsInitialized());
	if (_this->default_constructor)
		return _this->default_constructor.get();
	if (_this->auto_def_constr)
		return _this->auto_def_constr.get();
	return nullptr;
}

SemanticApi::ISMethod* TSClass::GetCopyConstr()const
{
	assert(IsAutoMethodsInitialized());
	if (_this->copy_constructors)
	{
		for (size_t i = 0; i < _this->copy_constructors->GetMethodsCount(); i++)
		{
			auto constructor = _this->copy_constructors->GetMethod(i);
			if (constructor->GetParamsCount() == 1
				&& constructor->GetParam(0)->GetClass() == this
				&& constructor->GetParam(0)->GetSyntax()->IsRef() == true) {
				return constructor;
			}
		}
	}
	if (_this->auto_copy_constr)
		return _this->auto_copy_constr.get();
	return nullptr;
}

SemanticApi::ISMethod* TSClass::GetAssignOperator()const
{
	assert(IsAutoMethodsInitialized());
	if (_this->operators[(short)Lexer::TOperator::Assign])
	{
		auto assign_ops = _this->operators[(short)Lexer::TOperator::Assign].get();
		for (size_t i = 0; i < assign_ops->GetMethodsCount(); i++)
		{

			auto assign_op = assign_ops->GetMethod(i);
			if (assign_op->GetParamsCount() == 2
				&& assign_op->GetParam(0)->GetClass() == this
				&& assign_op->GetParam(0)->GetSyntax()->IsRef() == true
				&& assign_op->GetParam(1)->GetClass() == this
				&& assign_op->GetParam(1)->GetSyntax()->IsRef() == true) {
				return assign_op;
			}
		}
	}
	if (_this->auto_assign_operator)
		return _this->auto_assign_operator.get();
	return nullptr;
}

SemanticApi::ISMethod* TSClass::GetDestructor()const
{
	assert(IsAutoMethodsInitialized());
	if (_this->destructor)
		return _this->destructor.get();
	if (_this->auto_destr)
		return _this->auto_destr.get();
	return nullptr;
}

bool TSClass::HasConversion(SemanticApi::ISClass* target_type)
{
	return GetConversion(true, target_type)
		|| GetConversion(false, target_type);
}

bool TSClass::IsExternal() const
{
	return this->GetSyntax()->IsExternal();
}
SyntaxApi::IClass * TSClass::IGetSyntax() const
{
	return GetSyntax();
}
bool TSClass::GetOperators(std::vector<SemanticApi::ISMethod*> &result, Lexer::TOperator op)const
{
	assert(IsAutoMethodsInitialized());
	if (!_this->operators[(short)op])
		return false;
	_this->operators[(short)op]->GetMethods(result);
	if (result.size() == 0 && op == Lexer::TOperator::Assign)
		if (_this->auto_assign_operator)
			result.push_back(_this->auto_assign_operator.get());
	return result.size() > 0;
}

void TSClass::AddClass(TSClass* use_class)
{
	_this->nested_classes.push_back(std::unique_ptr<TSClass>(use_class));
}
TSClass::~TSClass()
{
}
std::vector<SemanticApi::ISClassField*> TSClass::GetFields() const
{
	std::vector<SemanticApi::ISClassField*> result;
	result.reserve(_this->fields.size());
	for (auto& v : _this->fields)
		result.push_back(v.get());
	return result;
}
SemanticApi::ISMethod* TSClass::GetAutoDefConstr()const
{
	return _this->auto_def_constr.get();
}
SemanticApi::ISMethod * TSClass::GetAutoCopyConstr() const
{
	return _this->auto_copy_constr.get();
}
SemanticApi::ISMethod * TSClass::GetAutoAssignOperator() const
{
	return _this->auto_assign_operator.get();
}
SemanticApi::ISMethod* TSClass::GetAutoDestr()const
{
	return _this->auto_destr.get();
}
void TSClass::CopyExternalMethodBindingsFrom(TSClass* source)
{
	//слинковать сигнатуры методов
	auto i = _this->methods.begin();
	auto k = source->_this->methods.begin();
	while( i != _this->methods.end())
	{
		i->CopyExternalMethodBindingsFrom(&(*k));
		i++;
		k++;
	}
	if (_this->default_constructor)
		_this->default_constructor->CopyExternalMethodBindingsFrom(source->_this->default_constructor.get());
	if (_this->copy_constructors)
		_this->copy_constructors->CopyExternalMethodBindingsFrom(source->_this->copy_constructors.get());
	if (_this->destructor)
		_this->destructor->CopyExternalMethodBindingsFrom(source->_this->destructor.get());

	for (size_t i = 0; i < (short)Lexer::TOperator::End; i++)
		_this->operators[i]->CopyExternalMethodBindingsFrom(source->_this->operators[i].get());
	_this->conversions->CopyExternalMethodBindingsFrom(source->_this->conversions.get());

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
		if (GetType() != SemanticApi::TNodeWithTemplatesType::Template)
		{
			if (!IsSizeInitialized())
			{
				size_t class_size = 0;
				if (std::find(owners.begin(), owners.end(), this) != owners.end())
				{
					//Error(" ласс не может содержать поле с собственным типом(кроме дин. массивов)!");//TODO см. initautomethods дл¤ дин массивов
					GetSyntax()->Error("Класс не может содержать поле собственного типа!");
				}
				else {
					owners.push_back(this);					
					for (std::unique_ptr<TSClassField>& field : _this->fields)
					{
						dynamic_cast<TSClass*>(field->GetClass())->CalculateSizes(owners);
						if (!field->GetSyntax()->IsStatic())
						{
							field->SetOffset(class_size);
							if (field->GetSyntax()->HasSizeMultiplierId())
							{
								Lexer::TNameId multiplier_id = field->GetSyntax()->GetFactorId();
								size_t multiplier = this->FindTemplateIntParameter(multiplier_id);
								field->SetSizeMultiplier(multiplier);
							}
							else if (field->GetSyntax()->HasSizeMultiplier())
							{
								size_t multiplier = field->GetSyntax()->GetSizeMultiplier();
								field->SetSizeMultiplier(multiplier);
							}

							if (field->HasSizeMultiplier())
								class_size += field->GetClass()->GetSize()*field->GetSizeMultiplier();
							else
								class_size += field->GetClass()->GetSize();
						}
					}
				}
				owners.pop_back();
				SetSize(class_size);
			}
			for (const std::unique_ptr<TSClass>& nested_class : _this->nested_classes)
				nested_class->CalculateSizes(owners);
		}
		if (GetType() == SemanticApi::TNodeWithTemplatesType::Template)
			for (const std::unique_ptr<TSClass>& realization : GetRealizations())
				realization->CalculateSizes(owners);
	}
}

void TSClass::CalculateMethodsSizes()
{
	if (GetType() != SemanticApi::TNodeWithTemplatesType::Template)
	{

		for (TSOverloadedMethod& method : _this->methods)
		{
			method.CalculateParametersOffsets();
		}
		if (_this->default_constructor)
			_this->default_constructor->CalculateParametersOffsets();
		if (_this->copy_constructors)
			_this->copy_constructors->CalculateParametersOffsets();
		if (_this->destructor)
			_this->destructor->CalculateParametersOffsets();

		for (size_t i = 0; i < (short)Lexer::TOperator::End; i++)
			if (_this->operators[i])
				_this->operators[i]->CalculateParametersOffsets();
		if (_this->conversions)
			_this->conversions->CalculateParametersOffsets();

		for (const std::unique_ptr<TSClass>& nested_class : _this->nested_classes)
			nested_class->CalculateMethodsSizes();
	}
	if (GetType() == SemanticApi::TNodeWithTemplatesType::Template)
		for (const std::unique_ptr<TSClass>& realization : GetRealizations())
			realization->CalculateMethodsSizes();
}

void TSClass::InitAutoMethods()
{
	if (GetType() == SemanticApi::TNodeWithTemplatesType::Template)
		return;
	if (IsAutoMethodsInitialized())
		return;

	for (std::unique_ptr<TSClassField>& field : _this->fields)
	{
		dynamic_cast<TSClass*>(field->GetClass())->InitAutoMethods();
	}

	bool has_def_constr = false;
	bool has_copy_constr = false;
	bool has_destr = false;
	bool has_assign_op = false;

	for (std::unique_ptr<TSClassField>& field : _this->fields)
	{
		SemanticApi::ISClass* field_class = field->GetClass();
		if (field_class->GetDefConstr() != nullptr)
			has_def_constr = true;
		if (field_class->GetCopyConstr() != nullptr)
			has_copy_constr = true;
		if (field_class->GetDestructor() != nullptr)
			has_destr = true;
		if (field_class->GetAssignOperator() != nullptr)
			has_assign_op = true;
	}

	//TODO проверка наследуемых классов на наличие конструктора, деструктора и т.д.

	assert(!_this->auto_def_constr);
	assert(!_this->auto_copy_constr);
	assert(!_this->auto_destr);
	assert(!_this->auto_assign_operator);

	if (has_def_constr)
	{
		_this->auto_def_constr.reset(new TSMethod(this, SemanticApi::SpecialClassMethodType::AutoDefConstr));
	}
	//if (has_copy_constr)
	{
		_this->auto_copy_constr.reset(new TSMethod(this, SemanticApi::SpecialClassMethodType::AutoCopyConstr));
		TSParameter* p = new TSParameter(this, _this->auto_copy_constr.get(), this, true);
		_this->auto_copy_constr->AddParameter(p);

		_this->auto_copy_constr->LinkSignatureForSpecialMethod();

	}
	if (has_destr)
	{
		_this->auto_destr.reset(new TSMethod(this, SemanticApi::SpecialClassMethodType::AutoDestructor));
	}


	//if (has_assign_op)
	{
		_this->auto_assign_operator.reset(new TSMethod(this, SemanticApi::SpecialClassMethodType::AutoAssignOperator));
		TSParameter* p = new TSParameter(this, _this->auto_assign_operator.get(), this, true);
		_this->auto_assign_operator->AddParameter(p);
		p = new TSParameter(this, _this->auto_assign_operator.get(), this, true);
		_this->auto_assign_operator->AddParameter(p);

		_this->auto_assign_operator->LinkSignatureForSpecialMethod();
	}

	SetAutoMethodsInitialized();

	for (const std::unique_ptr<TSClass>& nested_class : _this->nested_classes)
		nested_class->InitAutoMethods();

	if (GetType() == SemanticApi::TNodeWithTemplatesType::Template)
		for (const std::unique_ptr<TSClass>& realization : GetRealizations())
			realization->InitAutoMethods();
}

void TSClass::SetExternal(const std::vector<SemanticApi::TExternalSMethod>& bindings, int& curr_bind)
{
	if (GetDefConstr() != nullptr && GetDefConstr()->IsExternal())
	{
		GetDefConstr()->SetAsExternal(bindings[curr_bind]);
		curr_bind++;
	}

	std::vector<SemanticApi::ISMethod*> m;

	m.clear();
	GetCopyConstructors(m);
	for (size_t i = 0; i < m.size(); i++)
	{
		if (m[i]->IsExternal())
		{
			m[i]->SetAsExternal(bindings[curr_bind]);
			curr_bind++;
		}
	}

	if (GetDestructor() != nullptr && GetDestructor()->IsExternal())
	{
		GetDestructor()->SetAsExternal(bindings[curr_bind]);
		curr_bind++;
	}


	for (int op = 0; op < (int)Lexer::TOperator::End; op++)
	{
		m.clear();
		GetOperators(m, (Lexer::TOperator)op);
		for (size_t i = 0; i < m.size(); i++)
		{
			if (m[i]->IsExternal())
			{
				m[i]->SetAsExternal(bindings[curr_bind]);
				curr_bind++;
			}
		}

	}

	m.clear();
	GetMethods(m);
	for (size_t i = 0; i < m.size(); i++)
	{
		if (m[i]->IsExternal())
		{
			m[i]->SetAsExternal(bindings[curr_bind]);
			curr_bind++;
		}
	}

	auto nested_count = GetNestedCount();
	for (size_t i = 0; i < nested_count; i++)
	{
		GetNested(i)->SetExternal(bindings, curr_bind);
	}
}