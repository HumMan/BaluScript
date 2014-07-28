#pragma once

#include <list>

#include "SStatement.h"
#include "SType.h"
#include "FormalParam.h"

class TExpression;
class TVariable;
class TSClassField;
class TSParameter;
class TSLocalVar;

class TOperation : public TTokenPos
{
public:
	///<summary>�������� ��� ������������� ������������� ��������</summary>
	virtual TFormalParam GetFormalParameter() = 0;
	virtual ~TOperation(){}
	virtual void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables) = 0;
};

class TSExpression_TMethodCall : public TOperation
{
	struct TOperationWithConversions
	{
		TOperation* expression;//��������� ���������� ����������
		TFormalParamWithConversions conversions;
		TOperationWithConversions(TOperation* expression)
		{
			this->expression = expression;
		}
		void BuildConvert(TFormalParam from_result, TSClass* param_class, bool param_ref)
		{
			conversions.BuildConvert(from_result, param_class, param_ref);
		}
	};

	std::list<TOperationWithConversions> input;
	TSMethod* invoke;
public:
	TOperation* left;
	TSExpression_TMethodCall()
	{
		left = NULL;
	}
	void Build(const std::vector<TOperation*>& param_expressions, const std::vector<TFormalParam>& params, TSMethod* method);
	TFormalParam GetFormalParameter();
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};

class TSExpression :public TSStatement,public TOperation
{
	std::unique_ptr<TOperation> first_op;
public:
	
	
	class TInt : public TOperation
	{
	public:
		TInt(TSClass* owner, TType* syntax_node);
		int val;
		TSType type;
		TFormalParam GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TFloat : public TOperation
	{
	public:
		TFloat(TSClass* owner, TType* syntax_node);
		float val;
		TSType type;
		TFormalParam GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TBool : public TOperation
	{
	public:
		TBool(TSClass* owner, TType* syntax_node);
		bool val;
		TSType type;
		TFormalParam GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TEnumValue : public TOperation
	{
	public:
		TEnumValue(TSClass* owner, TSClass* type);
		int val;
		TSClass* type;
		TFormalParam GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetMethods :public TOperation
	{
	public:
		std::unique_ptr<TOperation> left;
		TFormalParam left_result;
		TFormalParam result;
		TFormalParam GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetClassField :public TOperation
	{
	public:
		std::unique_ptr<TOperation>  left;
		TFormalParam left_result;
		TSClassField* field;
		TFormalParam GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetParameter :public TOperation
	{
	public:
		std::unique_ptr<TSParameter> parameter;
		TFormalParam GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetLocal :public TOperation
	{
	public:
		TSLocalVar* variable;
		TFormalParam GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetThis :public TOperation
	{
	public:
		TSClass* owner;
		TFormalParam GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetClass : public TOperation
	{
	public:
		std::unique_ptr<TGetClass> left;
		TSClass* get_class;
		TFormalParam GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
public:
	TSExpression(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TExpression* syntax_node)
		:TSStatement(TStatementType::Expression, use_owner, use_method, use_parent, (TStatement*)syntax_node){}
	void Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	TVariable* GetVar(TNameId name);
	TExpression* GetSyntax()
	{
		return (TExpression*)TSStatement::GetSyntax();
	}
	TFormalParam GetFormalParameter()
	{
		return first_op->GetFormalParameter();
	}
	//void Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value, int method_base);
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};