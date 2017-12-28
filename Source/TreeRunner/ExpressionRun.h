#pragma once

void TSExpression::TInt::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, type.GetClass());
	run_context.expression_result->get_as<int>() = val;
}

void TSExpression::TFloat::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, type.GetClass());
	run_context.expression_result->get_as<float>() = val;
}

void TSExpression::TBool::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, type.GetClass());
	run_context.expression_result->get_as<int>() = val;
}
void TSExpression::TString::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, type.GetClass());
	run_context.expression_result->get_as< ::TString>().Init(val);
}

void TSExpression::TEnumValue::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, type);
	run_context.expression_result->get_as<int>() = val;
}

void TSExpression_TMethodCall::Run(TExpressionRunContext run_context)
{
	//соглашение о вызовах - вызывающий инициализирует параметры, резервирует место для возвращаемого значения, вызывает деструкторы параметров
	std::vector<TStackValue> method_call_formal_params;
	params.Construct(method_call_formal_params, run_context);
	switch (type)
	{
		//case TSExpression_TMethodCall::DefaultAssignOperator:
		//{
		//	memcpy(method_call_formal_params[0].get(), method_call_formal_params[1].get(), method_call_formal_params[0].GetClass()->GetSize()*sizeof(int));
		//}break;

	case TSExpression_TMethodCall::Method:
	{
		TStackValue left_result;
		if (left != nullptr)
			left->Run(TExpressionRunContext(run_context, &left_result));

		if (invoke->GetRetClass() != nullptr)
			*run_context.expression_result = TStackValue(invoke->GetSyntax()->IsReturnRef(), invoke->GetRetClass());

		invoke->Run(TMethodRunContext(run_context.static_fields, &method_call_formal_params, run_context.expression_result, &left_result));
	}break;

	case TSExpression_TMethodCall::Operator:
	{
		if (invoke->GetRetClass() != nullptr)
			*run_context.expression_result = TStackValue(invoke->GetSyntax()->IsReturnRef(), invoke->GetRetClass());
		invoke->Run(TMethodRunContext(run_context.static_fields, &method_call_formal_params, run_context.expression_result, nullptr));
	}break;

	case TSExpression_TMethodCall::ObjectConstructor:
	{
		invoke->Run(TMethodRunContext(run_context.static_fields, &method_call_formal_params, nullptr, run_context.object));
	}break;

	default:
		assert(false);
	}
	params.Destroy(method_call_formal_params, run_context);
}

void TSExpression::TGetMethods::Run(TExpressionRunContext run_context)
{
	if (left == nullptr)
		return;

	//auto exp_result_type = left->GetFormalParameter();
	//if (!exp_result_type.IsRef())
	//{
	//	//TODO не в пустоту, а в массив временных объектов выражения
	//	left->Run(static_fields, formal_params, result, *new TStackValue(false, exp_result_type.GetClass()), local_variables);
	//}
	//else
	left->Run(run_context);
}

void TSExpression::TGetClassField::Run(TExpressionRunContext run_context)
{
	if (field->GetSyntax()->IsStatic())
	{
		*run_context.expression_result = TStackValue(true, field->GetClass());
		run_context.expression_result->SetAsReference((*run_context.static_fields)[field->GetOffset()].get());
	}
	else
	{
		if (left != nullptr)
		{
			auto exp_result_type = left->GetFormalParameter();
			if (!exp_result_type.IsRef())
			{
				//TODO не в пустоту, а в массив временных объектов выражения
				auto temp = new TStackValue(false, exp_result_type.GetClass());
				left->Run(TExpressionRunContext(run_context, temp));
				*run_context.expression_result = TStackValue(true, field->GetClass());
				run_context.expression_result->SetAsReference(((int*)(*temp).get()) + field->GetOffset());
			}
			else
			{
				TStackValue exp_result;
				left->Run(TExpressionRunContext(run_context, &exp_result));
				*run_context.expression_result = TStackValue(true, field->GetClass());
				run_context.expression_result->SetAsReference(((int*)exp_result.get()) + field->GetOffset());
			}
		}
		else
			//иначе необходимо получить ссылку на поле данного класса
		{
			*run_context.expression_result = TStackValue(true, field->GetClass());
			run_context.expression_result->SetAsReference(((int*)run_context.object->get()) + field->GetOffset());
		}
	}
}


void TSExpression::TGetParameter::Run(TExpressionRunContext run_context)
{
	//sp_base - parameter offset
	void* param = (*run_context.formal_params)[parameter->GetOffset()].get();
	*run_context.expression_result = TStackValue(true, parameter->GetClass());
	run_context.expression_result->SetAsReference(param);
}


void TSExpression::TGetLocal::Run(TExpressionRunContext run_context)
{
	void* variable_value = nullptr;
	if (variable->IsStatic())
	{
		variable_value = (*run_context.static_fields)[variable->GetOffset()].get();
	}
	else
	{
		variable_value = (*run_context.local_variables)[variable->GetOffset()].get();
	}
	*run_context.expression_result = TStackValue(true, variable->GetClass());
	run_context.expression_result->SetAsReference(variable_value);
}


void TSExpression_TGetClass::Run(TExpressionRunContext run_context)
{
	//nothing to do
}


void TSExpression_TCreateTempObject::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(false, left->GetFormalParameter().GetType());
	construct_object->Construct(*run_context.expression_result, run_context);
}

void TSExpression_TempObjectType::Run(TExpressionRunContext run_context)
{
	//nothing to do
}


void TSExpression::TGetThis::Run(TExpressionRunContext run_context)
{
	*run_context.expression_result = TStackValue(true, owner);
	run_context.expression_result->SetAsReference(run_context.object->get());
}

void TSExpression::Run(TStatementRunContext run_context)
{
	TStackValue exp_result;
	first_op->Run(TExpressionRunContext(run_context, &exp_result));
}

void TSExpression::Run(TExpressionRunContext run_context)
{
	first_op->Run(run_context);
}