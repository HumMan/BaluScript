#include "Class.h"

using namespace Lexer;

bool TClass::IsTemplate()const
{
	return is_template;
}

bool TClass::IsExternal()const
{
	return is_external;
}

ICanBeEnumeration* TClass::AsEnumeration() const
{
	return (ICanBeEnumeration*)this;
}
Lexer::TTokenPos TClass::AsTokenPos()const
{
	return *(Lexer::TTokenPos*)this;
}

Lexer::TNameId TClass::GetTemplateParam(int i)const
{
	return template_params[i];
}

int TClass::GetTemplateParamsCount()const
{
	return template_params.size();
}

Lexer::TNameId TClass::GetName()const
{
	return name;
}

TClass* TClass::GetOwner()const
{
	return owner;
}

void TClass::SetName(Lexer::TNameId name)
{
	this->name = name;
}

std::vector<Lexer::TNameId> TClass::GetFullClassName()const
{
	std::vector<Lexer::TNameId> result;
	if (owner != NULL)
		result = owner->GetFullClassName();
	result.push_back(name);
	return result;
}

TClass::TClass(TClass* use_owner) :parent(new TType(this))
{
	is_template = false;
	is_sealed = false;
	is_external = false;
	owner = use_owner;

	constr_copy.reset(new TOverloadedMethod());
	constr_move.reset(new TOverloadedMethod());
	for (int i = 0; i < (short)Lexer::TOperator::End;i++)
		operators[i].reset(new TOverloadedMethod());
	conversions.reset(new TOverloadedMethod());
}

void TClass::AddMethod(TMethod* use_method, Lexer::TNameId name) {
	//»щем перегруженные методы с таким же именем, иначе добавл¤ем новый
	TOverloadedMethod* temp = NULL;
	for (std::unique_ptr<TOverloadedMethod>& method : methods)
		if (method->GetName() == name)
			temp = method.get();
	if (temp == NULL)
	{
		methods.emplace_back(new TOverloadedMethod(name));
		temp = methods.back().get();
	}
	temp->methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddOperator(Lexer::TOperator op, TMethod* use_method)
{
	operators[(short)op]->methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddConversion(TMethod* use_method) 
{
	conversions->methods.push_back(std::unique_ptr<TMethod>(use_method));
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
	constr_copy->methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddMoveConstr(TMethod* use_method)
{
	if (use_method->GetParamsCount() == 0)
		Error("Конструктор перемещения должен иметь параметры!");
	constr_move->methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddDestr(TMethod* use_method)
{
	if (destructor)
		Error("Деструктор уже существует!");
	destructor = std::unique_ptr<TMethod>(use_method);
}

int TClass::GetFieldsCount()const
{
	return fields.size();
}
TClassField* TClass::GetField(int i) const
{
	return fields[i].get();
}
int TClass::GetMethodsCount()const
{
	return methods.size();
}
TOverloadedMethod* TClass::GetMethod(int i) const
{
	return methods[i].get();
}

void TClass::AddNested(TClass* use_class)
{
	nested_classes.push_back(std::unique_ptr<TClass>(use_class));
}

TClassField* TClass::EmplaceField(TClass* use_field_owner)
{
	fields.emplace_back(new TClassField(use_field_owner));
	return fields.back().get();
}

int TClass::GetNestedCount()const
{
	return nested_classes.size();
}

TClass* TClass::GetNested(Lexer::TNameId name)const
{
	for (size_t i = 0; i < nested_classes.size(); i++)
	{
		if (nested_classes[i]->GetName() == name)
			return nested_classes[i].get();
	}
	return nullptr;
}

TClass* TClass::GetNested(int i)const
{
	return nested_classes[i].get();
}

TType* TClass::GetParent()const
{
	return parent.get();
}

int TClass::FindNested(Lexer::TNameId name)const
{
	for (size_t i = 0; i < nested_classes.size(); i++)
	{
		if (nested_classes[i]->GetName() == name)
			return i;
	}
	return -1;
}

void TClass::AccessDecl(Lexer::ILexer* source, bool& readonly,
		TTypeOfAccess access) {
	if (source->Type() == Lexer::TTokenType::ResWord)
	{
		switch ((TResWord)source->Token())
		{
		case TResWord::Readonly:
			source->GetToken();
			source->GetToken(TTokenType::Colon);
			readonly = true;
			access = TTypeOfAccess::Public;
			break;
		case TResWord::Public:
			source->GetToken();
			source->GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Public;
			break;
		case TResWord::Protected:
			source->GetToken();
			source->GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Protected;
			break;
		case TResWord::Private:
			source->GetToken();
			source->GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Private;
			break;
		}
	}
}

void TClass::AnalyzeSyntax(Lexer::ILexer* source) 
{
	InitPos(source);

	if (source->TestAndGet(TResWord::Enum))
	{
		SetAsEnumeration();
		source->TestToken(TTokenType::Identifier);
		name = source->NameId();
		source->GetToken();
		source->GetToken(TTokenType::LBrace);
		do
		{
			auto enums = GetEnums();
			if (source->Test(TTokenType::Identifier))
			{
				for (size_t i = 0; i < enums.size(); i++)
					if (enums[i] == source->NameId())
						source->Error("Значение перечисления с таким именем уже существует!");
				enums.push_back(source->NameId());
				source->GetToken();
			}
			else
				break;
		} while (source->TestAndGet(TTokenType::Comma));
		source->GetToken(TTokenType::RBrace);
		return;
	}
	else
	{

		source->GetToken(TResWord::Class);
		is_sealed = source->TestAndGet(TResWord::Sealed);
		is_external = source->TestAndGet(TResWord::Extern);
		source->TestToken(TTokenType::Identifier);
		name = source->NameId();
		source->GetToken();
		if (source->TestAndGet(TOperator::Less)) 
		{
			is_template = true;
			do {
				source->TestToken(TTokenType::Identifier);
				template_params.push_back(source->NameId());
				source->GetToken();
				if (!source->TestAndGet(TTokenType::Comma))
					break;
			} while (true);
			source->GetToken(TOperator::Greater);
		}
		if (source->TestAndGet(TTokenType::Colon)) 
		{
			source->TestToken(TTokenType::Identifier);
			parent->AnalyzeSyntax(source);
		}
		source->GetToken(TTokenType::LBrace);

		bool readonly = false;
		TTypeOfAccess access = TTypeOfAccess::Public;

		while (true)
		{
			bool end_while = false;
			AccessDecl(source, readonly, access);
			if (source->Type() == TTokenType::ResWord)
				switch ((TResWord)source->Token())
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
					source->GetToken(TResWord::Multifield);
					source->GetToken(TTokenType::LParenth);
					TNameId factor = source->NameId();
					source->GetToken();
					source->GetToken(TTokenType::RParenth);
					fields.emplace_back(new TClassField(this));
					fields.back()->SetFactorId(factor);
					fields.back()->SetAccess(access);
					fields.back()->SetReadOnly(readonly);
					fields.back()->AnalyzeSyntax(source);
				}
					break;
				default:
					end_while = true;
			}
			else if (source->Type() == TTokenType::Identifier) 
			{
				fields.emplace_back(new TClassField(this));
				fields.back()->SetAccess(access);
				fields.back()->SetReadOnly(readonly);
				fields.back()->AnalyzeSyntax(source);
			}
			else
				break;
			if (end_while)
				break;
		}
		source->GetToken(TTokenType::RBrace);
	}
}

bool TClass::HasDefConstr()const
{
	return (bool)constr_default;
}
TMethod* TClass::GetDefaultConstructor() const
{
	return constr_default.get();
}
bool TClass::HasDestructor()const
{
	return (bool)destructor;
}
TMethod* TClass::GetDestructor() const
{
	return destructor.get();
}
TOverloadedMethod* TClass::GetCopyConstr() const
{
	return constr_copy.get();
}
TOverloadedMethod* TClass::GetMoveConstr() const
{
	return constr_move.get();
}
TOverloadedMethod* TClass::GetOperator(int i) const
{
	return operators[i].get();
}
TOverloadedMethod* TClass::GetConversion() const
{
	return conversions.get();
}