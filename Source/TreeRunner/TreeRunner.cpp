#include "TreeRunner.h"

#include "ExpressionRun.h"

#include "../BytecodeBaseOps/baseOps.h"

#include <array>

#include "../syntaxAnalyzer.h"

void TreeRunner::RunConversion(const SemanticApi::IActualParamWithConversion* curr_op, TGlobalRunContext& global_context, TStackValue &value)
{
	if (curr_op->IsRefToRValue())
	{
		auto copy_constr = curr_op->GetCopyConstr();
		TMethodRunContext method_context(&global_context);
		method_context.GetObject()=TStackValue(false, value.GetClass());
		method_context.GetFormalParams().push_back(std::move(value));
		TreeRunner::Run(copy_constr, method_context);
		value = std::move(method_context.GetObject());
	}
	auto conversion = curr_op->GetConverstion();
	if (conversion != nullptr)
	{
		TMethodRunContext method_context(&global_context);
		method_context.GetFormalParams().push_back(std::move(value));
		method_context.GetResult() = TStackValue(conversion->IsReturnRef(), conversion->GetRetClass());
		TreeRunner::Run(conversion, method_context);
		value = std::move(method_context.GetResult());
	}
	assert(value.get() != nullptr);
}

void TreeRunner::Construct(SemanticApi::ISConstructObject* object, TStackValue& constructed_object, TStatementRunContext& run_context)
{
	auto constructor_call = object->GetConstructorCall();
	if (constructor_call)
	{
		TMethodRunContext method_context(run_context.GetMethodContext()->GetGlobalContext());		
		TStatementRunContext constr_run_context(&method_context);
		method_context.GetObject() = std::move(constructed_object);
		method_context.GetFormalParams() = std::move(run_context.GetMethodContext()->GetFormalParams());
		constr_run_context.GetLocalVariables() = std::move(run_context.GetLocalVariables());
		TExpressionRunContext expr_run_context(&constr_run_context);
		TExpressionRunner::Run(constructor_call, expr_run_context);
		assert(expr_run_context.GetExpressionResult().get() == nullptr);
		constructed_object = std::move(method_context.GetObject());
		run_context.GetMethodContext()->GetFormalParams() = std::move(method_context.GetFormalParams());
		run_context.GetLocalVariables() = std::move(constr_run_context.GetLocalVariables());
	}
}

void TreeRunner::Destruct(SemanticApi::ISConstructObject* object, TStackValue& constructed_object, TGlobalRunContext& run_context)
{
	auto object_type = object->GetObjectType();
	SemanticApi::ISMethod* destr = object_type->GetDestructor();
	if (destr != nullptr)
	{
		TMethodRunContext method_context(&run_context);
		method_context.GetObject() = std::move(constructed_object);
		TreeRunner::Run(destr, method_context);
	}
}

void TreeRunner::ConstructParams(const SemanticApi::IActualParameters* parameters, std::vector<TStackValue> &method_call_formal_params, TStatementRunContext& run_context)
{
	auto input = parameters->GetInput();
	for (auto par : input)
	{
		TExpressionRunContext expression_context(&run_context);
		TExpressionRunner::Run(par->GetExpression(), expression_context);
		assert(expression_context.GetExpressionResult().get() != nullptr);
		TreeRunner::RunConversion(par, *run_context.GetMethodContext()->GetGlobalContext(), expression_context.GetExpressionResult());
		method_call_formal_params.push_back(std::move(expression_context.GetExpressionResult()));
	}
}

void TreeRunner::DestroyParams(std::vector<TStackValue> &method_call_formal_params, TStatementRunContext& run_context)
{
	for (size_t i = 0; i < method_call_formal_params.size(); i++)
	{
		auto input_class = method_call_formal_params[i].GetClass();
		//если параметр это значение а не ссылка, то для него нужно вызвать деструктор
		if (!method_call_formal_params[i].IsRef() && input_class->GetDestructor() != nullptr)
		{
			//здесь должна быть проверка что мы не удаляем временный объект на который есть ссылки, напр GetObj()[0]
			TMethodRunContext method_context(run_context.GetGlobalContext());
			method_context.GetObject() = std::move(method_call_formal_params[i]);
			TreeRunner::Run(input_class->GetDestructor(), method_context);
			method_call_formal_params[i] = std::move(method_context.GetObject());
		}
	}
}

void TreeRunner::Run(SemanticApi::ISLocalVar* local_var, TStatementRunContext& run_context)
{
	if (!local_var->IsStatic())
		run_context.GetLocalVariables().push_back(TStackValue(false, local_var->GetType()->GetClass()));

	if (local_var->IsStatic() && run_context.GetGlobalContext()->GetStaticFields()[local_var->GetOffset()].IsInitialized())
	{
		return;
	}
	TStackValue var_object(true, local_var->GetClass());
	if (local_var->IsStatic())
		var_object.SetAsReference(run_context.GetGlobalContext()->GetStaticFields()[local_var->GetOffset()].get());
	else
		var_object.SetAsReference(run_context.GetLocalVariables().back().get());

	TreeRunner::Construct(local_var->GetConstructObject(),var_object, run_context);

	if (local_var->IsStatic())
	{
		run_context.GetGlobalContext()->GetStaticFields()[local_var->GetOffset()].Initialize();
	}

	auto assign_expr = local_var->GetAssignExpression();
	if (assign_expr !=nullptr)
		TExpressionRunner::Run(assign_expr, run_context);
}

void TreeRunner::Destruct(SemanticApi::ISLocalVar* local_var, TGlobalRunContext& global_context, std::vector<TStackValue>& local_variables)
{
	if (!local_var->IsStatic())
	{
		TStackValue without_result, var_object(true, local_var->GetClass());
		var_object.SetAsReference(local_variables.back().get());
		TreeRunner::Destruct(local_var->GetConstructObject(), var_object, global_context);
	}
}


int PackToStack(std::vector<TStackValue> &formal_params, std::array<int, 8> &stack)
{
	size_t i = 0;
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

void TreeRunner::Run(SemanticApi::ISBytecode* bytecode, TStatementRunContext& run_context)
{
	std::array<int, 8> stack;
	int last_param = PackToStack(run_context.GetMethodContext()->GetFormalParams(), stack);
	int* sp = &stack[last_param];
	
	int* object = (int*)run_context.GetMethodContext()->GetObject().get();
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
		//место для результата уже зарезервировано
		auto& result = run_context.GetMethodContext()->GetResult();
		assert(result.GetClass() == bytecode_method->GetRetClass());
		assert(result.GetSize() == bytecode_method->GetReturnSize());
		assert(result.IsRef() == bytecode_method->IsReturnRef());
		if (bytecode_method->IsReturnRef())
		{
			result.SetAsReference(*(void**)stack[0]);
		}
		else
		{
			memcpy(result.get(), &stack[0], bytecode_method->GetReturnSize() * sizeof(int));
		}
		run_context.GetMethodContext()->SetResultReturned();
	}
}



void TreeRunner::Run(SemanticApi::ISMethod* method, TMethodRunContext& method_run_context)
{
	method_run_context.AddRefsFromParams();

	auto& global_context = *method_run_context.GetGlobalContext();
	auto& object = method_run_context.GetObject();
	auto& formal_params = method_run_context.GetFormalParams();

	if (method->IsExternal())
	{
		method->GetExternal()(method_run_context);
	}
	else
	{
		TStatementRunContext statement_run_context(&method_run_context);

		if (method->GetType() == SemanticApi::SpecialClassMethodType::NotSpecial ||
			method->GetType() == SemanticApi::SpecialClassMethodType::Operator ||
			method->GetType() == SemanticApi::SpecialClassMethodType::Conversion)
		{
			TreeRunner::RunStatements(method->GetStatements(), statement_run_context);
			if (method->GetRetClass() != nullptr && !method_run_context.IsResultReturned())
			{
				throw RuntimeException(RuntimeExceptionId::MethodMustReturnValue);
			}
		}
		else
		{
			auto owner = method->GetOwner();
			switch (method->GetType())
			{
			case SemanticApi::SpecialClassMethodType::AutoDefConstr:
			{
				TreeRunner::RunAutoDefConstr(owner, global_context, object);
			}break;
			case SemanticApi::SpecialClassMethodType::AutoCopyConstr:
			{
				TreeRunner::RunAutoCopyConstr(owner, global_context, formal_params, object);
			}break;
			case SemanticApi::SpecialClassMethodType::AutoDestructor:
			{
				TreeRunner::RunAutoDestr(owner, global_context, object);
			}break;
			case SemanticApi::SpecialClassMethodType::DefaultConstr:
			{
				if (owner->GetAutoDefConstr())
					TreeRunner::RunAutoDefConstr(owner, global_context, object);
				TreeRunner::RunStatements(method->GetStatements(), statement_run_context);
			}break;
			case SemanticApi::SpecialClassMethodType::CopyConstr:
			{
				if (owner->GetAutoDefConstr())
					TreeRunner::RunAutoDefConstr(owner, global_context, object);
				TreeRunner::RunStatements(method->GetStatements(), statement_run_context);
			}break;
			case SemanticApi::SpecialClassMethodType::Destructor:
			{
				TreeRunner::RunStatements(method->GetStatements(), statement_run_context);
				if (owner->GetAutoDestr())
					TreeRunner::RunAutoDestr(owner, global_context, object);
			}break;
			case SemanticApi::SpecialClassMethodType::AutoAssignOperator:
			{
				TreeRunner::RunAutoAssign(owner, global_context, formal_params);
			}break;
			default:
				assert(false);
			}
		}

		auto& temp_objects = statement_run_context.GetTempObjects();
		for (size_t i = 0; i < temp_objects.size(); i++)
		{
			auto& obj = temp_objects[i];
			auto destructor = obj.GetClass()->GetDestructor();
			if (destructor != nullptr && !obj.IsRef())
			{
				TMethodRunContext method_context(&global_context);
				method_context.GetObject() = std::move(obj);
				TreeRunner::Run(destructor, method_context);
			}
		}

	}

	method_run_context.RemoveRefsFromParams();
}

void TreeRunner::Run(SemanticApi::ISFor* for_statement, TStatementRunContext& run_context)
{	
	auto compare = for_statement->GetCompare();
	auto compare_conversion = for_statement->GetCompareConversion();
	auto statements = for_statement->GetStatements();
	auto increment = for_statement->GetIncrement();
	while (true)
	{
		TExpressionRunContext expression_context(&run_context);
		TExpressionRunner::Run(dynamic_cast<SemanticApi::ISOperations::ISOperation*>(compare), expression_context);
		TreeRunner::RunConversion(compare_conversion, *run_context.GetGlobalContext(), expression_context.GetExpressionResult());
		if (*(bool*)expression_context.GetExpressionResult().get())
		{

			TreeRunner::RunStatements(statements,run_context);
			if (run_context.GetMethodContext()->IsResultReturned())
				break;
			TreeRunner::RunStatements(increment, run_context);
			if (run_context.GetMethodContext()->IsResultReturned())
				break;
		}
		else break;
	}
}


void TreeRunner::Run(SemanticApi::ISIf* if_op, TStatementRunContext& run_context)
{
	TExpressionRunContext expression_context(&run_context);

	TExpressionRunner::Run(dynamic_cast<SemanticApi::ISOperations::ISOperation*>(if_op->GetBoolExpr()),
		expression_context);

	TreeRunner::RunConversion(if_op->GetBoolExprConversion(), *run_context.GetGlobalContext(), expression_context.GetExpressionResult());
	if (*(bool*)expression_context.GetExpressionResult().get())
	{
		TreeRunner::RunStatements(if_op->GetStatements(), run_context);
		if (run_context.GetMethodContext()->IsResultReturned())
			return;
	}
	else
	{
		TreeRunner::RunStatements(if_op->GetElseStatements(), run_context);
		if (run_context.GetMethodContext()->IsResultReturned())
			return;
	}
}

class TStatementsVisitor: public SemanticApi::ISStatementVisitor
{
public:
	TStatementRunContext* run_context;
	virtual void Visit(SemanticApi::ISIf * node) override
	{
		TreeRunner::Run(node, *run_context);
	}
	virtual void Visit(SemanticApi::ISOperations::ISExpression * node) override
	{
		TExpressionRunner::Run(node, *run_context);
	}
	virtual void Visit(SemanticApi::ISFor * node) override
	{
		TreeRunner::Run(node, *run_context);
	}
	virtual void Visit(SemanticApi::ISLocalVar * node) override
	{
		TreeRunner::Run(node, *run_context);
	}
	virtual void Visit(SemanticApi::ISReturn * node) override
	{
		TreeRunner::Run(node, *run_context);
	}
	virtual void Visit(SemanticApi::ISStatements * node) override
	{
		TreeRunner::RunStatements(node, *run_context);
	}
	virtual void Visit(SemanticApi::ISWhile * node) override
	{
		TreeRunner::Run(node, *run_context);
	}
	virtual void Visit(SemanticApi::ISBytecode * node) override
	{
		TreeRunner::Run(node, *run_context);
	}
};

void TreeRunner::RunStatements(SemanticApi::ISStatements* statements_op, TStatementRunContext& run_context)
{
	auto statements = statements_op->GetStatements();

	for (auto statement : statements)
	{
		TStatementsVisitor visitor;
		visitor.run_context = &run_context;
		statement->Accept(&visitor);

		if (run_context.GetMethodContext()->IsResultReturned())
			break;
	}
	//for (TSStatements::TVarDecl& var_decl : var_declarations)
	auto var_declarations = statements_op->GetVarDeclarations();
	auto size = var_declarations.size();
	for (size_t i = 0; i < size; i++)
	{
		auto& var_decl = var_declarations[size - i - 1];
		TreeRunner::Destruct(var_decl.pointer, *run_context.GetGlobalContext(), run_context.GetLocalVariables());
		if (!var_decl.pointer->IsStatic())
			run_context.GetLocalVariables().pop_back();
	}
}


void TreeRunner::Run(SemanticApi::ISReturn* return_statement, TStatementRunContext& run_context)
{
	TExpressionRunContext expression_context(&run_context);
	TExpressionRunner::Run(dynamic_cast<SemanticApi::ISOperations::ISOperation*>(return_statement->GetResult()),
		expression_context);
	TreeRunner::RunConversion(return_statement->GetConverstion(), *run_context.GetGlobalContext(), expression_context.GetExpressionResult());
	auto& exp_result = expression_context.GetExpressionResult();
	auto& result = run_context.GetMethodContext()->GetResult();
	assert(result.GetClass() == exp_result.GetClass());
	assert(result.GetSize() == exp_result.GetSize());
	assert(result.IsRef() == exp_result.IsRef());
	if (result.IsRef())
	{
		result.SetAsReference(exp_result.get());
	}
	else
	{
		memcpy(result.get(), exp_result.get(), result.GetSize() * sizeof(int));
	}
	run_context.GetMethodContext()->SetResultReturned();
}


void TreeRunner::RunAutoDefConstr(SemanticApi::ISClass* _this, TGlobalRunContext& global_context, TStackValue& object)
{
	assert(_this->IsAutoMethodsInitialized());
	assert(_this->GetAutoDefConstr());

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

			if (field->HasSizeMultiplier())
			{
				for (size_t i = 0; i < field->GetSizeMultiplier(); i++)
				{
					TMethodRunContext method_context(&global_context);
					method_context.GetObject()= TStackValue(true, field_class);
					method_context.GetObject().SetAsReference(&(((int*)object.get())[field->GetOffset() + i*field->GetClass()->GetSize()]));
					TreeRunner::Run(field_def_constr, method_context);
				}
			}
			else
			{
				TMethodRunContext method_context(&global_context);
				method_context.GetObject() = TStackValue(true, field_class);
				method_context.GetObject().SetAsReference(&(((int*)object.get())[field->GetOffset()]));
				TreeRunner::Run(field_def_constr, method_context);
			}
		}
	}
}

void TreeRunner::RunAutoDestr(SemanticApi::ISClass* _this, TGlobalRunContext& global_context, TStackValue& object)
{
	assert(_this->IsAutoMethodsInitialized());
	assert(_this->GetAutoDestr());

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

			
			if (field->HasSizeMultiplier())
			{
				for (size_t i = 0; i < field->GetSizeMultiplier(); i++)
				{
					TMethodRunContext method_context(&global_context);
					method_context.GetObject() = TStackValue(true, field_class);
					method_context.GetObject().SetAsReference(&(((int*)object.get())[field->GetOffset() + i*field->GetClass()->GetSize()]));
					TreeRunner::Run(field_destr, method_context);
				}
			}
			else
			{
				TMethodRunContext method_context(&global_context);
				method_context.GetObject() = TStackValue(true, field_class);
				method_context.GetObject().SetAsReference(&(((int*)object.get())[field->GetOffset()]));
				TreeRunner::Run(field_destr, method_context);
			}
		}
	}
}

void TreeRunner::RunFieldCopyConstr(SemanticApi::ISClassField* field, TGlobalRunContext& global_context, TStackValue& source_object, TStackValue& dest_field)
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
					TMethodRunContext method_context(&global_context);

					TStackValue dest_field_with_mult(true, field_class);
					//перенастраиваем указатель this - инициализируемый объект
					dest_field_with_mult.SetAsReference(&(((int*)dest_field.get())[i * field_class->GetSize()]));
					method_context.GetObject() = std::move(dest_field_with_mult);

					//передаем в качестве параметра ссылку на копируемый объект
					method_context.GetFormalParams().push_back(TStackValue(true, field_class));
					method_context.GetFormalParams().back().SetAsReference(&((int*)source_object.get())[field->GetOffset() + i * field_class->GetSize()]);
					TreeRunner::Run(field_copy_constr, method_context);
				}
			}
			else
			{
				TMethodRunContext method_context(&global_context);
				TStackValue dest_field_with_mult(true, field_class);
				//перенастраиваем указатель this - инициализируемый объект
				dest_field_with_mult.SetAsReference(dest_field.get());
				method_context.GetObject() = std::move(dest_field_with_mult);
				//передаем в качестве параметра ссылку на копируемый объект
				method_context.GetFormalParams().push_back(TStackValue(true, field_class));
				method_context.GetFormalParams().back().SetAsReference(&((int*)source_object.get())[field->GetOffset()]);
				TreeRunner::Run(field_copy_constr, method_context);
			}
		}
	}
}

void TreeRunner::RunAutoCopyConstr(SemanticApi::ISClass* _this, TGlobalRunContext& global_context, std::vector<TStackValue> &formal_params, TStackValue& object)
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


void TreeRunner::RunAutoAssign(SemanticApi::ISClass* _this, TGlobalRunContext& global_context, std::vector<TStackValue> &formal_params)
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

							TMethodRunContext method_context(&global_context);
							method_context.GetFormalParams() = std::move(field_formal_params);

							TreeRunner::Run(field_assign_op, method_context);
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

						TMethodRunContext method_context(&global_context);
						method_context.GetFormalParams() = std::move(field_formal_params);

						TreeRunner::Run(field_assign_op, method_context);
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


void TreeRunner::Run(SemanticApi::ISWhile* while_statement, TStatementRunContext& run_context)
{	
	while (true)
	{
		TExpressionRunContext while_run_context(&run_context);
		TExpressionRunner::Run(
			dynamic_cast<SemanticApi::ISOperations::ISOperation*>(while_statement->GetCompare()), 
			while_run_context);
		TreeRunner::RunConversion(while_statement->GetCompareConversion(), *run_context.GetGlobalContext(), while_run_context.GetExpressionResult());
		if (*(bool*)while_run_context.GetExpressionResult().get())
		{
			TreeRunner::RunStatements(while_statement->GetStatements(), run_context);
			if (run_context.GetMethodContext()->IsResultReturned())
				break;
		}
		else break;
	}
}

void TreeRunner::InitializeStaticClassFields(std::vector<SemanticApi::ISClassField*> static_fields, TGlobalRunContext& run_context)
{
	auto& static_objects = run_context.GetStaticFields();
	int i = static_objects.size();
	for (auto v : static_fields)
	{
		static_objects.emplace_back(false, v->GetClass());
		auto def_constr = v->GetClass()->GetDefConstr();
		v->SetOffset(i);
		static_objects[v->GetOffset()].Initialize();
		if (def_constr != nullptr)
		{
			TStackValue var_object(true, v->GetClass());
			var_object.SetAsReference(static_objects[v->GetOffset()].get());
			TMethodRunContext method_context(&run_context);
			method_context.GetObject() = std::move(var_object);
			TreeRunner::Run(def_constr, method_context);
		}
		i++;
	}
}
void TreeRunner::DeinitializeStatic(TGlobalRunContext& run_context)
{
	auto& static_objects = run_context.GetStaticFields();
	for (auto& v : static_objects)
	{
		auto destructor = v.GetClass()->GetDestructor();
		if (destructor != nullptr)
		{
			TStackValue var_object(true, v.GetClass());
			var_object.SetAsReference(v.get());
			TMethodRunContext method_context(&run_context);
			method_context.GetObject() = std::move(var_object);
			TreeRunner::Run(destructor, method_context);
		}

	}
	static_objects.clear();
}
void TreeRunner::InitializeStaticVariables(std::vector<SemanticApi::ISLocalVar*> static_variables, TGlobalRunContext& run_context)
{
	auto& static_objects = run_context.GetStaticFields();
	int i = static_objects.size();
	for (auto v : static_variables)
	{
		static_objects.emplace_back(false, v->GetClass());
		v->SetOffset(i);
		i++;
	}
}