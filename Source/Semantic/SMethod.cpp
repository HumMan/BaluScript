#include "SMethod.h"

#include <common.h>
#include <assert.h>

#include "SStatements.h"
#include "SOverloadedMethod.h"
#include "FormalParam.h"
#include "SType.h"
#include "SClass.h"

#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"

TSpecialClassMethod::Type GetMethodTypeFromSyntax(TMethod* use_syntax)
{
	switch (use_syntax->GetMemberType())
	{
	case TClassMember::DefaultConstr:
		return TSpecialClassMethod::Default;
	case TClassMember::CopyConstr:
		return TSpecialClassMethod::CopyConstr;
	case TClassMember::Destr:
		return TSpecialClassMethod::Destructor;
	default:
		return TSpecialClassMethod::NotSpecial;
	}
}

TSMethod::TSMethod(TSClass* use_owner, TMethod* use_syntax)
	:TSyntaxNode(use_syntax), ret(use_owner, use_syntax->GetRetType()), TSpecialClassMethod(GetMethodTypeFromSyntax(use_syntax))
{
	owner = use_owner;
	is_external = false;
	external_func = NULL;
	has_return = use_syntax->has_return;
	ret_ref = use_syntax->IsReturnRef();
	
}

TSMethod::TSMethod(TSClass* use_owner, TSpecialClassMethod::Type special_method_type)
	:TSyntaxNode(NULL), ret(use_owner, (TSClass*)NULL), TSpecialClassMethod(special_method_type)
{
	owner = use_owner;
	SetBodyLinked();
	SetSignatureLinked();
	is_external = false;
	has_return=false;
	ret_ref=false;
}

TSMethod::~TSMethod()
{
}

void TSMethod::CopyExternalMethodBindingsFrom(TSMethod* source)
{
	external_func = source->external_func;
}

void TSMethod::AddParameter(TSParameter* use_par)
{
	assert(GetType() != TSpecialClassMethod::NotSpecial);
	parameters.push_back(std::unique_ptr<TSParameter>(use_par));
}

void TSMethod::LinkSignature(TGlobalBuildContext build_context)
{
	if (IsSignatureLinked())
		return;
	SetSignatureLinked();
	if(GetSyntax()->has_return)
		ret.LinkSignature(build_context);
	for (const std::unique_ptr<TSParameter>& v : parameters)
	{
		v->LinkSignature(build_context);
	}
	
}

TSClass* TSMethod::GetRetClass()
{
	if (has_return)
		return ret.GetClass();
	else
		return NULL;
}

TSParameter* TSMethod::GetParam(int id)
{
	return parameters[id].get();
}

int TSMethod::GetParamsCount()
{
	return parameters.size();
}

bool TSMethod::HasParams(std::vector<std::unique_ptr<TSParameter>> &use_params)const
{
	if (use_params.size() != parameters.size())
		return false;
	for (size_t i = 0; i<parameters.size(); i++)
		if (!parameters[i]->IsEqualTo(*(use_params[i])))
			return false;
	return true;
}

void TSMethod::LinkBody(TGlobalBuildContext build_context)
{
	if (is_external)
		return;
	if (IsBodyLinked())
		return;
	SetBodyLinked();
	statements = std::unique_ptr<TSStatements>(new TSStatements(owner, this, NULL, GetSyntax()->statements.get()));
	statements->Build(build_context);
	//if (!GetSyntax()->IsBytecode())
	//	statements->Build();

	if (GetSyntax()->has_return)
		ret.LinkBody(build_context);
	for (const std::unique_ptr<TSParameter>& v : parameters)
	{
		v->LinkBody(build_context);
	}
}

void TSMethod::CalculateParametersOffsets()
{
	parameters_size = 0;
	for (size_t i = 0; i<parameters.size(); i++)
	{
		if (!parameters[i]->IsOffsetInitialized())
		{
			//parameters[i]->SetOffset(parameters_size);
			parameters[i]->SetOffset(i);
			parameters[i]->CalculateSize();
			parameters_size += parameters[i]->GetSize();
		}
	}
	if (GetRetClass() != NULL)
	{
		if (GetSyntax()->IsReturnRef())
			ret_size = 1;
		else
			ret_size = ret.GetClass()->GetSize();
	}
	else
		ret_size = 0;
}

void TSMethod::Run(TMethodRunContext method_run_context)
{
	if (is_external)
	{
		external_func(method_run_context);
	}
	else
	{
		bool result_returned = false;

		std::vector<TStackValue> local_variables;

		TStatementRunContext run_context;
		*(TMethodRunContext*)&run_context = method_run_context;
		run_context.result_returned = &result_returned;
		run_context.local_variables = &local_variables;

		if (GetType() == TSpecialClassMethod::NotSpecial)
		{
			statements->Run(run_context);
			//TODO заглушка для отслеживания завершения метода без возврата значения
			//if (has_return)
			//	assert(result_returned);
		}
		else
		{
			switch (GetType())
			{
			case TSpecialClassMethod::AutoDefConstr:
			{
				owner->RunAutoDefConstr(*run_context.static_fields, *run_context.object);
			}break;
			case TSpecialClassMethod::AutoCopyConstr:
			{
				owner->RunAutoCopyConstr(*run_context.static_fields, *run_context.formal_params, *run_context.object);
			}break;
			case TSpecialClassMethod::AutoDestructor:
			{
				owner->RunAutoDestr(*run_context.static_fields, *run_context.object);
			}break;
			case TSpecialClassMethod::Default:
			{
				if (owner->auto_def_constr)
					owner->RunAutoDefConstr(*run_context.static_fields, *run_context.object);
				statements->Run(run_context);
			}break;
			case TSpecialClassMethod::CopyConstr:
			{
				if (owner->auto_def_constr)
					owner->RunAutoDefConstr(*run_context.static_fields, *run_context.object);
				statements->Run(run_context);
			}break;
			case TSpecialClassMethod::Destructor:
			{
				statements->Run(run_context);
				if (owner->auto_destr)
					owner->RunAutoDestr(*run_context.static_fields, *run_context.object);
			}break;
			case TSpecialClassMethod::AutoAssignOperator:
			{
				owner->RunAutoAssign(*run_context.static_fields, *run_context.formal_params);
			}break;
			default:
				assert(false);
			}
		}
	}
	//statements->Run(sp,result_returned,&sp[-GetParametersSize()->GetRuturnSize()]);

	//int locals_size=0;
	//result_result.GetOps()+=BuildLocalsAndParamsDestructor(program,locals_size);
	//if(method->GetMemberType()==TClassMember::Destr)
	//{
	//	TMethod* auto_destr=owner->GetAutoDestructor();
	//	if(auto_destr!=NULL)
	//	{
	//		program.Push(TOp(TOpcode::PUSH_THIS),result_result.GetOps());
	//		result_result.GetOps()+=auto_destr->BuildCall(program).GetOps();
	//	}
	//}
	//program.Push(TOp(TOpcode::RETURN,
	//	method->GetParamsSize()+locals_size+!method->IsStatic(),method->GetRetSize()),result_result.GetOps());
	//method->SetHasReturn(true);
	//return TVoid();
}

void TSMethod::Build()
{
	for (const std::unique_ptr<TParameter>& v : GetSyntax()->parameters)
	{
		parameters.push_back(std::unique_ptr<TSParameter>(new TSParameter(owner, this, v.get(), v->type.get())));
	}
	is_external = GetSyntax()->is_extern;
}

void TSMethod::CheckForErrors()
{
	if (owner->GetOwner() == NULL&&!GetSyntax()->IsStatic())
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
	case Lexer::TResWord::Func:
		assert(!GetSyntax()->method_name.IsNull());
		break;
	case Lexer::TResWord::Default:
	case Lexer::TResWord::Copy:
	case Lexer::TResWord::Move:
		if (GetSyntax()->is_static)GetSyntax()->Error("Конструктор должен быть не статичным!");
		break;
	case Lexer::TResWord::Destr:
		if (GetSyntax()->is_static)GetSyntax()->Error("Деструктор должен быть не статичным!");
		break;
	case Lexer::TResWord::Operator:
		if (!GetSyntax()->is_static)GetSyntax()->Error("Оператор должен быть статичным!");
		break;
	case Lexer::TResWord::Conversion:
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
		case Lexer::TResWord::Default:
			if (ret.GetClass() != NULL)GetSyntax()->Error("Конструктор по умолчанию не должен возвращать значение!");
			if (parameters.size() != 0)GetSyntax()->Error("Конструктор по умолчанию не имеет параметров!");
			break;
		case Lexer::TResWord::Copy:
		case Lexer::TResWord::Move:
			if (ret.GetClass() != NULL)GetSyntax()->Error("Конструктор не должен возвращать значение!");
			break;
		case TClassMember::Destr:
			if (ret.GetClass() != NULL)GetSyntax()->Error("Деструктор не должен возвращать значение!");
			if (parameters.size() != 0)GetSyntax()->Error("Деструктор не имеет параметров!");
			break;
		case TClassMember::Operator:
			if (GetSyntax()->operator_type == Lexer::TOperator::Not)//унарные операторы
			{
				if (GetParamsCount() != 1)
					GetSyntax()->Error("Унарный оператор должен иметь один параметр!");
				if (GetParam(0)->GetClass() != owner)
					GetSyntax()->Error("Хотя бы один из параметров оператора должен быть классом для которого он используется!");
			}
			else if (GetSyntax()->operator_type == Lexer::TOperator::UnaryMinus)
			{
				if (!LexerIsIn(GetParamsCount(), 1, 2))
					GetSyntax()->Error("У унарного оператора ""-"" должнен быть 1 параметр!");
				if (GetParam(0)->GetClass() != owner
					&& (GetParamsCount() == 2 && GetParam(1)->GetClass() != owner))
					GetSyntax()->Error("Параметром унарного оператора должен быть класс для которого он используется!");
			}
			else if (GetSyntax()->operator_type == Lexer::TOperator::ParamsCall || GetSyntax()->operator_type == Lexer::TOperator::GetArrayElement)
			{
				if (GetParamsCount()<2)
					GetSyntax()->Error("Оператор вызова параметров должен иметь минимум 2 операнда!");
				if (GetParam(0)->GetClass() != owner)
					GetSyntax()->Error("Первый параметр оператора вызова должен быть данным классом!");
			}
			else //остальные бинарные операторы
			{
				if ((GetSyntax()->operator_type == Lexer::TOperator::Equal || GetSyntax()->operator_type == Lexer::TOperator::NotEqual)
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

TVariable* TSMethod::GetVar(Lexer::TNameId name)
{
	for (size_t i = 0; i<parameters.size(); i++)
		if (parameters[i]->GetSyntax()->GetName() == name)
			return (TVariable*)parameters[i].get();
	return (TVariable*)owner->GetField(name, false);
}

TSClass* TSMethod::GetOwner()const
{
	return owner;
}