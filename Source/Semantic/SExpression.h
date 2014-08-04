#pragma once

#include <list>

#include "SStatement.h"
#include "SType.h"
#include "SParameter.h"

class TExpression;
class TVariable;
class TSClassField;
class TSParameter;
class TSLocalVar;

class TOperation : public TTokenPos
{
public:
	///<summary>Получить тип возвращаемого подвыражением значения</summary>
	virtual TExpressionResult GetFormalParameter() = 0;
	virtual ~TOperation(){}
	virtual void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables) = 0;
};

class TSExpression_TMethodCall : public TOperation
{
	TActualParameters params;
	TSMethod* invoke;

	
public:
	TOperation* left;
	bool memcpy_assign;
	TSExpression_TMethodCall()
	{
		left = NULL;
		memcpy_assign = false;
	}
	void Build(const std::vector<TOperation*>& param_expressions, TSMethod* method);
	void Build(const std::vector<TOperation*>& param_expressions);
	TExpressionResult GetFormalParameter();
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
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TFloat : public TOperation
	{
	public:
		TFloat(TSClass* owner, TType* syntax_node);
		float val;
		TSType type;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TBool : public TOperation
	{
	public:
		TBool(TSClass* owner, TType* syntax_node);
		bool val;
		TSType type;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TEnumValue : public TOperation
	{
	public:
		TEnumValue(TSClass* owner, TSClass* type);
		int val;
		TSClass* type;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetMethods :public TOperation
	{
	public:
		std::unique_ptr<TOperation> left;
		TExpressionResult left_result;
		TExpressionResult result;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetClassField :public TOperation
	{
	public:
		std::unique_ptr<TOperation>  left;
		TExpressionResult left_result;
		TSClassField* field;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetParameter :public TOperation
	{
	public:
		std::unique_ptr<TSParameter> parameter;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetLocal :public TOperation
	{
	public:
		TSLocalVar* variable;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetThis :public TOperation
	{
	public:
		TSClass* owner;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetClass : public TOperation
	{
	public:
		std::unique_ptr<TGetClass> left;
		TSClass* get_class;
		TExpressionResult GetFormalParameter();
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
	TExpressionResult GetFormalParameter()
	{
		return first_op->GetFormalParameter();
	}
	//void Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value, int method_base);
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};