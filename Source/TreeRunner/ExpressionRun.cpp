#include "ExpressionRun.h"

#include "../NativeTypes/String.h"

#include "TreeRunner.h"

class TSemanticTreeVisitor : public ISExpressionVisitor
{
	TExpressionRunContext* run_context;
public:
	void AcceptNode(SemanticApi::ISOperations::ISOperation* node, TExpressionRunContext& run_context)
	{
		auto old_context = this->run_context;
		this->run_context = &run_context;
		node->Accept(this);
		this->run_context = old_context;	
	}

	virtual void Visit(SemanticApi::ISOperations::ISExpression_TMethodCall *_this) override
	{
		//соглашение о вызовах - вызывающий инициализирует параметры, резервирует место для возвращаемого значения, вызывает деструкторы параметров
		std::vector<TStackValue> method_call_formal_params;
		TreeRunner::ConstructParams(_this->GetParams(), method_call_formal_params, *run_context->GetStatementContext());

		auto invoke = _this->GetInvoke();
		switch (_this->GetType())
		{
		case SemanticApi::TMethodCallType::Method:
		{
			TExpressionRunContext left_context(run_context->GetStatementContext());
			auto left = _this->GetLeft();
			if (left != nullptr)
				AcceptNode(left, left_context);

			

			TStackValue invoke_result;
			if (invoke->GetRetClass() != nullptr)
			{
				invoke_result = TStackValue(invoke->IsReturnRef(), invoke->GetRetClass());
			}

			TMethodRunContext invoke_context(run_context->GetGlobalContext());
			invoke_context.GetFormalParams() = std::move(method_call_formal_params);
			invoke_context.GetResult() = invoke_result;
			invoke_context.GetObject() = left_context.GetExpressionResult();

			TreeRunner::Run(invoke, invoke_context);

			//если вызов метода для временного объекта, то не забываем его потом удалить
			if (invoke_context.GetObject().get()!=nullptr && !invoke_context.GetObject().IsRef())
			{
				run_context->GetStatementContext()->GetTempObjects().push_back(std::move(invoke_context.GetObject()));
			}

			if (invoke->GetRetClass() != nullptr)
			{
				run_context->SetExpressionResult(invoke_context.GetResult());
			}
			method_call_formal_params = std::move(invoke_context.GetFormalParams());
		}break;

		case SemanticApi::TMethodCallType::Operator:
		{
			TStackValue invoke_result;
			if (invoke->GetRetClass() != nullptr)
			{
				invoke_result = TStackValue(invoke->IsReturnRef(), invoke->GetRetClass());
			}

			TMethodRunContext invoke_context(run_context->GetGlobalContext());
			invoke_context.GetFormalParams() = std::move(method_call_formal_params);
			invoke_context.GetResult() = invoke_result;

			TreeRunner::Run(invoke, invoke_context);

			if (invoke->GetRetClass() != nullptr)
			{
				run_context->SetExpressionResult(invoke_context.GetResult());
			}

			method_call_formal_params = std::move(invoke_context.GetFormalParams());
		}break;

		case SemanticApi::TMethodCallType::ObjectConstructor:
		{
			TMethodRunContext invoke_context(run_context->GetGlobalContext());
			invoke_context.GetFormalParams() = std::move(method_call_formal_params);
			invoke_context.GetObject() = run_context->GetMethodContext()->GetObject();
			TreeRunner::Run(invoke, invoke_context);
			run_context->GetMethodContext()->GetObject() = invoke_context.GetObject();
			method_call_formal_params = std::move(invoke_context.GetFormalParams());
		}break;

		default:
			assert(false);
		}
		TreeRunner::DestroyParams(method_call_formal_params, *run_context->GetStatementContext());
	}
	virtual void Visit(SemanticApi::ISOperations::ISExpression_TypeDecl *_this) override
	{
	}
	virtual void Visit(SemanticApi::ISOperations::ISExpression_TCreateTempObject *_this) override
	{
		run_context->SetExpressionResult(TStackValue(false, _this->GetLeft()->GetFormalParam()->GetType()));
		TreeRunner::Construct(_this->GetConstructObject(), run_context->GetExpressionResult(), *run_context->GetStatementContext());
	}
	virtual void Visit(SemanticApi::ISOperations::IBool *_this) override
	{
		run_context->SetExpressionResult(TStackValue(false, _this->GetType()->GetClass()));
		run_context->GetExpressionResult().get_as<int>() = _this->GetValue();
	}
	virtual void Visit(SemanticApi::ISOperations::ISExpression *_this) override
	{
		_this->GetFirstOp()->Accept(this);
	}
	virtual void Visit(SemanticApi::ISOperations::IInt *_this) override
	{
		run_context->SetExpressionResult(TStackValue(false, _this->GetType()->GetClass()));
		run_context->GetExpressionResult().get_as<int>() = _this->GetValue();
	}
	virtual void Visit(SemanticApi::ISOperations::IFloat *_this) override
	{
		run_context->SetExpressionResult(TStackValue(false, _this->GetType()->GetClass()));
		run_context->GetExpressionResult().get_as<float>() = _this->GetValue();
	}
	virtual void Visit(SemanticApi::ISOperations::IString *_this) override
	{
		run_context->SetExpressionResult(TStackValue(false, _this->GetType()->GetClass()));
		run_context->GetExpressionResult().get_as< ::TString>().Init(_this->GetValue());
	}
	virtual void Visit(SemanticApi::ISOperations::IEnumValue *_this) override
	{
		run_context->SetExpressionResult(TStackValue(false, _this->GetType()));
		run_context->GetExpressionResult().get_as<int>() = _this->GetValue();
	}
	virtual void Visit(SemanticApi::ISOperations::IGetMethods *_this) override
	{
		if (_this->GetLeft() == nullptr)
			return;

		AcceptNode(_this->GetLeft(), *run_context);
	}
	virtual void Visit(SemanticApi::ISOperations::IGetClassField *_this) override
	{
		auto field = _this->GetField();
		auto left = _this->GetLeft();
		auto field_class = field->GetClass();
		if (field->IsStatic())
		{
			run_context->SetExpressionResult(TStackValue(true, field_class));
			run_context->GetExpressionResult().SetAsReference(
				run_context->GetGlobalContext()->GetStaticFields()[field->GetOffset()].get());
		}
		else
		{
			if (left != nullptr)
			{
				TExpressionRunContext left_expression_context(run_context->GetStatementContext());
				AcceptNode(left, left_expression_context);

				auto& exp_result = left_expression_context.GetExpressionResult();
					
				run_context->SetExpressionResult(TStackValue(true, field_class));
				run_context->GetExpressionResult().SetAsReference(((int*)exp_result.get()) + field->GetOffset());

				//если поле для временного объекта, то его нужно очистить после завершения statement
				if (!exp_result.IsRef())
				{
					run_context->GetStatementContext()->GetTempObjects().push_back(
						std::move(exp_result));
				}
			}
			else				
			{
				//иначе необходимо получить ссылку на поле данного класса this.a
				run_context->SetExpressionResult(TStackValue(true, field_class));
				run_context->GetExpressionResult().SetAsReference(
					((int*)run_context->GetMethodContext()->GetObject().get()) + field->GetOffset());
			}
		}
	}
	virtual void Visit(SemanticApi::ISOperations::IGetParameter *_this) override
	{
		auto parameter = _this->GetParameter();
		void* param = run_context->GetMethodContext()->GetFormalParams()[parameter->GetOffset()].get();
		run_context->SetExpressionResult(TStackValue(true, parameter->GetClass()));
		run_context->GetExpressionResult().SetAsReference(param);
	}
	virtual void Visit(SemanticApi::ISOperations::IGetLocal *_this) override
	{
		auto variable = _this->GetVariable();
		void* variable_value = nullptr;
		if (variable->IsStatic())
		{
			variable_value = run_context->GetGlobalContext()->GetStaticFields()[variable->GetOffset()].get();
		}
		else
		{
			variable_value = run_context->GetStatementContext()->GetLocalVariables()[variable->GetOffset()].get();
		}
		run_context->SetExpressionResult(TStackValue(true, variable->GetClass()));
		run_context->GetExpressionResult().SetAsReference(variable_value);
	}
	virtual void Visit(SemanticApi::ISOperations::IGetThis *_this) override
	{
		run_context->SetExpressionResult(TStackValue(true, _this->GetOwner()));
		run_context->GetExpressionResult().SetAsReference(run_context->GetMethodContext()->GetObject().get());
	}
};

void TExpressionRunner::Run(SemanticApi::ISOperations::ISExpression* _this, TStatementRunContext& run_context)
{
	TSemanticTreeVisitor visitor;
	TExpressionRunContext expression_run_context(&run_context);
	visitor.AcceptNode(_this, expression_run_context);
}

void TExpressionRunner::Run(SemanticApi::ISOperations::ISOperation * _this, TExpressionRunContext& run_context)
{
	TSemanticTreeVisitor visitor;
	visitor.AcceptNode(_this, run_context);
}
