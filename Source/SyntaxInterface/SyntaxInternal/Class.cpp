#include "Class.h"

#include "ClassField.h"
#include "Type.h"
#include "OverloadedMethod.h"
#include "Method.h"

using namespace Lexer;
using namespace SyntaxInternal;

namespace SyntaxInternal
{
	class TClass::TClassInternal
	{
	public:
		bool is_template;

		std::vector<Lexer::TNameId> template_params;

		std::vector<std::unique_ptr<TClassField>> fields;
		std::vector<std::unique_ptr<TOverloadedMethod>> methods;
		std::unique_ptr<TMethod> constr_default;
		std::unique_ptr<TOverloadedMethod> constr_copy, constr_move;
		///<summary>Пользовательский деструктор</summary>
		std::unique_ptr<TMethod> destructor;
		std::unique_ptr<TOverloadedMethod> operators[(short)Lexer::TOperator::End];
		std::unique_ptr<TOverloadedMethod> conversions;

		std::vector<std::unique_ptr<TClass>> nested_classes;

		///<summary>Название класса</summary>
		Lexer::TNameId name;
		///<summary>Тип от которого унаследован данный класс</summary>
		std::unique_ptr<TType> parent;
		///<summary>От данного класса запрещено наследование</summary>
		bool is_sealed;
		///<summary>Класс в пределах которого объявлен данный класс</summary>
		TClass* owner;
		bool is_external;
	};
}

bool TClass::IsTemplate()const
{
	return _this->is_template;
}

bool TClass::IsExternal()const
{
	return _this->is_external;
}

Lexer::TNameId TClass::GetTemplateParam(int i)const
{
	return _this->template_params[i];
}

size_t TClass::GetTemplateParamsCount()const
{
	return _this->template_params.size();
}

Lexer::TNameId TClass::GetName()const
{
	return _this->name;
}

TClass* TClass::GetOwner()const
{
	return _this->owner;
}

void TClass::SetName(Lexer::TNameId name)
{
	_this->name = name;
}

std::vector<Lexer::TNameId> TClass::GetFullClassName()const
{
	std::vector<Lexer::TNameId> result;
	if (_this->owner != nullptr)
		result = _this->owner->GetFullClassName();
	result.push_back(_this->name);
	return result;
}

TClass::TClass(TClass* use_owner)
{
	_this.reset(new TClassInternal());
	_this->parent.reset(new TType(this));
	_this->is_template = false;
	_this->is_sealed = false;
	_this->is_external = false;
	_this->owner = use_owner;

	_this->constr_copy.reset(new TOverloadedMethod());
	_this->constr_move.reset(new TOverloadedMethod());
	for (size_t i = 0; i < (short)Lexer::TOperator::End;i++)
		_this->operators[i].reset(new TOverloadedMethod());
	_this->conversions.reset(new TOverloadedMethod());
}

TClass::~TClass()
{
}

void TClass::AddMethod(TMethod* use_method, Lexer::TNameId name) {
	//»щем перегруженные методы с таким же именем, иначе добавл¤ем новый
	TOverloadedMethod* temp = nullptr;
	for (std::unique_ptr<TOverloadedMethod>& method : _this->methods)
		if (method->GetName() == name)
			temp = method.get();
	if (temp == nullptr)
	{
		_this->methods.emplace_back(new TOverloadedMethod(name));
		temp = _this->methods.back().get();
	}
	temp->AddMethod(use_method);
}

void TClass::AddOperator(Lexer::TOperator op, TMethod* use_method)
{
	_this->operators[(short)op]->AddMethod(use_method);
}

void TClass::AddConversion(TMethod* use_method) 
{
	_this->conversions->AddMethod(use_method);
}

void TClass::AddDefaultConstr(TMethod* use_method)
{
	if (use_method->GetParamsCount() != 0)
		Error("Конструктор по умолчанию уже существует!");
	_this->constr_default.reset(use_method);
}

void TClass::AddCopyConstr(TMethod* use_method)
{
	if(use_method->GetParamsCount()==0)
		Error("Конструктор копии должен иметь параметры!");
	_this->constr_copy->AddMethod(use_method);
}

void TClass::AddMoveConstr(TMethod* use_method)
{
	if (use_method->GetParamsCount() == 0)
		Error("Конструктор перемещения должен иметь параметры!");
	_this->constr_move->AddMethod(use_method);
}

void TClass::AddDestr(TMethod* use_method)
{
	if (_this->destructor)
		Error("Деструктор уже существует!");
	_this->destructor = std::unique_ptr<TMethod>(use_method);
}

size_t TClass::GetFieldsCount()const
{
	return _this->fields.size();
}
SyntaxApi::IClassField* TClass::GetField(size_t i) const
{
	return _this->fields[i].get();
}
size_t TClass::GetMethodsCount()const
{
	return _this->methods.size();
}
SyntaxApi::IOverloadedMethod* TClass::GetMethod(int i) const
{
	return _this->methods[i].get();
}

void TClass::AddNested(TClass* use_class)
{
	_this->nested_classes.push_back(std::unique_ptr<TClass>(use_class));
}

TClassField* TClass::EmplaceField(TClass* use_field_owner)
{
	_this->fields.emplace_back(new TClassField(use_field_owner));
	return _this->fields.back().get();
}

size_t TClass::GetNestedCount()const
{
	return _this->nested_classes.size();
}

TClass* TClass::GetNested(Lexer::TNameId name)const
{
	for (size_t i = 0; i < _this->nested_classes.size(); i++)
	{
		if (_this->nested_classes[i]->GetName() == name)
			return _this->nested_classes[i].get();
	}
	return nullptr;
}

TClass* TClass::GetNested(size_t i)const
{
	return _this->nested_classes[i].get();
}

SyntaxApi::IType* TClass::GetParent()const
{
	return _this->parent.get();
}

size_t TClass::FindNested(Lexer::TNameId name)const
{
	for (size_t i = 0; i < _this->nested_classes.size(); i++)
	{
		if (_this->nested_classes[i]->GetName() == name)
			return i;
	}
	return -1;
}

void TClass::AccessDecl(Lexer::ILexer* source, bool& readonly,
	SyntaxApi::TTypeOfAccess access) {
	if (source->Type() == Lexer::TTokenType::ResWord)
	{
		switch ((TResWord)source->Token())
		{
		case TResWord::Readonly:
			source->GetToken();
			source->GetToken(TTokenType::Colon);
			readonly = true;
			access = SyntaxApi::TTypeOfAccess::Public;
			break;
		case TResWord::Public:
			source->GetToken();
			source->GetToken(TTokenType::Colon);
			readonly = false;
			access = SyntaxApi::TTypeOfAccess::Public;
			break;
		case TResWord::Protected:
			source->GetToken();
			source->GetToken(TTokenType::Colon);
			readonly = false;
			access = SyntaxApi::TTypeOfAccess::Protected;
			break;
		case TResWord::Private:
			source->GetToken();
			source->GetToken(TTokenType::Colon);
			readonly = false;
			access = SyntaxApi::TTypeOfAccess::Private;
			break;
		}
	}
}

void TClass::AnalyzeSyntax(Lexer::ILexer* source) 
{
	InitPos(source);

	if(_this->owner!=nullptr)
		_this->owner->AddNested(this);

	if (source->TestAndGet(TResWord::Enum))
	{
		SetAsEnumeration();
		source->TestToken(TTokenType::Identifier);
		_this->name = source->NameId();
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
		_this->is_sealed = source->TestAndGet(TResWord::Sealed);
		_this->is_external = source->TestAndGet(TResWord::Extern);
		source->TestToken(TTokenType::Identifier);
		_this->name = source->NameId();
		source->GetToken();
		if (source->TestAndGet(TOperator::Less)) 
		{
			_this->is_template = true;
			do {
				source->TestToken(TTokenType::Identifier);
				_this->template_params.push_back(source->NameId());
				source->GetToken();
				if (!source->TestAndGet(TTokenType::Comma))
					break;
			} while (true);
			source->GetToken(TOperator::Greater);
		}
		if (source->TestAndGet(TTokenType::Colon)) 
		{
			source->TestToken(TTokenType::Identifier);
			_this->parent->AnalyzeSyntax(source);
		}
		source->GetToken(TTokenType::LBrace);

		bool readonly = false;
		SyntaxApi::TTypeOfAccess access = SyntaxApi::TTypeOfAccess::Public;

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
					nested->AnalyzeSyntax(source);
				}
				break;
				case TResWord::Enum:
				{
					TClass* enumeraiton = new TClass(this);
					enumeraiton->AnalyzeSyntax(source);
				}
				break;
				case TResWord::Func:
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
					_this->fields.emplace_back(new TClassField(this));
					_this->fields.back()->SetFactorId(factor);
					_this->fields.back()->SetAccess(access);
					_this->fields.back()->SetReadOnly(readonly);
					_this->fields.back()->AnalyzeSyntax(source);
				}
					break;
				default:
					end_while = true;
			}
			else if (source->Type() == TTokenType::Identifier) 
			{
				_this->fields.emplace_back(new TClassField(this));
				_this->fields.back()->SetAccess(access);
				_this->fields.back()->SetReadOnly(readonly);
				_this->fields.back()->AnalyzeSyntax(source);
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
	return (bool)_this->constr_default;
}
SyntaxApi::IMethod* TClass::GetDefaultConstructor() const
{
	return _this->constr_default.get();
}
bool TClass::HasDestructor()const
{
	return (bool)_this->destructor;
}
SyntaxApi::IMethod* TClass::GetDestructor() const
{
	return _this->destructor.get();
}
SyntaxApi::IOverloadedMethod* TClass::GetCopyConstr() const
{
	return _this->constr_copy.get();
}
SyntaxApi::IOverloadedMethod* TClass::GetMoveConstr() const
{
	return _this->constr_move.get();
}
SyntaxApi::IOverloadedMethod* TClass::GetOperator(int i) const
{
	return _this->operators[i].get();
}
SyntaxApi::IOverloadedMethod* TClass::GetConversion() const
{
	return _this->conversions.get();
}