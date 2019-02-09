#include "ExpressionRun.h"

#include "../NativeTypes/String.h"

#include "TreeRunner.h"

class TSemanticTreeVisitor : public ISExpressionVisitor
{
	TExpressionRunContext run_context;
public:
	void AcceptNode(SemanticApi::ISOperations::ISOperation* node, TExpressionRunContext run_context)
	{
		auto old_context = this->run_context;
		this->run_context = run_context;
		node->Accept(this);
		this->run_context = old_context;
	}

	virtual void Visit(SemanticApi::ISOperations::ISExpression_TMethodCall *_this) override
	{
		//соглашение о вызовах - вызывающий инициализирует параметры, резервирует место для возвращаемого значения, вызывает деструкторы параметров
		std::vector<TStackValue> method_call_formal_params;
		TreeRunner::Construct(_this->GetParams(), method_call_formal_params, run_context);

		auto invoke = _this->GetInvoke();
		switch (_this->GetType())
		{
			//case TSExpression_TMethodCall::DefaultAssignOperator:
			//{
			//	memcpy(method_call_formal_params[0].get(), method_call_formal_params[1].get(), method_call_formal_params[0].GetClass()->GetSize()*sizeof(int));
			//}break;

		case SemanticApi::TMethodCallType::Method:
		{
			TStackValue left_result;
			auto left = _this->GetLeft();
			if (left != nullptr)
				AcceptNode(left, TExpressionRunContext(run_context, &left_result));

			if (invoke->GetRetClass() != nullptr)
				*run_context.expression_result = TStackValue(invoke->IsReturnRef(), invoke->GetRetClass());

			TreeRunner::Run(invoke, TMethodRunContext(run_context, &method_call_formal_params, run_context.expression_result, &left_result));
		}break;

		case SemanticApi::TMethodCallType::Operator:
		{
			if (invoke->GetRetClass() != nullptr)
				*run_context.expression_result = TStackValue(invoke->IsReturnRef(), invoke->GetRetClass());
			TreeRunner::Run(invoke, TMethodRunContext(run_context, &method_call_formal_params, run_context.expression_result, nullptr));
		}break;

		case SemanticApi::TMethodCallType::ObjectConstructor:
		{
			TreeRunner::Run(invoke, TMethodRunContext(run_context, &method_call_formal_params, nullptr, run_context.object));
		}break;

		default:
			assert(false);
		}
		TreeRunner::Destroy(_this->GetParams(),method_call_formal_params, run_context);
	}
	virtual void Visit(SemanticApi::ISOperations::ISExpression_TypeDecl *_this) override
	{
	}
	virtual void Visit(SemanticApi::ISOperations::ISExpression_TCreateTempObject *_this) override
	{
		*run_context.expression_result = TStackValue(false, _this->GetLeft()->GetFormalParam()->GetType());
		TreeRunner::Construct(_this->GetConstructObject(), *run_context.expression_result, run_context);
	}
	virtual void Visit(SemanticApi::ISOperations::IBool *_this) override
	{
		*run_context.expression_result = TStackValue(false, _this->GetType()->GetClass());
		run_context.expression_result->get_as<int>() = _this->GetValue();
	}
	virtual void Visit(SemanticApi::ISOperations::ISExpression *_this) override
	{
		_this->GetFirstOp()->Accept(this);
	}
	virtual void Visit(SemanticApi::ISOperations::IInt *_this) override
	{
		*run_context.expression_result = TStackValue(false, _this->GetType()->GetClass());
		run_context.expression_result->get_as<int>() = _this->GetValue();
	}
	virtual void Visit(SemanticApi::ISOperations::IFloat *_this) override
	{
		*run_context.expression_result = TStackValue(false, _this->GetType()->GetClass());
		run_context.expression_result->get_as<float>() = _this->GetValue();
	}
	virtual void Visit(SemanticApi::ISOperations::IString *_this) override
	{
		*run_context.expression_result = TStackValue(false, _this->GetType()->GetClass());
		run_context.expression_result->get_as< ::TString>().Init(_this->GetValue());
	}
	virtual void Visit(SemanticApi::ISOperations::IEnumValue *_this) override
	{
		*run_context.expression_result = TStackValue(false, _this->GetType());
		run_context.expression_result->get_as<int>() = _this->GetValue();
	}
	virtual void Visit(SemanticApi::ISOperations::IGetMethods *_this) override
	{
		if (_this->GetLeft() == nullptr)
			return;

		//auto exp_result_type = left->GetFormalParameter();
		//if (!exp_result_type.IsRef())
		//{
		//	//TODO не в пустоту, а в массив временных объектов выражения
		//	left->Run(static_fields, formal_params, result, *new TStackValue(false, exp_result_type.GetClass()), local_variables);
		//}
		//else
		AcceptNode(_this->GetLeft(), run_context);
	}
	virtual void Visit(SemanticApi::ISOperations::IGetClassField *_this) override
	{
		auto field = _this->GetField();
		auto left = _this->GetLeft();
		auto field_class = field->GetClass();
		if (field->IsStatic())
		{
			*run_context.expression_result = TStackValue(true, field_class);
			run_context.expression_result->SetAsReference((*run_context.static_fields)[field->GetOffset()].get());
		}
		else
		{
			if (left != nullptr)
			{
				auto exp_result_type = left->GetFormalParam();
				if (!exp_result_type->IsRef())
				{
					//TODO не в пустоту, а в массив временных объектов выражения
					auto temp = new TStackValue(false, exp_result_type->GetClass());
					AcceptNode(left, TExpressionRunContext(run_context, temp));
					*run_context.expression_result = TStackValue(true, field_class);
					run_context.expression_result->SetAsReference(((int*)(*temp).get()) + field->GetOffset());
				}
				else
				{
					TStackValue exp_result;
					AcceptNode(left, TExpressionRunContext(run_context, &exp_result));
					*run_context.expression_result = TStackValue(true, field_class);
					run_context.expression_result->SetAsReference(((int*)exp_result.get()) + field->GetOffset());
				}
			}
			else
				//иначе необходимо получить ссылку на поле данного класса
			{
				*run_context.expression_result = TStackValue(true, field_class);
				run_context.expression_result->SetAsReference(((int*)run_context.object->get()) + field->GetOffset());
			}
		}
	}
	virtual void Visit(SemanticApi::ISOperations::IGetParameter *_this) override
	{
		auto parameter = _this->GetParameter();
		//sp_base - parameter offset
		void* param = (*run_context.formal_params)[parameter->GetOffset()].get();
		*run_context.expression_result = TStackValue(true, parameter->GetClass());
		run_context.expression_result->SetAsReference(param);
	}
	virtual void Visit(SemanticApi::ISOperations::IGetLocal *_this) override
	{
		auto variable = _this->GetVariable();
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
	virtual void Visit(SemanticApi::ISOperations::IGetThis *_this) override
	{
		*run_context.expression_result = TStackValue(true, _this->GetOwner());
		run_context.expression_result->SetAsReference(run_context.object->get());
	}
};

void TExpressionRunner::Run(SemanticApi::ISOperations::ISExpression* _this, TStatementRunContext run_context)
{
	TStackValue exp_result;
	TSemanticTreeVisitor visitor;
	visitor.AcceptNode(_this, TExpressionRunContext(run_context, &exp_result));
}

void TExpressionRunner::Run(SemanticApi::ISOperations::ISOperation * _this, TExpressionRunContext run_context)
{
	TStackValue exp_result;
	TSemanticTreeVisitor visitor;
	visitor.AcceptNode(_this, run_context);
}
