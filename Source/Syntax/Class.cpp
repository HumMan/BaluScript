﻿#include "Class.h"

#include "../NativeTypes/DynArray.h"
#include "OverloadedMethod.h"
#include "Method.h"
#include "Statements.h"

bool TClass::IsTemplate()
{
	return is_template;
}

bool TClass::IsExternal()
{
	return is_external;
}

void TClass::SetIsTemplate(bool use_is_template)
{
	is_template = use_is_template;
}

int TClass::GetTemplateParamsCount()
{
	return template_params.size();
}

TNameId TClass::GetName()
{
	return name;
}

TClass* TClass::GetOwner()
{
	return owner;
}

std::vector<TNameId> TClass::GetFullClassName()
{
	std::vector<TNameId> result;
	if (owner != NULL)
		result = owner->GetFullClassName();
	result.push_back(name);
	return result;
}

TClass::TClass(TClass* use_owner) :parent(this)
{
	is_template = false;
	is_sealed = false;
	is_external = false;
	owner = use_owner;
}

void TClass::AddMethod(TMethod* use_method, TNameId name) {
	//»щем перегруженные методы с таким же именем, иначе добавл¤ем новый
	TOverloadedMethod* temp = NULL;
	for (TOverloadedMethod& method : methods)
		if (method.GetName() == name)
			temp = &method;
	if (temp == NULL)
	{
		methods.emplace_back(name);
		temp = &methods.back();
	}
	temp->methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddOperator(TOperator::Enum op, TMethod* use_method) 
{
	operators[op].methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddConversion(TMethod* use_method) 
{
	conversions.methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddDefaultConstr(TMethod* use_method)
{
	if (use_method->GetParamsCount() != 0)
		Error("Конструктор по умолчанию уже существует!");
	constr_default.reset(use_method);
}

void TClass::AddCopyConstr(TMethod* use_method)
{
	if(use_method->GetParamsCount()==0)
		Error("Конструктор копии должен иметь параметры!");
	constr_copy.methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddMoveConstr(TMethod* use_method)
{
	if (use_method->GetParamsCount() == 0)
		Error("Конструктор перемещения должен иметь параметры!");
	constr_move.methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddDestr(TMethod* use_method)
{
	if (destructor)
		Error("Деструктор уже существует!");
	destructor = std::unique_ptr<TMethod>(use_method);
}

void TClass::AddNested(TClass* use_class)
{
	nested_classes.push_back(std::unique_ptr<TClass>(use_class));
}

TClassField* TClass::EmplaceField(TClass* use_field_owner)
{
	fields.emplace_back(use_field_owner);
	return &fields.back();
}

TClass* TClass::GetNested(TNameId name)
{
	for (const std::unique_ptr<TClass>& nested_class : nested_classes)
		if (nested_class->name == name)
			return nested_class.get();
	return NULL;
}

void TClass::AccessDecl(TLexer& source, bool& readonly,
		TTypeOfAccess::Enum access) {
	if (source.Type() == TTokenType::ResWord)
	{
		switch (source.Token())
		{
		case TResWord::Readonly:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = true;
			access = TTypeOfAccess::Public;
			break;
		case TResWord::Public:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Public;
			break;
		case TResWord::Protected:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Protected;
			break;
		case TResWord::Private:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Private;
			break;
		}
	}
}

void TClass::AnalyzeSyntax(TLexer& source) 
{
	InitPos(source);

	if (source.TestAndGet(TTokenType::ResWord, TResWord::Enum))
	{
		SetAsEnumeration();
		source.TestToken(TTokenType::Identifier);
		name = source.NameId();
		source.GetToken();
		source.GetToken(TTokenType::LBrace);
		do
		{
			source.Test(TTokenType::Identifier);
			for (int i = 0; i < enums.size(); i++)
				if (enums[i] == source.NameId())
					source.Error("Значение перечисления с таким имененм уже существует!");
			enums.push_back(source.NameId());
			source.GetToken();
		} while (source.TestAndGet(TTokenType::Comma));
		source.GetToken(TTokenType::RBrace);
		return;
	}
	else
	{

		source.GetToken(TTokenType::ResWord, TResWord::Class);
		is_sealed = source.TestAndGet(TTokenType::ResWord, TResWord::Sealed);
		is_external = source.TestAndGet(TTokenType::ResWord, TResWord::Extern);
		source.TestToken(TTokenType::Identifier);
		name = source.NameId();
		source.GetToken();
		if (source.TestAndGet(TTokenType::Operator, TOperator::Less)) 
		{
			is_template = true;
			do {
				source.TestToken(TTokenType::Identifier);
				template_params.push_back(source.NameId());
				source.GetToken();
				if (!source.TestAndGet(TTokenType::Comma))
					break;
			} while (true);
			source.GetToken(TTokenType::Operator, TOperator::Greater);
		}
		if (source.TestAndGet(TTokenType::Colon)) 
		{
			source.TestToken(TTokenType::Identifier);
			parent.AnalyzeSyntax(source);
		}
		source.GetToken(TTokenType::LBrace);

		bool readonly = false;
		TTypeOfAccess::Enum access = TTypeOfAccess::Public;

		while (true)
		{
			bool end_while = false;
			AccessDecl(source, readonly, access);
			if (source.Type() == TTokenType::ResWord)
				switch (source.Token())
			{
				case TResWord::Class: 
				{
					TClass* nested = new TClass(this);
					AddNested(nested);
					nested->AnalyzeSyntax(source);
				}
				break;
				case TResWord::Enum:
				{
					TClass* enumeraiton = new TClass(this);
					AddNested(enumeraiton);
					enumeraiton->AnalyzeSyntax(source);
				}
				break;
				case TResWord::Func:
				//case TResWord::Constr:
				case TResWord::Default:
				case TResWord::Copy:
				case TResWord::Move:
				case TResWord::Destr:
				case TResWord::Operator:
				case TResWord::Conversion:
				{
					TMethod* func = new TMethod(this);
					func->SetAccess(access);
					func->AnalyzeSyntax(source);
				}
					break;
				case TResWord::Multifield:
				{
					source.GetToken(TTokenType::ResWord, TResWord::Multifield);
					source.GetToken(TTokenType::LParenth);
					TNameId factor = source.NameId();
					source.GetToken();
					source.GetToken(TTokenType::RParenth);
					fields.emplace_back(this);
					fields.back().SetFactorId(factor);
					fields.back().SetAccess(access);
					fields.back().SetReadOnly(readonly);
					fields.back().AnalyzeSyntax(source);
				}
					break;
				default:
					end_while = true;
			}
			else if (source.Type() == TTokenType::Identifier) 
			{
				fields.emplace_back(this);
				fields.back().SetAccess(access);
				fields.back().SetReadOnly(readonly);
				fields.back().AnalyzeSyntax(source);
			}
			else
				break;
			if (end_while)
				break;
		}
		source.GetToken(TTokenType::RBrace);
	}
}