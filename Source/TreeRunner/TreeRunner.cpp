#include "TreeRunner.h"

void RunConversion(SemanticApi::IActualParamWithConversion* curr_op, std::vector<TStaticValue> &static_fields, TStackValue &value)
{
	if (curr_op->IsRefToRValue())
	{
		auto copy_constr = curr_op->GetCopyConstr();
		assert(copy_constr != nullptr);
		//if (copy_constr != nullptr)
		{
			std::vector<TStackValue> constr_params;
			constr_params.push_back(value);
			TStackValue constr_result;
			TStackValue constructed_object(false, value.GetClass());
			TreeRunner::Run(copy_constr,TMethodRunContext(&static_fields, &constr_params, &constr_result, &constructed_object));
			value = constructed_object;
		}
		//else
		//{
		//	TStackValue constructed_object(false, value.GetClass());
		//	memcpy(constructed_object.get(), value.get(), value.GetClass()->GetSize()*sizeof(int));
		//	value = constructed_object;
		//}
	}
	auto conversion = curr_op->GetConverstion();
	if (conversion != nullptr)
	{
		std::vector<TStackValue> conv_params;
		conv_params.push_back(value);
		TStackValue result;
		TreeRunner::Run(conversion,TMethodRunContext(&static_fields, &conv_params, &result, &value));
		value = result;
	}
}

void TreeRunner::Construct(SemanticApi::ISConstructObject* object, TStackValue& constructed_object, TStatementRunContext run_context)
{
	//auto constructor_call = object->GetConstructorCall();
	//if (constructor_call)
	//{
	//	TStatementRunContext constr_run_context(run_context);
	//	constr_run_context.object = &constructed_object;
	//	Run(constructor_call, TExpressionRunContext(constr_run_context, nullptr));
	//}
}
//
void TreeRunner::Destruct(SemanticApi::ISConstructObject* object, TStackValue& constructed_object, TGlobalRunContext run_context)
{
//	TSMethod* destr = object_type->GetDestructor();
//	if (destr != nullptr)
//	{
//		destr->Run(TMethodRunContext(run_context.static_fields, nullptr, nullptr, &constructed_object));
//	}
}
//
void TreeRunner::Construct(const SemanticApi::IActualParameters* parameters, std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context)
{
//	for (TActualParamWithConversion& par : input)
//	{
//		TStackValue exp_result;
//		par.expression->Run(TExpressionRunContext(run_context, &exp_result));
//
//		par.RunConversion(*run_context.static_fields, exp_result);
//		method_call_formal_params.push_back(exp_result);
//	}
}
//
void TreeRunner::Destroy(const SemanticApi::IActualParameters* parameters, std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context)
{
//	auto it = input.begin();
//	for (size_t i = 0; i < method_call_formal_params.size(); i++)
//	{
//		if (!it->result.IsRef() && it->result.GetClass()->GetDestructor() != nullptr)
//		{
//			TStackValue destructor_result;
//			std::vector<TStackValue> without_params;
//			it->result.GetClass()->GetDestructor()->Run(TMethodRunContext(run_context.static_fields, &without_params, &destructor_result, &method_call_formal_params[i]));
//		}
//		it++;
//	}
}
//
//
void Run(SemanticApi::ISLocalVar* local_var, TStatementRunContext run_context)
{
//	if (!IsStatic())
//		run_context.local_variables->push_back(TStackValue(false, type.GetClass()));
//
//	//TODO т.к. в GetClassMember создаются временные объекты
//	//if (!IsStatic())
//	//	assert(GetOffset() == local_variables.size() - 1);//иначе ошибка Build локальных переменных
//
//	if (IsStatic() && (*run_context.static_fields)[GetOffset()].IsInitialized())
//	{
//		return;
//	}
//	TStackValue var_object(true, GetClass());
//	if (IsStatic())
//		var_object.SetAsReference((*run_context.static_fields)[GetOffset()].get());
//	else
//		var_object.SetAsReference(run_context.local_variables->back().get());
//
//	construct_object->Construct(var_object, run_context);
//
//	if (IsStatic())
//	{
//		(*run_context.static_fields)[GetOffset()].Initialize();
//	}
//
//	if (assign_expr)
//		assign_expr->Run(run_context);
}
//
void Destruct(SemanticApi::ISLocalVar* local_var, std::vector<TStaticValue> &static_fields, std::vector<TStackValue>& local_variables)
{
//	if (!IsStatic())
//	{
//		TStackValue without_result, var_object(true, GetClass());
//		var_object.SetAsReference(local_variables.back().get());
//		construct_object->Destruct(var_object, TGlobalRunContext(&static_fields));
//	}
}
//
//
void PackToStack(std::vector<TStackValue> &formal_params, int* &sp)
{
//	int i = 0;
//	for (TStackValue& v : formal_params)
//	{
//		if (v.IsRef())
//		{
//			*(void**)sp = v.get();
//			(*(int**)&sp)++;
//		}
//		else
//		{
//			memcpy(sp, v.get(), v.GetClass()->GetSize() * sizeof(int));
//			*(int**)&sp += v.GetClass()->GetSize();
//		}
//		if (i == (formal_params.size() - 1))
//			(*(int**)&sp)--;
//		i++;
//	}
}
//
void Run(SemanticApi::ISBytecode, TStatementRunContext run_context)
{
//	int stack[255];
//	int* sp = stack;
//	PackToStack(*run_context.formal_params, sp);
//
//	auto object = (run_context.object != nullptr) ? (int*)run_context.object->get() : nullptr;
//	auto code = GetSyntax()->GetBytecode();
//	for (const SyntaxApi::TBytecodeOp& op : code)
//	{
//
//		if (
//			TVirtualMachine::ExecuteIntOps(op.op, sp, object) ||
//			TVirtualMachine::ExecuteFloatOps(op.op, sp, object) ||
//			TVirtualMachine::ExecuteBoolOps(op.op, sp, object) ||
//			TVirtualMachine::ExecuteBaseOps(op.op, sp, object) ||
//			TVirtualMachine::ExecuteVec2Ops(op.op, sp, object) ||
//			TVirtualMachine::ExecuteVec2iOps(op.op, sp, object))
//		{
//
//		}
//		else
//		{
//			throw;//GetSyntax()->Error("Неизвестная команда!");
//		}
//	}
//	if (GetMethod()->GetRetClass() != nullptr)
//	{
//		*run_context.result = TStackValue(GetMethod()->GetSyntax()->IsReturnRef(), GetMethod()->GetRetClass());
//		if (GetMethod()->GetSyntax()->IsReturnRef())
//		{
//			run_context.result->SetAsReference(*(void**)stack);
//		}
//		else
//		{
//			memcpy(run_context.result->get(), stack, GetMethod()->GetReturnSize() * sizeof(int));
//		}
//	}
}
//
//
//
void TreeRunner::Run(SemanticApi::ISMethod* method, TMethodRunContext method_run_context)
{
	//if (method->IsExternal())
	//{
	//	method->GetExternal()(&method_run_context);
	//}
	//else
	//{
	//	bool result_returned = false;

	//	std::vector<TStackValue> local_variables;

	//	TStatementRunContext run_context;
	//	*(TMethodRunContext*)&run_context = method_run_context;
	//	run_context.result_returned = &result_returned;
	//	run_context.local_variables = &local_variables;

	//	if (method->GetType() == SemanticApi::SpecialClassMethodType::NotSpecial)
	//	{
	//		TreeRunner::RunStatements(statements,run_context);
	//		//TODO заглушка для отслеживания завершения метода без возврата значения
	//		//if (has_return)
	//		//	assert(result_returned);
	//	}
	//	else
	//	{
	//		auto owner = method->GetOwner();
	//		switch (method->GetType())
	//		{
	//		case SemanticApi::SpecialClassMethodType::AutoDefConstr:
	//		{
	//			TreeRunner::RunAutoDefConstr(owner,*run_context.static_fields, *run_context.object);
	//		}break;
	//		case SemanticApi::SpecialClassMethodType::AutoCopyConstr:
	//		{
	//			TreeRunner::RunAutoCopyConstr(owner, *run_context.static_fields, *run_context.formal_params, *run_context.object);
	//		}break;
	//		case SemanticApi::SpecialClassMethodType::AutoDestructor:
	//		{
	//			TreeRunner::RunAutoDestr(owner, *run_context.static_fields, *run_context.object);
	//		}break;
	//		case SemanticApi::SpecialClassMethodType::Default:
	//		{
	//			if (owner->GetAutoDefConstr())
	//				TreeRunner::RunAutoDefConstr(owner, *run_context.static_fields, *run_context.object);
	//			TreeRunner::RunStatements(_this->statements,run_context);
	//		}break;
	//		case SemanticApi::SpecialClassMethodType::CopyConstr:
	//		{
	//			if (owner->GetAutoDefConstr())
	//				owner->RunAutoDefConstr(*run_context.static_fields, *run_context.object);
	//			_this->statements->Run(run_context);
	//		}break;
	//		case SemanticApi::SpecialClassMethodType::Destructor:
	//		{
	//			_this->statements->Run(run_context);
	//			if (owner->GetAutoDestr())
	//				owner->RunAutoDestr(*run_context.static_fields, *run_context.object);
	//		}break;
	//		case SemanticApi::SpecialClassMethodType::AutoAssignOperator:
	//		{
	//			owner->RunAutoAssign(*run_context.static_fields, *run_context.formal_params);
	//		}break;
	//		default:
	//			assert(false);
	//		}
	//	}
	//}
}
//
void TreeRunner::Run(SemanticApi::ISFor, TStatementRunContext run_context)
{
//	TStackValue compare_result;
//	while (true)
//	{
//		compare->Run(TExpressionRunContext(run_context, &compare_result));
//		compare_conversion->RunConversion(*run_context.static_fields, compare_result);
//		if (*(bool*)compare_result.get())
//		{
//
//			statements->Run(run_context);
//			if (*run_context.result_returned)
//				break;
//			increment->Run(run_context);
//			if (*run_context.result_returned)
//				break;
//		}
//		else break;
//	}
}
//
//
void Run(SemanticApi::ISIf, TStatementRunContext run_context)
{
//	TStackValue compare_result;
//
//
//
//	bool_expr->Run(TExpressionRunContext(run_context, &compare_result));
//	bool_expr_conversion->RunConversion(*run_context.static_fields, compare_result);
//	if (*(bool*)compare_result.get())
//	{
//		statements->Run(run_context);
//		if (*run_context.result_returned)
//			return;
//	}
//	else
//	{
//		else_statements->Run(run_context);
//		if (*run_context.result_returned)
//			return;
//	}
}
//
//
void Run(SemanticApi::ISStatements, TStatementRunContext run_context)
{
//	for (const std::unique_ptr<TSStatement>& statement : statements)
//	{
//		statement->Run(run_context);
//		if (*run_context.result_returned)
//			break;
//	}
//	//for (TSStatements::TVarDecl& var_decl : var_declarations)
//	auto size = var_declarations.size();
//	for (size_t i = 0; i < size; i++)
//	{
//		auto& var_decl = var_declarations[size - i - 1];
//		var_decl.pointer->Destruct(*run_context.static_fields, *run_context.local_variables);
//		if (!var_decl.pointer->IsStatic())
//			run_context.local_variables->pop_back();
//	}
}
//
//
void Run(SemanticApi::ISReturn* return_statement, TStatementRunContext run_context)
{
//	TStackValue return_value;
//	result->Run(TExpressionRunContext(run_context, &return_value));
//	*run_context.result_returned = true;
//	conversions->RunConversion(*run_context.static_fields, return_value);
//	*run_context.result = return_value;
}
//
//
void RunAutoDefConstr(SemanticApi::ISClass* _this, std::vector<TStaticValue> &static_fields, TStackValue& object)
{
//	assert(IsAutoMethodsInitialized());
//	assert(_this->auto_def_constr);
//
//	//bool field_has_def_constr = false;
//	//bool parent_has_def_constr = parent.GetClass() == nullptr ? false : parent.GetClass()->HasDefConstr();
//
//	std::vector<TStackValue> formal_params;
//	TStackValue result;
//
//	for (std::unique_ptr<TSClassField>& field : _this->fields)
//	{
//		assert(field->GetClass()->IsAutoMethodsInitialized());
//		TSMethod* field_def_constr = field->GetClass()->GetDefConstr();
//		if (field_def_constr != nullptr && !field->GetSyntax()->IsStatic())
//		{
//			TSClass* field_class = field->GetClass();
//
//			ValidateAccess(field->GetSyntax(), this, field_def_constr);
//			TStackValue field_object(true, field_class);
//			if (field->HasSizeMultiplier())
//			{
//				for (size_t i = 0; i < field->GetSizeMultiplier(); i++)
//				{
//					field_object.SetAsReference(&(((int*)object.get())[field->GetOffset() + i*field->GetClass()->GetSize()]));
//					field_def_constr->Run(TMethodRunContext(&static_fields, &formal_params, &result, &field_object));
//				}
//			}
//			else
//			{
//				field_object.SetAsReference(&(((int*)object.get())[field->GetOffset()]));
//				field_def_constr->Run(TMethodRunContext(&static_fields, &formal_params, &result, &field_object));
//			}
//		}
//	}
}
//
void RunAutoDestr(SemanticApi::ISClass* _this, std::vector<TStaticValue> &static_fields, TStackValue& object)
{
//	assert(IsAutoMethodsInitialized());
//	assert(_this->auto_destr);
//
//	std::vector<TStackValue> formal_params;
//	TStackValue result;
//
//	for (std::unique_ptr<TSClassField>& field : _this->fields)
//	{
//		assert(field->GetClass()->IsAutoMethodsInitialized());
//		TSMethod* field_destr = field->GetClass()->GetDestructor();
//		if (field_destr != nullptr && !field->GetSyntax()->IsStatic())
//		{
//			TSClass* field_class = field->GetClass();
//
//			ValidateAccess(field->GetSyntax(), this, field_destr);
//			TStackValue field_object(true, field_class);
//			if (field->HasSizeMultiplier())
//			{
//				for (size_t i = 0; i < field->GetSizeMultiplier(); i++)
//				{
//					field_object.SetAsReference(&(((int*)object.get())[field->GetOffset() + i*field->GetClass()->GetSize()]));
//					field_destr->Run(TMethodRunContext(&static_fields, &formal_params, &result, &field_object));
//				}
//			}
//			else
//			{
//				field_object.SetAsReference(&(((int*)object.get())[field->GetOffset()]));
//				field_destr->Run(TMethodRunContext(&static_fields, &formal_params, &result, &field_object));
//			}
//		}
//	}
}
//
void RunAutoCopyConstr(SemanticApi::ISClass* _this, std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& object)
{
//	assert(IsAutoMethodsInitialized());
//	assert(_this->auto_copy_constr);
//
//	//конструктор копии должен принимать один аргумент (кроме ссылки на объект) с тем же типом что и данный класс
//	assert(formal_params.size() == 1);
//	assert(formal_params[0].GetClass() == this);
//
//	TStackValue result;
//
//	if (_this->fields.size() > 0)
//	{
//
//		for (std::unique_ptr<TSClassField>& field : _this->fields)
//		{
//			assert(field->GetClass()->IsAutoMethodsInitialized());
//			TSMethod* field_copy_constr = field->GetClass()->GetCopyConstr();
//			if (!field->GetSyntax()->IsStatic())
//			{
//				TSClass* field_class = field->GetClass();
//				//если у поля имеется конструктор копии - вызываем его
//				if (field_copy_constr != nullptr)
//				{
//					ValidateAccess(field->GetSyntax(), this, field_copy_constr);
//					TStackValue field_object(true, field_class);
//					if (field->HasSizeMultiplier())
//					{
//						for (size_t i = 0; i < field->GetSizeMultiplier(); i++)
//						{
//							//настраиваем указатель this - инициализируемый объект
//							field_object.SetAsReference(&(((int*)object.get())[field->GetOffset() + i*field->GetClass()->GetSize()]));
//							std::vector<TStackValue> field_formal_params;
//
//							//передаем в качестве параметра ссылку на копируемый объект
//							field_formal_params.push_back(TStackValue(true, field_class));
//							field_formal_params.back().SetAsReference(&((int*)formal_params[0].get())[field->GetOffset() + i*field->GetClass()->GetSize()]);
//							field_copy_constr->Run(TMethodRunContext(&static_fields, &field_formal_params, &result, &field_object));
//						}
//					}
//					else
//					{
//						//настраиваем указатель this - инициализируемый объект
//						field_object.SetAsReference(&(((int*)object.get())[field->GetOffset()]));
//						std::vector<TStackValue> field_formal_params;
//
//						//передаем в качестве параметра ссылку на копируемый объект
//						field_formal_params.push_back(TStackValue(true, field_class));
//						field_formal_params.back().SetAsReference(&((int*)formal_params[0].get())[field->GetOffset()]);
//						field_copy_constr->Run(TMethodRunContext(&static_fields, &field_formal_params, &result, &field_object));
//					}
//				}
//			}
//		}
//	}
//	else
//	{
//		memcpy(object.get(), formal_params[0].get(), object.GetSize() * sizeof(int));
//	}
}
//
//
void RunAutoAssign(SemanticApi::ISClass* _this, std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params)
{
//	assert(IsAutoMethodsInitialized());
//	assert(_this->auto_assign_operator);
//
//	//оператор присваиваиня должен принимать два аргумента с тем же типом что и данный класс по ссылке
//	assert(formal_params.size() == 2);
//	assert(formal_params[0].GetClass() == this);
//	assert(formal_params[1].GetClass() == this);
//
//	if (_this->fields.size() > 0)
//	{
//		TStackValue result;
//		for (std::unique_ptr<TSClassField>& field : _this->fields)
//		{
//			assert(field->GetClass()->IsAutoMethodsInitialized());
//			TSMethod* field_assign_op = field->GetClass()->GetAssignOperator();
//			if (!field->GetSyntax()->IsStatic())
//			{
//				TSClass* field_class = field->GetClass();
//				//если у поля имеется конструктор копии - вызываем его
//				if (field_assign_op != nullptr)
//				{
//					ValidateAccess(field->GetSyntax(), this, field_assign_op);
//
//					if (field->HasSizeMultiplier())
//					{
//						for (size_t i = 0; i < field->GetSizeMultiplier(); i++)
//						{
//							std::vector<TStackValue> field_formal_params;
//							//передаем в качестве параметра ссылку на копируемый объект
//							field_formal_params.push_back(TStackValue(true, field_class));
//							field_formal_params.back().SetAsReference(&((int*)formal_params[0].get())[field->GetOffset() + i*field->GetClass()->GetSize()]);
//
//							field_formal_params.push_back(TStackValue(true, field_class));
//							field_formal_params.back().SetAsReference(&((int*)formal_params[1].get())[field->GetOffset() + i*field->GetClass()->GetSize()]);
//
//							field_assign_op->Run(TMethodRunContext(&static_fields, &field_formal_params, &result, nullptr));
//						}
//					}
//					else
//					{
//						std::vector<TStackValue> field_formal_params;
//
//						//передаем в качестве параметра ссылку на копируемый объект
//						field_formal_params.push_back(TStackValue(true, field_class));
//						field_formal_params.back().SetAsReference(&((int*)formal_params[0].get())[field->GetOffset()]);
//
//						field_formal_params.push_back(TStackValue(true, field_class));
//						field_formal_params.back().SetAsReference(&((int*)formal_params[1].get())[field->GetOffset()]);
//
//						field_assign_op->Run(TMethodRunContext(&static_fields, &field_formal_params, &result, nullptr));
//					}
//				}
//			}
//		}
//	}
//	else
//	{
//		memcpy(formal_params[0].get(), formal_params[1].get(), formal_params[0].GetClass()->GetSize() * sizeof(int));
//	}
}
//
//
void Run(SemanticApi::ISWhile* while_statement, TStatementRunContext run_context)
{
//	TStackValue compare_result;
//	while (true)
//	{
//		TExpressionRunContext while_run_context(run_context, &compare_result);
//		compare->Run(while_run_context);
//		compare_conversion->RunConversion(*run_context.static_fields, compare_result);
//		if (*(bool*)compare_result.get())
//		{
//			statements->Run(run_context);
//			if (*run_context.result_returned)
//				break;
//		}
//		else break;
//	}
}

void InitializeStaticClassFields(std::vector<SemanticApi::ISClassField*> static_fields, std::vector<TStaticValue> &static_objects)
{
	for (auto v : static_fields)
	{
		static_objects.emplace_back(false, v->GetClass());
		auto def_constr = v->GetClass()->GetDefConstr();
		static_objects[v->GetOffset()].Initialize();
		if (def_constr != nullptr)
		{
			std::vector<TStackValue> constr_formal_params;
			TStackValue without_result, var_object(true, v->GetClass());
			var_object.SetAsReference(static_objects[v->GetOffset()].get());
			TreeRunner::Run(def_constr,TMethodRunContext(&static_objects, &constr_formal_params, &without_result, &var_object));
		}

	}
}
void InitializeStaticVariables(std::vector<SemanticApi::ISLocalVar*> static_variables, std::vector<TStaticValue> &static_objects)
{
	for (auto v : static_variables)
	{
		static_objects.emplace_back(false, v->GetClass());
	}
}