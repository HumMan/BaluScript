#include "SMethod.h"

#include <assert.h>

#include "SStatements.h"
#include "SOverloadedMethod.h"

TSMethod::TSMethod(TSClass* use_owner, TMethod* use_syntax)
	:TSyntaxNode(use_syntax), ret(use_owner, use_syntax->GetRetType()),
	linked_signature(false), linked_body(false)
{
	owner = use_owner;
}

void TSMethod::LinkSignature()
{
	if (linked_signature)
		return;
	linked_signature = true;
	if(GetSyntax()->has_return)
		ret.Link();
	for (const std::shared_ptr<TSParameter>& v : parameters)
	{
		v->Link();
	}
	
}

TSClass* TSMethod::GetRetClass()
{
	return ret.GetClass();
}

TSParameter* TSMethod::GetParam(int id)
{
	return parameters[id].get();
}

int TSMethod::GetParamsCount()
{
	return parameters.size();
}

bool TSMethod::HasParams(std::vector<std::shared_ptr<TSParameter>> &use_params)const
{
	if (use_params.size() != parameters.size())
		return false;
	for (int i = 0; i<parameters.size(); i++)
		if (!parameters[i]->IsEqualTo(*(use_params[i])))
			return false;
	return true;
}

void TSMethod::LinkBody()
{
	if (linked_body)
		return;
	linked_body = true;
	statements->Link();
}

void TSMethod::Build()
{
	for (const std::unique_ptr<TParameter>& v : GetSyntax()->parameters)
	{
		parameters.push_back(std::shared_ptr<TSParameter>(new TSParameter(owner, this, v.get(), &v->type)));
	}
}


void TSMethod::CheckForErrors()
{
	if (owner->GetOwner() == NULL&&!GetSyntax()->IsStatic())
		GetSyntax()->Error("Базовый класс может содержать только статические методы!");
	for (int i = 0; i<parameters.size(); i++)
	{
		if (!parameters[i]->GetSyntax()->GetName().IsNull())
			for (int k = 0; k<i; k++)
			{
			if (parameters[i]->GetSyntax()->GetName() == parameters[k]->GetSyntax()->GetName())
				parameters[i]->GetSyntax()->Error("Параметр с таким именем уже существует!");
			}
	}
	if (!GetSyntax()->method_name.IsNull())
	{
		if (owner->GetClass(GetSyntax()->method_name) != NULL)
			GetSyntax()->Error("Класс не может быть именем метода!");
		if (owner->GetField(GetSyntax()->method_name, false) != NULL)
			GetSyntax()->Error("Член класса с таким именем уже существует!");
		//TODO проверить члены родительского класса и т.д. (полный запрет на перекрытие имен)
	}
	switch (GetSyntax()->member_type)
	{
	case TResWord::Func:
		assert(!GetSyntax()->method_name.IsNull());
		break;
	case TResWord::Constr:
		if (GetSyntax()->is_static)GetSyntax()->Error("Конструктор должен быть не статичным!");
		break;
	case TResWord::Destr:
		if (GetSyntax()->is_static)GetSyntax()->Error("Деструктор должен быть не статичным!");
		break;
	case TResWord::Operator:
		if (!GetSyntax()->is_static)GetSyntax()->Error("Оператор должен быть статичным!");
		break;
	case TResWord::Conversion:
		if (!GetSyntax()->is_static)GetSyntax()->Error("Оператор приведения типа должен быть статичным!");
		break;
	default:assert(false);
	}
	{
		//проверяем правильность указания параметров и возвращаемого значения
		switch (GetSyntax()->member_type)
		{
		case TClassMember::Func:
			break;
		case TClassMember::Constr:
			if (ret.GetClass() != NULL)GetSyntax()->Error("Конструктор не должен возвращать значение!");
			break;
		case TClassMember::Destr:
			if (ret.GetClass() != NULL)GetSyntax()->Error("Деструктор не должен возвращать значение!");
			if (parameters.size() != 0)GetSyntax()->Error("Деструктор не имеет параметров!");
			break;
		case TClassMember::Operator:
			if (GetSyntax()->operator_type == TOperator::Not)//унарные операторы
			{
				if (GetParamsCount() != 1)
					GetSyntax()->Error("Унарный оператор должен иметь один параметр!");
				if (GetParam(0)->GetClass() != owner)
					GetSyntax()->Error("Хотя бы один из параметров оператора должен быть классом для которого он используется!");
			}
			else if (GetSyntax()->operator_type == TOperator::UnaryMinus)
			{
				if (!IsIn(GetParamsCount(), 1, 2))
					GetSyntax()->Error("У унарного оператора ""-"" должнен быть 1 параметр!");
				if (GetParam(0)->GetClass() != owner
					&& (GetParamsCount() == 2 && GetParam(1)->GetClass() != owner))
					GetSyntax()->Error("Параметром унарного оператора должен быть класс для которого он используется!");
			}
			else if (GetSyntax()->operator_type == TOperator::ParamsCall || GetSyntax()->operator_type == TOperator::GetArrayElement)
			{
				if (GetParamsCount()<2)
					GetSyntax()->Error("Оператор вызова параметров должен иметь минимум 2 операнда!");
				if (GetParam(0)->GetClass() != owner)
					GetSyntax()->Error("Первый параметр оператора вызова должен быть данным классом!");
			}
			else //остальные бинарные операторы
			{
				if ((GetSyntax()->operator_type == TOperator::Equal || GetSyntax()->operator_type == TOperator::NotEqual)
					&& ret.GetClass() != owner->GetClass(GetSyntax()->source->GetIdFromName("bool")))
					GetSyntax()->Error("Оператор сравнения должен возвращать логическое значение!");
				if (GetParamsCount() != 2)
					GetSyntax()->Error("У бинарного оператора должно быть 2 параметра!");
				if (GetParam(0)->GetClass() != owner
					&& (GetParamsCount() == 2 && GetParam(1)->GetClass() != owner))
					GetSyntax()->Error("Хотя бы один из параметров оператора должен быть классом для которого он используется!");
			}
			break;
		case TClassMember::Conversion:
			if (GetParamsCount() != 1
				|| GetParam(0)->GetClass() != owner)
				GetSyntax()->Error("Оператор приведения типа должен иметь один параметр с типом равным классу в котором он находится!");
			break;
		default:assert(false);
		}
	}
}