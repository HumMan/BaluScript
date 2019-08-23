﻿#include "TreeRunner.h"

#include "ExpressionRun.h"

#include "../BytecodeBaseOps/baseOps.h"

#include <array>

void TreeRunner::RunConversion(const SemanticApi::IActualParamWithConversion* curr_op, TGlobalRunContext global_context, TStackValue &value)
{
	if (curr_op->IsRefToRValue())
	{
		auto copy_constr = curr_op->GetCopyConstr();

		std::vector<TStackValue> constr_params;
		constr_params.push_back(value);
		TStackValue constr_result;
		TStackValue constructed_object(false, value.GetClass());
		TreeRunner::Run(copy_constr, TMethodRunContext(global_context, &constr_params, &constr_result, &constructed_object));
		value = constructed_object;
	}
	auto conversion = curr_op->GetConverstion();
	if (conversion != nullptr)
	{
		std::vector<TStackValue> conv_params;
		conv_params.push_back(value);
		TStackValue result;
		TreeRunner::Run(conversion,TMethodRunContext(global_context, &conv_params, &result, &value));
		value = result;
	}
}

void TreeRunner::Construct(SemanticApi::ISConstructObject* object, TStackValue& constructed_object, TStatementRunContext run_context)
{
	auto constructor_call = object->GetConstructorCall();
	if (constructor_call)
	{
		TStatementRunContext constr_run_context(run_context);
		constr_run_context.object = &constructed_object;
		TExpressionRunner::Run(constructor_call, TExpressionRunContext(constr_run_context, nullptr));
	}
}

void TreeRunner::Destruct(SemanticApi::ISConstructObject* object, TStackValue& constructed_object, TGlobalRunContext run_context)
{
	auto object_type = object->GetObjectType();
	SemanticApi::ISMethod* destr = object_type->GetDestructor();
	if (destr != nullptr)
	{
		TreeRunner::Run(destr,TMethodRunContext(run_context, nullptr, nullptr, &constructed_object));
	}
}

void TreeRunner::Construct(const SemanticApi::IActualParameters* parameters, std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context)
{
	auto input = parameters->GetInput();
	for (auto par : input)
	{
		TStackValue exp_result;
		TExpressionRunner::Run(par->GetExpression(), TExpressionRunContext(run_context, &exp_result));

		TreeRunner::RunConversion(par, run_context, exp_result);
		method_call_formal_params.push_back(exp_result);
	}
}

void TreeRunner::Destroy(std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context)
{
	for (size_t i = 0; i < method_call_formal_params.size(); i++)
	{
		auto input_class = method_call_formal_params[i].GetClass();
		//если параметр это значение а не ссылка, то для него нужно вызвать деструктор
		if (!method_call_formal_params[i].IsRef() && input_class->GetDestructor() != nullptr)
		{
			TStackValue destructor_result;
			std::vector<TStackValue> without_params;
			//здесь должна быть проверка что мы не удаляем временный объект на который есть ссылки, напр GetObj()[0]
			//TreeRunner::Run(input_class->GetDestructor(), TMethodRunContext(run_context, &without_params, &destructor_result, &method_call_formal_params[i]));
		}
	}
}

void TreeRunner::Run(SemanticApi::ISLocalVar* local_var, TStatementRunContext run_context)
{
	if (!local_var->IsStatic())
		run_context.local_variables->push_back(TStackValue(false, local_var->GetType()->GetClass()));

	//TODO т.к. в GetClassMember создаются временные объекты
	//if (!IsStatic())
	//	assert(GetOffset() == local_variables.size() - 1);//иначе ошибка Build локальных переменных

	if (local_var->IsStatic() && (*run_context.static_fields)[local_var->GetOffset()].IsInitialized())
	{
		return;
	}
	TStackValue var_object(true, local_var->GetClass());
	if (local_var->IsStatic())
		var_object.SetAsReference((*run_context.static_fields)[local_var->GetOffset()].get());
	else
		var_object.SetAsReference(run_context.local_variables->back().get());

	TreeRunner::Construct(local_var->GetConstructObject(),var_object, run_context);

	if (local_var->IsStatic())
	{
		(*run_context.static_fields)[local_var->GetOffset()].Initialize();
	}

	auto assign_expr = local_var->GetAssignExpression();
	if (assign_expr !=nullptr)
		TExpressionRunner::Run(assign_expr, run_context);
}

void TreeRunner::Destruct(SemanticApi::ISLocalVar* local_var, TGlobalRunContext global_context, std::vector<TStackValue>& local_variables)
{
	if (!local_var->IsStatic())
	{
		TStackValue without_result, var_object(true, local_var->GetClass());
		var_object.SetAsReference(local_variables.back().get());
		//TreeRunner::Destruct(local_var->GetConstructObject(), var_object, TGlobalRunContext(global_context));
	}
}


int PackToStack(std::vector<TStackValue> &formal_params, std::array<int, 8> &stack)
{
	int i = 0;
	for (TStackValue& v : formal_params)
	{
		if (v.IsRef())
		{
			*(void**)&stack[i] = v.get();
			i++;
		}
		else
		{
			memcpy(&stack[i], v.get(), v.GetClass()->GetSize() * sizeof(int));
			i += v.GetClass()->GetSize();
		}
		if (i == (formal_params.size() - 1))
		{
			assert(i < stack.size());
		}
	}
	return i-1;
}

void TreeRunner::Run(SemanticApi::ISBytecode* bytecode, TStatementRunContext run_context)
{
	std::array<int, 8> stack;
	int last_param = PackToStack(*run_context.formal_params, stack);
	int* sp = &stack[last_param];

	auto object = (run_context.object != nullptr) ? (int*)run_context.object->get() : nullptr;
	auto code = bytecode->GetBytecode();
	for (auto op : code)
	{

		if (
			TSimpleOps::ExecuteIntOps(op, sp, object) ||
			TSimpleOps::ExecuteFloatOps(op, sp, object) ||
			TSimpleOps::ExecuteBoolOps(op, sp, object) ||
			TSimpleOps::ExecuteVec2Ops(op, sp, object) ||
			TSimpleOps::ExecuteVec2iOps(op, sp, object))
		{

		}
		else
		{
			throw;//GetSyntax()->Error("Неизвестная команда!");
		}
	}
	auto bytecode_method = bytecode->IGetMethod();
	if (bytecode_method->GetRetClass() != nullptr)
	{
		*run_context.result = TStackValue(bytecode_method->IsReturnRef(), bytecode_method->GetRetClass());
		if (bytecode_method->IsReturnRef())
		{
			run_context.result->SetAsReference(*(void**)stack[0]);
		}
		else
		{
			memcpy(run_context.result->get(), &stack[0], bytecode_method->GetReturnSize() * sizeof(int));
		}
	}
}



void TreeRunner::Run(SemanticApi::ISMethod* method, TMethodRunContext method_run_context)
{
	method_run_context.AddRefsFromParams();

	if (method->IsExternal())
	{
		method->GetExternal()(&method_run_context);
	}
	else
	{
		bool result_returned = false;

		std::vector<TStackValue> local_variables;
		std::vector<TStackValue*> temp_objects;

		TStatementRunContext run_context;
		*(TMethodRunContext*)&run_context = method_run_context;
		run_context.result_returned = &result_returned;
		run_context.local_variables = &local_variables;
		run_context.temp_objects = &temp_objects;

		if (method->GetType() == SemanticApi::SpecialClassMethodType::NotSpecial ||
			method->GetType() == SemanticApi::SpecialClassMethodType::Operator ||
			method->GetType() == SemanticApi::SpecialClassMethodType::Conversion)
		{
			TreeRunner::RunStatements(method->GetStatements(),run_context);
			//TODO заглушка для отслеживания завершения метода без возврата значения
			//if (has_return)
			//	assert(result_returned);
		}
		else
		{
			auto owner = method->GetOwner();
			switch (method->GetType())
			{
			case SemanticApi::SpecialClassMethodType::AutoDefConstr:
			{
				TreeRunner::RunAutoDefConstr(owner,run_context, *run_context.object);
			}break;
			case SemanticApi::SpecialClassMethodType::AutoCopyConstr:
			{
				TreeRunner::RunAutoCopyConstr(owner, run_context, *run_context.formal_params, *run_context.object);
			}break;
			case SemanticApi::SpecialClassMethodType::AutoDestructor:
			{
				TreeRunner::RunAutoDestr(owner, run_context, *run_context.object);
			}break;
			case SemanticApi::SpecialClassMethodType::DefaultConstr:
			{
				if (owner->GetAutoDefConstr())
					TreeRunner::RunAutoDefConstr(owner, run_context, *run_context.object);
				TreeRunner::RunStatements(method->GetStatements(),run_context);
			}break;
			case SemanticApi::SpecialClassMethodType::CopyConstr:
			{
				if (owner->GetAutoDefConstr())
					TreeRunner::RunAutoDefConstr(owner, run_context, *run_context.object);
				TreeRunner::RunStatements(method->GetStatements(), run_context);
			}break;
			case SemanticApi::SpecialClassMethodType::Destructor:
			{
				TreeRunner::RunStatements(method->GetStatements(), run_context);
				if (owner->GetAutoDestr())
					TreeRunner::RunAutoDestr(owner, run_context, *run_context.object);
			}break;
			case SemanticApi::SpecialClassMethodType::AutoAssignOperator:
			{
				TreeRunner::RunAutoAssign(owner, run_context, *run_context.formal_params);
			}break;
			default:
				assert(false);
			}
		}

		//for (int i = 0; i < run_context.temp_objects->size(); i++)
		//{
		//	auto obj = (*run_context.temp_objects)[i];
		//	
		//	if (obj->GetClass()->GetDestructor() != nullptr && !obj->IsRef())
		//	{
		//		std::vector<TStackValue> without_params;
		//		TStackValue without_result;
		//		TreeRunner::Run(obj->GetClass()->GetDestructor(),
		//			TMethodRunContext(run_context, &without_params, &without_result, obj));
		//	}
		//	delete obj;
		//}

	}

	method_run_context.RemoveRefsFromParams();
}

void TreeRunner::Run(SemanticApi::ISFor* for_statement, TStatementRunContext run_context)
{	
	auto compare = for_statement->GetCompare();
	auto compare_conversion = for_statement->GetCompareConversion();
	auto statements = for_statement->GetStatements();
	auto increment = for_statement->GetIncrement();
	while (true)
	{
		TStackValue compare_result;
		TExpressionRunner::Run(dynamic_cast<SemanticApi::ISOperations::ISOperation*>(compare), 
			TExpressionRunContext(run_context, &compare_result));
		TreeRunner::RunConversion(compare_conversion, run_context, compare_result);
		if (*(bool*)compare_result.get())
		{

			TreeRunner::RunStatements(statements,run_context);
			if (*run_context.result_returned)
				break;
			TreeRunner::RunStatements(increment, run_context);
			if (*run_context.result_returned)
				break;
		}
		else break;
	}
}


void TreeRunner::Run(SemanticApi::ISIf* if_op, TStatementRunContext run_context)
{
	TStackValue compare_result;

	TExpressionRunner::Run(dynamic_cast<SemanticApi::ISOperations::ISOperation*>(if_op->GetBoolExpr()),
		TExpressionRunContext(run_context, &compare_result));

	TreeRunner::RunConversion(if_op->GetBoolExprConversion(), run_context, compare_result);
	if (*(bool*)compare_result.get())
	{
		TreeRunner::RunStatements(if_op->GetStatements(), run_context);
		if (*run_context.result_returned)
			return;
	}
	else
	{
		TreeRunner::RunStatements(if_op->GetElseStatements(), run_context);
		if (*run_context.result_returned)
			return;
	}
}

class TStatementsVisitor: public SemanticApi::ISStatementVisitor
{
public:
	TStatementRunContext run_context;
	virtual void Visit(SemanticApi::ISIf * node) override
	{
		TreeRunner::Run(node, run_context);
	}
	virtual void Visit(SemanticApi::ISOperations::ISExpression * node) override
	{
		TExpressionRunner::Run(node, run_context);
	}
	virtual void Visit(SemanticApi::ISFor * node) override
	{
		TreeRunner::Run(node, run_context);
	}
	virtual void Visit(SemanticApi::ISLocalVar * node) override
	{
		TreeRunner::Run(node, run_context);
	}
	virtual void Visit(SemanticApi::ISReturn * node) override
	{
		TreeRunner::Run(node, run_context);
	}
	virtual void Visit(SemanticApi::ISStatements * node) override
	{
		TreeRunner::RunStatements(node, run_context);
	}
	virtual void Visit(SemanticApi::ISWhile * node) override
	{
		TreeRunner::Run(node, run_context);
	}
	virtual void Visit(SemanticApi::ISBytecode * node) override
	{
		TreeRunner::Run(node, run_context);
	}
};

void TreeRunner::RunStatements(SemanticApi::ISStatements* statements_op, TStatementRunContext run_context)
{
	auto statements = statements_op->GetStatements();

	for (auto statement : statements)
	{
		TStatementsVisitor visitor;
		visitor.run_context = run_context;
		statement->Accept(&visitor);
		run_context = visitor.run_context;

		if (*run_context.result_returned)
			break;
	}
	//for (TSStatements::TVarDecl& var_decl : var_declarations)
	auto var_declarations = statements_op->GetVarDeclarations();
	auto size = var_declarations.size();
	for (size_t i = 0; i < size; i++)
	{
		auto& var_decl = var_declarations[size - i - 1];
		TreeRunner::Destruct(var_decl.pointer, run_context, *run_context.local_variables);
		if (!var_decl.pointer->IsStatic())
			run_context.local_variables->pop_back();
	}
}


void TreeRunner::Run(SemanticApi::ISReturn* return_statement, TStatementRunContext run_context)
{
	TStackValue return_value;
	TExpressionRunner::Run(dynamic_cast<SemanticApi::ISOperations::ISOperation*>(return_statement->GetResult()),
		TExpressionRunContext(run_context, &return_value));
	*run_context.result_returned = true;
	TreeRunner::RunConversion(return_statement->GetConverstion(), run_context, return_value);
	*run_context.result = return_value;
}


void TreeRunner::RunAutoDefConstr(SemanticApi::ISClass* _this, TGlobalRunContext global_context, TStackValue& object)
{
	assert(_this->IsAutoMethodsInitialized());
	assert(_this->GetAutoDefConstr());

	//bool field_has_def_constr = false;
	//bool parent_has_def_constr = parent.GetClass() == nullptr ? false : parent.GetClass()->HasDefConstr();

	std::vector<TStackValue> formal_params;
	TStackValue result;

	auto fields = _this->GetFields();

	for (auto field : fields)
	{
		assert(field->GetClass()->IsAutoMethodsInitialized());
		SemanticApi::ISMethod* field_def_constr = field->GetClass()->GetDefConstr();
		if (field_def_constr != nullptr && !field->IsStatic())
		{
			SemanticApi::ISClass* field_class = field->GetClass();

			//TODO проверка доступа должна выполняться в SemanticApi
			//как вариант - сделать всё кроме полей и методов публичным
			//ValidateAccess(field->GetSyntax(), this, field_def_constr);
			TStackValue field_object(true, field_class);
			if (field->HasSizeMultiplier())
			{
				for (size_t i = 0; i < field->GetSizeMultiplier(); i++)
				{
					field_object.SetAsReference(&(((int*)object.get())[field->GetOffset() + i*field->GetClass()->GetSize()]));
					TreeRunner::Run(field_def_constr, TMethodRunContext(global_context, &formal_params, &result, &field_object));
				}
			}
			else
			{
				field_object.SetAsReference(&(((int*)object.get())[field->GetOffset()]));
				TreeRunner::Run(field_def_constr, TMethodRunContext(global_context, &formal_params, &result, &field_object));
			}
		}
	}
}

void TreeRunner::RunAutoDestr(SemanticApi::ISClass* _this, TGlobalRunContext global_context, TStackValue& object)
{
	assert(_this->IsAutoMethodsInitialized());
	assert(_this->GetAutoDestr());

	std::vector<TStackValue> formal_params;
	TStackValue result;

	auto fields = _this->GetFields();

	for (auto field : fields)
	{
		assert(field->GetClass()->IsAutoMethodsInitialized());
		SemanticApi::ISMethod* field_destr = field->GetClass()->GetDestructor();
		if (field_destr != nullptr && !field->IsStatic())
		{
			SemanticApi::ISClass* field_class = field->GetClass();

			//TODO
			//ValidateAccess(field->GetSyntax(), this, field_destr);

			TStackValue field_object(true, field_class);
			if (field->HasSizeMultiplier())
			{
				for (size_t i = 0; i < field->GetSizeMultiplier(); i++)
				{
					field_object.SetAsReference(&(((int*)object.get())[field->GetOffset() + i*field->GetClass()->GetSize()]));
					TreeRunner::Run(field_destr, TMethodRunContext(global_context, &formal_params, &result, &field_object));
				}
			}
			else
			{
				field_object.SetAsReference(&(((int*)object.get())[field->GetOffset()]));
				TreeRunner::Run(field_destr,TMethodRunContext(global_context, &formal_params, &result, &field_object));
			}
		}
	}
}

void TreeRunner::RunFieldCopyConstr(SemanticApi::ISClassField* field, TGlobalRunContext global_context, TStackValue& source_object, TStackValue& dest_field)
{
	assert(field->GetClass()->IsAutoMethodsInitialized());
	
	if (!field->IsStatic())
	{
		SemanticApi::ISClass* field_class = field->GetClass();
		SemanticApi::ISMethod* field_copy_constr = field_class->GetCopyConstr();
		//если у поля имеется конструктор копии - вызываем его
		if (field_copy_constr != nullptr)
		{
			
			if (field->HasSizeMultiplier())
			{
				for (size_t i = 0; i < field->GetSizeMultiplier(); i++)
				{
					TStackValue result;

					TStackValue dest_field_with_mult(true, field_class);
					//перенастраиваем указатель this - инициализируемый объект
					dest_field_with_mult.SetAsReference(&(((int*)dest_field.get())[i * field_class->GetSize()]));
					std::vector<TStackValue> field_formal_params;

					//передаем в качестве параметра ссылку на копируемый объект
					field_formal_params.push_back(TStackValue(true, field_class));
					field_formal_params.back().SetAsReference(&((int*)source_object.get())[field->GetOffset() + i * field_class->GetSize()]);
					TreeRunner::Run(field_copy_constr, TMethodRunContext(global_context, &field_formal_params, &result, &dest_field_with_mult));
				}
			}
			else
			{
				TStackValue result;
				std::vector<TStackValue> field_formal_params;
				//передаем в качестве параметра ссылку на копируемый объект
				field_formal_params.push_back(TStackValue(true, field_class));
				field_formal_params.back().SetAsReference(&((int*)source_object.get())[field->GetOffset()]);
				TreeRunner::Run(field_copy_constr, TMethodRunContext(global_context, &field_formal_params, &result, &dest_field));
			}
		}
	}
}

void TreeRunner::RunAutoCopyConstr(SemanticApi::ISClass* _this, TGlobalRunContext global_context, std::vector<TStackValue> &formal_params, TStackValue& object)
{
	assert(_this->IsAutoMethodsInitialized());
	assert(_this->GetAutoCopyConstr());

	//конструктор копии должен принимать один аргумент (кроме ссылки на объект) с тем же типом что и данный класс
	assert(formal_params.size() == 1);
	assert(formal_params[0].GetClass() == _this);

	auto fields = _this->GetFields();

	if (fields.size() > 0)
	{
		for (auto field : fields)
		{
			TStackValue field_object(true, field->GetClass());
			//настраиваем указатель this - инициализируемый объект
			field_object.SetAsReference(&(((int*)object.get())[field->GetOffset()]));
			RunFieldCopyConstr(field, global_context, formal_params[0], field_object);
		}
	}
	else
	{
		memcpy(object.get(), formal_params[0].get(), object.GetSize() * sizeof(int));
	}
}


void TreeRunner::RunAutoAssign(SemanticApi::ISClass* _this, TGlobalRunContext global_context, std::vector<TStackValue> &formal_params)
{
	assert(_this->IsAutoMethodsInitialized());
	assert(_this->GetAutoAssignOperator());

	//оператор присваиваиня должен принимать два аргумента с тем же типом что и данный класс по ссылке
	assert(formal_params.size() == 2);
	assert(formal_params[0].GetClass() == _this);
	assert(formal_params[1].GetClass() == _this);

	auto fields = _this->GetFields();

	if (fields.size() > 0)
	{		
		TStackValue result;
		for (auto field : fields)
		{
			assert(field->GetClass()->IsAutoMethodsInitialized());
			SemanticApi::ISMethod* field_assign_op = field->GetClass()->GetAssignOperator();
			if (!field->IsStatic())
			{
				SemanticApi::ISClass* field_class = field->GetClass();
				//если у поля имеется конструктор копии - вызываем его
				if (field_assign_op != nullptr)
				{
					//TODO
					//ValidateAccess(field->GetSyntax(), this, field_assign_op);

					if (field->HasSizeMultiplier())
					{
						for (size_t i = 0; i < field->GetSizeMultiplier(); i++)
						{
							std::vector<TStackValue> field_formal_params;
							//передаем в качестве параметра ссылку на копируемый объект
							field_formal_params.push_back(TStackValue(true, field_class));
							field_formal_params.back().SetAsReference(&((int*)formal_params[0].get())[field->GetOffset() + i*field->GetClass()->GetSize()]);

							field_formal_params.push_back(TStackValue(true, field_class));
							field_formal_params.back().SetAsReference(&((int*)formal_params[1].get())[field->GetOffset() + i*field->GetClass()->GetSize()]);

							TreeRunner::Run(field_assign_op, TMethodRunContext(global_context, &field_formal_params, &result, nullptr));
						}
					}
					else
					{
						std::vector<TStackValue> field_formal_params;

						//передаем в качестве параметра ссылку на копируемый объект
						field_formal_params.push_back(TStackValue(true, field_class));
						field_formal_params.back().SetAsReference(&((int*)formal_params[0].get())[field->GetOffset()]);

						field_formal_params.push_back(TStackValue(true, field_class));
						field_formal_params.back().SetAsReference(&((int*)formal_params[1].get())[field->GetOffset()]);

						TreeRunner::Run(field_assign_op, TMethodRunContext(global_context, &field_formal_params, &result, nullptr));
					}
				}
			}
		}
	}
	else
	{
		memcpy(formal_params[0].get(), formal_params[1].get(), formal_params[0].GetClass()->GetSize() * sizeof(int));
	}
}


void TreeRunner::Run(SemanticApi::ISWhile* while_statement, TStatementRunContext run_context)
{	
	while (true)
	{
		TStackValue compare_result;
		TExpressionRunContext while_run_context(run_context, &compare_result);
		TExpressionRunner::Run(
			dynamic_cast<SemanticApi::ISOperations::ISOperation*>(while_statement->GetCompare()), 
			while_run_context);
		TreeRunner::RunConversion(while_statement->GetCompareConversion(), run_context, compare_result);
		if (*(bool*)compare_result.get())
		{
			TreeRunner::RunStatements(while_statement->GetStatements(), run_context);
			if (*run_context.result_returned)
				break;
		}
		else break;
	}
}

void TreeRunner::InitializeStaticClassFields(std::vector<SemanticApi::ISClassField*> static_fields, TGlobalRunContext run_context)
{
	auto& static_objects = *run_context.static_fields;
	int i = static_objects.size();
	for (auto v : static_fields)
	{
		static_objects.emplace_back(false, v->GetClass());
		auto def_constr = v->GetClass()->GetDefConstr();
		v->SetOffset(i);
		static_objects[v->GetOffset()].Initialize();
		if (def_constr != nullptr)
		{
			std::vector<TStackValue> constr_formal_params;
			TStackValue without_result, var_object(true, v->GetClass());
			var_object.SetAsReference(static_objects[v->GetOffset()].get());
			TreeRunner::Run(def_constr,TMethodRunContext(run_context, &constr_formal_params, &without_result, &var_object));
		}
		i++;
	}
}
void TreeRunner::DeinitializeStatic(TGlobalRunContext run_context)
{
	auto& static_objects = *run_context.static_fields;
	for (auto v : static_objects)
	{
		auto destructor = v.GetClass()->GetDestructor();
		if (destructor != nullptr)
		{
			std::vector<TStackValue> destr_formal_params;
			TStackValue without_result, var_object(true, v.GetClass());
			var_object.SetAsReference(v.get());
			TreeRunner::Run(destructor, TMethodRunContext(run_context, &destr_formal_params, &without_result, &var_object));
		}

	}
}
void TreeRunner::InitializeStaticVariables(std::vector<SemanticApi::ISLocalVar*> static_variables, TGlobalRunContext run_context)
{
	auto& static_objects = *run_context.static_fields;
	int i = static_objects.size();
	for (auto v : static_variables)
	{
		static_objects.emplace_back(false, v->GetClass());
		v->SetOffset(i);
		i++;
	}
}