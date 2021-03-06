﻿#include "Method.h"

#include "Statements.h"
#include "Parameter.h"
#include "Type.h"

#include <common.h>

using namespace Lexer;
using namespace SyntaxInternal;

void TMethod::ParametersDecl(Lexer::ILexer* source) {
	//Считываем все параметры метода и возвращаемое значение
	if (source->TestAndGet(TOperator::ParamsCall)) {
	} else if (source->Test(TTokenType::LParenth)) {
		source->GetToken(TTokenType::LParenth);
		while (source->Test(TTokenType::Identifier)) {
			TParameter* t = new TParameter(owner, this);
			parameters.push_back(std::unique_ptr<TParameter>(t));
			t->AnalyzeSyntax(source);
			if (source->Test(TTokenType::Comma))
				source->GetToken();
			else
				break;
		}
		source->GetToken(TTokenType::RParenth);
	}
	if (source->TestAndGet(TTokenType::Colon))
	{
		ret_ref = source->TestAndGet(TTokenType::Ampersand);//TODO проверка ссылки после типа а то не понятно почему ошибка
		ret->AnalyzeSyntax(source);
		has_return = true;
	}
	else
		has_return = false;
}

void TMethod::AnalyzeSyntax(Lexer::ILexer* source, bool realization) {
	InitPos(source);
	source->TestToken(TTokenType::ResWord);
	member_type = (SyntaxApi::TClassMember) source->Token();
	if (!LexerIsIn(member_type, SyntaxApi::TClassMember::Func, SyntaxApi::TClassMember::Conversion))
		source->Error(
				"Ожидалось объявление метода,конструктора,деструктора,оператора или приведения типа!");
	source->GetToken();
	if(owner->IsExternal())
		is_extern = true;
	else
		is_extern = source->TestAndGet(TResWord::Extern);
	is_static = source->TestAndGet(TResWord::Static);
	switch (member_type) {
	case SyntaxApi::TClassMember::Func:
	case SyntaxApi::TClassMember::DefaultConstr:
	case SyntaxApi::TClassMember::CopyConstr:
	case SyntaxApi::TClassMember::Destr:
	case SyntaxApi::TClassMember::Operator:
	case SyntaxApi::TClassMember::Conversion:
		break;
	default:
		source->Error(
				"Ожидалось объявление метода,конструктора,деструктора,оператора или приведения типа!");
	}
	//Если это не реализация метода, а поиск по названию, то в начале может стоять название класса
	if (!realization) 
	{
		if (source->NameId() != owner->GetName())
			Error("Ожидалось имя базового класса!");
		source->GetToken();
		while (source->Test(TTokenType::Dot)) 
		{
			source->GetToken(TTokenType::Dot);
			if (!source->Test(TTokenType::Identifier))
				break;
			TClass* t = owner->GetNested(source->NameId());
			if (t != nullptr) 
			{
				source->GetToken();
				owner = t;
			} else
				break;
		}
	}
	//
	if (member_type == SyntaxApi::TClassMember::Func) {
		source->TestToken(TTokenType::Identifier);
		method_name = source->NameId();
		source->GetToken();
	}
	else if (member_type == SyntaxApi::TClassMember::Operator) {
		source->TestToken(TTokenType::Operator);
		operator_type = (TOperator) source->Token();
		source->GetToken();
	}
	{
		ParametersDecl(source);
		if (realization) {
			switch (member_type) {
			case SyntaxApi::TClassMember::Func:
				owner->AddMethod(this, method_name);
				break;
			case SyntaxApi::TClassMember::DefaultConstr:
				owner->AddDefaultConstr(this);
				break;
			case SyntaxApi::TClassMember::CopyConstr:
				owner->AddCopyConstr(this);
				break;
			case SyntaxApi::TClassMember::Destr:
				owner->AddDestr(this);
				break;
			case SyntaxApi::TClassMember::Operator:
				if (operator_type == TOperator::Minus || operator_type
						== TOperator::UnaryMinus) {
					if (GetParamsCount() == 1)
						operator_type = TOperator::UnaryMinus;
					else
						operator_type = TOperator::Minus;
				}
				owner->AddOperator(operator_type, this);
				break;
			case SyntaxApi::TClassMember::Conversion:
				owner->AddConversion(this);
				break;
			default:
			assert(false);
			}
		}
	}
	if (is_extern && realization) {
		if (!source->Test(TTokenType::Semicolon))
			source->Error("Для внешнего метода тело должно отсутствовать!");
		source->GetToken(TTokenType::Semicolon);
	} else if (realization) {
		one_instruction = !source->Test(TTokenType::LBrace);
		statements->AnalyzeSyntax(source);
	}
}

TMethod::TMethod(TClass* use_owner, SyntaxApi::TClassMember use_member_type)
	:ret(new TType(use_owner)), ret_ref(false), owner(use_owner)
	, is_static(false), is_extern(false)
	, statements(new TStatements(use_owner, this, nullptr, -1))
	, has_return(false)
	, one_instruction(false)
	, operator_type(TOperator::End)
	, member_type(use_member_type)
{
}

TMethod::~TMethod()
{

}

void TMethod::SetHasReturn(bool use_has_return)
{
	has_return = use_has_return;
}
bool TMethod::HasReturn()const
{
	return has_return;
}
TStatements* TMethod::GetStatementsT()const
{
	return statements.get();
}
TNameId TMethod::GetName()const
{
	return method_name;
}
SyntaxApi::IClass* TMethod::GetOwner()const
{
	return owner;
}
TOperator TMethod::GetOperatorType()const
{
	assert(member_type == SyntaxApi::TClassMember::Operator);
	return operator_type;
}
SyntaxApi::TClassMember TMethod::GetMemberType()const
{
	return member_type;
}
TParameter* TMethod::GetParamT(size_t use_id)const
{
	return parameters[use_id].get();
}
size_t TMethod::GetParamsCount()const
{
	return parameters.size();
}
bool TMethod::IsReturnRef()const
{
	return ret_ref;
}
bool TMethod::IsStatic()const
{
	return is_static;
}
bool TMethod::IsExternal()const
{
	return is_extern;
}
SyntaxApi::IStatements* TMethod::GetStatements()const
{
	return statements.get();
}
SyntaxApi::IParameter* TMethod::GetParam(size_t use_id)const
{
	return parameters[use_id].get();
}
SyntaxApi::IType* TMethod::GetRetType()const
{
	return ret.get();
}
TType* TMethod::GetRetTypeT()const
{
	return ret.get();
}
//bool TMethod::IsBytecode()
//{
//		return one_instruction
//		&&statements->GetHigh() == 0
//		&& statements->GetStatement(0)->GetType() == SyntaxApi::TStatementType::Bytecode;
//}
void TMethod::AddParam(TParameter* use_param)
{
	parameters.push_back(std::unique_ptr<TParameter>(use_param));
}

void TMethod::CheckForErrors()
{
	//TODO - всё уже есть в TSMethod
	//if (owner->GetOwner() == nullptr&&!is_static)
	//	Error("Базовый класс может содержать только статические методы!");
	//for (size_t i = 0; i<parameters.size(); i++)
	//{
	//	if (!parameters[i]->GetName().IsNull())
	//		for (size_t k = 0; k<i; k++)
	//		{
	//		if (parameters[i]->GetName() == parameters[k]->GetName())
	//			parameters[i]->Error("Параметр с таким именем уже существует!");
	//		}
	//}
	//switch (member_type)
	//{
	//case TResWord::Func:
	//	assert(!method_name.IsNull());
	//	break;
	//case TResWord::Constr:
	//	if (is_static)Error("Конструктор должен быть не статичным!");
	//	break;
	//case TResWord::Destr:
	//	if (is_static)Error("Деструктор должен быть не статичным!");
	//	break;
	//case TResWord::Operator:
	//	if (!is_static)Error("Оператор должен быть статичным!");
	//	break;
	//case TResWord::Conversion:
	//	if (!is_static)Error("Оператор приведения типа должен быть статичным!");
	//	break;
	//default:assert(false);
	//}
	//{
	//	//проверяем правильность указания параметров и возвращаемого значения
	//	switch (member_type)
	//	{
	//	case TClassMember::Func:
	//		break;
	//	case TClassMember::Constr:
	//		if (has_return)
	//			Error("Конструктор не должен возвращать значение!");
	//		break;
	//	case TClassMember::Destr:
	//		if (has_return)
	//			Error("Деструктор не должен возвращать значение!");
	//		if (parameters.size()>0)
	//			Error("Деструктор не имеет параметров!");
	//		break;
	//	case TClassMember::Operator:
	//		if (operator_type == TOperator::Not)//унарные операторы
	//		{
	//			if (GetParamsCount() != 1)
	//				Error("Унарный оператор должен иметь один параметр!");
	//		}
	//		else if (operator_type == TOperator::UnaryMinus)
	//		{
	//			if (!IsIn(GetParamsCount(), 1, 2))
	//				Error("У унарного оператора ""-"" должнен быть 1 параметр!");
	//		}
	//		else if (operator_type == TOperator::ParamsCall || operator_type == TOperator::GetArrayElement)
	//		{
	//			if (GetParamsCount()<2)
	//				Error("Оператор вызова параметров должен иметь минимум 2 операнда!");
	//		}
	//		else //остальные бинарные операторы
	//		{
	//			if (GetParamsCount() != 2)
	//				Error("У бинарного оператора должно быть 2 параметра!");
	//		}
	//		break;
	//	case TClassMember::Conversion:
	//		if (GetParamsCount() != 1)
	//			Error("Оператор приведения типа должен иметь один параметр!");
	//		break;
	//	default:assert(false);
	//	}
	//}
}
