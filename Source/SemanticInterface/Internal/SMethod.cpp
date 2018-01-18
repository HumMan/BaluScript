#include "SMethod.h"

#include <common.h>
#include <assert.h>

#include "SStatements.h"
#include "SOverloadedMethod.h"
#include "SType.h"
#include "SClass.h"

class TSMethod::TPrivate
{
public:	
	TSType ret;
	bool has_return;
	bool ret_ref;
	TSClass* owner;
	std::vector<std::unique_ptr<TSParameter>> parameters;
	size_t ret_size;
	size_t parameters_size;

	std::unique_ptr<TSStatements> statements;

	bool is_external;
	SemanticApi::TExternalSMethod external_func;
	TPrivate(TSClass *use_owner, SyntaxApi::IType *use_syntax_node) 
		: ret(use_owner, use_syntax_node)
	{
		owner = nullptr;
		is_external = false;
		external_func = nullptr;
	}
	TPrivate(TSClass *use_owner, TSClass *use_class)
		: ret(use_owner, use_class)
	{
		owner = nullptr;
		is_external = false;
		external_func = nullptr;
	}
};

SemanticApi::SpecialClassMethodType GetMethodTypeFromSyntax(SyntaxApi::IMethod* use_syntax)
{
	switch (use_syntax->GetMemberType())
	{
	case SyntaxApi::TClassMember::DefaultConstr:
		return SemanticApi::SpecialClassMethodType::Default;
	case SyntaxApi::TClassMember::CopyConstr:
		return SemanticApi::SpecialClassMethodType::CopyConstr;
	case SyntaxApi::TClassMember::Destr:
		return SemanticApi::SpecialClassMethodType::Destructor;
	default:
		return SemanticApi::SpecialClassMethodType::NotSpecial;
	}
}

TSMethod::TSMethod(TSClass* use_owner, SyntaxApi::IMethod* use_syntax)
	:TSyntaxNode(use_syntax), TSpecialClassMethod(GetMethodTypeFromSyntax(use_syntax))
{
	_this.reset(new TPrivate(use_owner, use_syntax->GetRetType()));
	_this->owner = use_owner;
	_this->has_return = use_syntax->HasReturn();
	_this->ret_ref = use_syntax->IsReturnRef();
	
}

TSMethod::TSMethod(TSClass* use_owner, SemanticApi::SpecialClassMethodType special_method_type)
	:TSyntaxNode(nullptr), TSpecialClassMethod(special_method_type)
{
	_this.reset(new TPrivate(use_owner, (TSClass*)nullptr));
	_this->owner = use_owner;
	SetBodyLinked();
	SetSignatureLinked();
	_this->has_return=false;
	_this->ret_ref=false;
}

TSMethod::~TSMethod()
{
}

void TSMethod::CopyExternalMethodBindingsFrom(TSMethod* source)
{
	_this->external_func = source->_this->external_func;
}

SemanticApi::TExternalSMethod TSMethod::GetExternal() const
{
	assert(_this->is_external);
	return _this->external_func;
}

bool TSMethod::IsExternal() const
{
	return _this->is_external;
}

void TSMethod::AddParameter(TSParameter* use_par)
{
	assert(GetType() != SemanticApi::SpecialClassMethodType::NotSpecial);
	_this->parameters.push_back(std::unique_ptr<TSParameter>(use_par));
}

void TSMethod::LinkSignature(SemanticApi::TGlobalBuildContext build_context)
{
	if (IsSignatureLinked())
		return;
	SetSignatureLinked();
	if(GetSyntax()->HasReturn())
		_this->ret.LinkSignature(build_context);
	for (const std::unique_ptr<TSParameter>& v : _this->parameters)
	{
		v->LinkSignature(build_context);
	}
	
}

TSClass* TSMethod::GetRetClass()const
{
	if (_this->has_return)
		return dynamic_cast<TSClass*>(_this->ret.GetClass());
	else
		return nullptr;
}

TSParameter* TSMethod::GetParam(int id)const
{
	return _this->parameters[id].get();
}
std::vector<TSParameter*>  TSMethod::GetParameters()const
{
	std::vector<TSParameter*> result;
	result.resize(_this->parameters.size());
	for (size_t i = 0; i < _this->parameters.size(); i++)
		result[i] = _this->parameters[i].get();
	return result;
}
size_t TSMethod::GetParamsCount()const
{
	return _this->parameters.size();
}

bool TSMethod::HasParams(const std::vector<TSParameter*> &use_params)const
{
	if (use_params.size() != _this->parameters.size())
		return false;
	for (size_t i = 0; i<_this->parameters.size(); i++)
		if (!_this->parameters[i]->IsEqualTo(*(use_params[i])))
			return false;
	return true;
}

void TSMethod::LinkBody(SemanticApi::TGlobalBuildContext build_context)
{
	if (_this->is_external)
		return;
	if (IsBodyLinked())
		return;
	SetBodyLinked();
	_this->statements = std::unique_ptr<TSStatements>(new TSStatements(_this->owner, this, nullptr, GetSyntax()->GetStatements()));
	_this->statements->Build(build_context);
	//if (!GetSyntax()->IsBytecode())
	//	statements->Build();

	if (GetSyntax()->HasReturn())
		_this->ret.LinkBody(build_context);
	for (const std::unique_ptr<TSParameter>& v : _this->parameters)
	{
		v->LinkBody(build_context);
	}
}

void TSMethod::CalculateParametersOffsets()
{
	_this->parameters_size = 0;
	for (size_t i = 0; i<_this->parameters.size(); i++)
	{
		if (!_this->parameters[i]->IsOffsetInitialized())
		{
			//parameters[i]->SetOffset(parameters_size);
			_this->parameters[i]->SetOffset(i);
			_this->parameters[i]->CalculateSize();
			_this->parameters_size += _this->parameters[i]->GetSize();
		}
	}
	if (GetRetClass() != nullptr)
	{
		if (GetSyntax()->IsReturnRef())
			_this->ret_size = 1;
		else
			_this->ret_size = _this->ret.GetClass()->GetSize();
	}
	else
		_this->ret_size = 0;
}

bool TSMethod::IsReturnRef() const
{
	return this->GetSyntax()->IsReturnRef();
}

SemanticApi::ISStatements * TSMethod::GetStatements() const
{
	return _this->statements.get();
}

void TSMethod::Build()
{
	for (size_t i = 0; i < GetSyntax()->GetParamsCount(); i++)
	{
		SyntaxApi::IParameter* v = GetSyntax()->GetParam(i);
		_this->parameters.push_back(std::unique_ptr<TSParameter>(new TSParameter(_this->owner, this, v, v->GetType())));
	}
	_this->is_external = GetSyntax()->IsExternal();
}

void TSMethod::CheckForErrors()
{
	auto& owner = _this->owner;
	auto& parameters = _this->parameters;

	if (owner->GetOwner() == nullptr&&!GetSyntax()->IsStatic())
		GetSyntax()->Error("Базовый класс может содержать только статические методы!");
	for (size_t i = 0; i<parameters.size(); i++)
	{
		if (!parameters[i]->GetSyntax()->GetName().IsNull())
			for (size_t k = 0; k<i; k++)
			{
			if (parameters[i]->GetSyntax()->GetName() == parameters[k]->GetSyntax()->GetName())
				parameters[i]->GetSyntax()->Error("Параметр с таким именем уже существует!");
			}
	}
	if (!GetSyntax()->GetName().IsNull())
	{
		if (owner->GetClass(GetSyntax()->GetName()) != nullptr)
			GetSyntax()->Error("Класс не может быть именем метода!");
		if (owner->GetField(GetSyntax()->GetName(), false) != nullptr)
			GetSyntax()->Error("Член класса с таким именем уже существует!");
		//TODO проверить члены родительского класса и т.д. (полный запрет на перекрытие имен)
	}
	switch ((Lexer::TResWord)GetSyntax()->GetMemberType())
	{
	case Lexer::TResWord::Func:
		assert(!GetSyntax()->GetName().IsNull());
		break;
	case Lexer::TResWord::Default:
	case Lexer::TResWord::Copy:
	case Lexer::TResWord::Move:
		if (GetSyntax()->IsStatic())GetSyntax()->Error("Конструктор должен быть не статичным!");
		break;
	case Lexer::TResWord::Destr:
		if (GetSyntax()->IsStatic())GetSyntax()->Error("Деструктор должен быть не статичным!");
		break;
	case Lexer::TResWord::Operator:
		if (!GetSyntax()->IsStatic())GetSyntax()->Error("Оператор должен быть статичным!");
		break;
	case Lexer::TResWord::Conversion:
		if (!GetSyntax()->IsStatic())GetSyntax()->Error("Оператор приведения типа должен быть статичным!");
		break;
	default:assert(false);
	}
	{
		//проверяем правильность указания параметров и возвращаемого значения
		switch (GetSyntax()->GetMemberType())
		{
		case SyntaxApi::TClassMember::Func:
			break;
		case SyntaxApi::TClassMember::DefaultConstr:
			if (_this->ret.GetClass() != nullptr)GetSyntax()->Error("Конструктор по умолчанию не должен возвращать значение!");
			if (parameters.size() != 0)GetSyntax()->Error("Конструктор по умолчанию не имеет параметров!");
			break;
		case SyntaxApi::TClassMember::CopyConstr:
		case SyntaxApi::TClassMember::MoveConstr:
			if (_this->ret.GetClass() != nullptr)GetSyntax()->Error("Конструктор не должен возвращать значение!");
			break;
		case SyntaxApi::TClassMember::Destr:
			if (_this->ret.GetClass() != nullptr)GetSyntax()->Error("Деструктор не должен возвращать значение!");
			if (parameters.size() != 0)GetSyntax()->Error("Деструктор не имеет параметров!");
			break;
		case SyntaxApi::TClassMember::Operator:
			if (GetSyntax()->GetOperatorType() == Lexer::TOperator::Not)//унарные операторы
			{
				if (GetParamsCount() != 1)
					GetSyntax()->Error("Унарный оператор должен иметь один параметр!");
				if (GetParam(0)->GetClass() != owner)
					GetSyntax()->Error("Хотя бы один из параметров оператора должен быть классом для которого он используется!");
			}
			else if (GetSyntax()->GetOperatorType() == Lexer::TOperator::UnaryMinus)
			{
				if (!LexerIsIn((int)GetParamsCount(), 1, 2))
					GetSyntax()->Error("У унарного оператора ""-"" должнен быть 1 параметр!");
				if (GetParam(0)->GetClass() != owner
					&& (GetParamsCount() == 2 && GetParam(1)->GetClass() != owner))
					GetSyntax()->Error("Параметром унарного оператора должен быть класс для которого он используется!");
			}
			else if (GetSyntax()->GetOperatorType() == Lexer::TOperator::ParamsCall || GetSyntax()->GetOperatorType() == Lexer::TOperator::GetArrayElement)
			{
				if (GetParamsCount()<2)
					GetSyntax()->Error("Оператор вызова параметров должен иметь минимум 2 операнда!");
				if (GetParam(0)->GetClass() != owner)
					GetSyntax()->Error("Первый параметр оператора вызова должен быть данным классом!");
			}
			else //остальные бинарные операторы
			{
				if ((GetSyntax()->GetOperatorType() == Lexer::TOperator::Equal || GetSyntax()->GetOperatorType() == Lexer::TOperator::NotEqual)
					&& _this->ret.GetClass() != owner->GetClass(GetSyntax()->GetLexer()->GetIdFromName("bool")))
					GetSyntax()->Error("Оператор сравнения должен возвращать логическое значение!");
				if (GetParamsCount() != 2)
					GetSyntax()->Error("У бинарного оператора должно быть 2 параметра!");
				if (GetParam(0)->GetClass() != owner
					&& (GetParamsCount() == 2 && GetParam(1)->GetClass() != owner))
					GetSyntax()->Error("Хотя бы один из параметров оператора должен быть классом для которого он используется!");
			}
			break;
		case SyntaxApi::TClassMember::Conversion:
			if (GetParamsCount() != 1
				|| GetParam(0)->GetClass() != owner)
				GetSyntax()->Error("Оператор приведения типа должен иметь один параметр с типом равным классу в котором он находится!");
			break;
		default:assert(false);
		}
	}
}

SemanticApi::IVariable* TSMethod::GetVar(Lexer::TNameId name)const
{
	for (size_t i = 0; i<_this->parameters.size(); i++)
		if (_this->parameters[i]->GetSyntax()->GetName() == name)
			return (TVariable*)_this->parameters[i].get();
	return (TVariable*)_this->owner->GetField(name, false);
}

TSClass* TSMethod::GetOwner()const
{
	return _this->owner;
}

void TSMethod::SetAsExternal(SemanticApi::TExternalSMethod method)
{
	assert(_this->external_func == nullptr);
	_this->external_func = method;
	_this->is_external = true;
}

size_t TSMethod::GetParametersSize()const
{
	return _this->parameters_size;
}

size_t TSMethod::GetReturnSize()const
{
	return _this->ret_size;
}