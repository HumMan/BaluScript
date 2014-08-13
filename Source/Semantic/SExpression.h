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
class TSConstructObject;

class TSOperation : public TTokenPos
{
public:
	///<summary>Получить тип возвращаемого подвыражением значения</summary>
	virtual TExpressionResult GetFormalParameter() = 0;
	virtual ~TSOperation(){}
	virtual void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables) = 0;
};

class TSExpression_TMethodCall : public TSOperation
{
	TActualParameters params;
	TSMethod* invoke;
	
	bool is_static;
	bool with_external_object;
public:
	std::unique_ptr<TSOperation> left;
	enum TMethodCallType
	{
		Method,
		ObjectConstructor,
		Operator,
		DefaultAssignOperator
	}type;
	TSExpression_TMethodCall(TMethodCallType type)
	{
		invoke = NULL;
		this->type = type;
	}
	void Build(const std::vector<TSOperation*>& param_expressions, TSMethod* method);
	//void Build(const std::vector<TSOperation*>& param_expressions);
	TExpressionResult GetFormalParameter();
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	void Run(TStackValue& object_to_construct, std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};

class TSExpression_TGetClass : public TSOperation
{
public:
	std::unique_ptr<TSExpression_TGetClass> left;
	TSClass* get_class;
	TExpressionResult GetFormalParameter();
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};

class TSExpression_TCreateTempObject : public TSOperation
{
	
public:
	std::unique_ptr<TSExpression_TGetClass> left;
	std::unique_ptr<TSConstructObject> construct_object;
	void Build(const std::vector<TExpression*>& param_expressions);
	TExpressionResult GetFormalParameter();
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};

class TSExpression :public TSStatement,public TSOperation
{
	std::unique_ptr<TSOperation> first_op;
public:
	
	
	class TInt : public TSOperation
	{
	public:
		TInt(TSClass* owner, TType* syntax_node);
		int val;
		TSType type;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TFloat : public TSOperation
	{
	public:
		TFloat(TSClass* owner, TType* syntax_node);
		float val;
		TSType type;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TBool : public TSOperation
	{
	public:
		TBool(TSClass* owner, TType* syntax_node);
		bool val;
		TSType type;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TEnumValue : public TSOperation
	{
	public:
		TEnumValue(TSClass* owner, TSClass* type);
		int val;
		TSClass* type;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetMethods :public TSOperation
	{
	public:
		std::unique_ptr<TSOperation> left;
		TExpressionResult left_result;
		TExpressionResult result;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetClassField :public TSOperation
	{
	public:
		std::unique_ptr<TSOperation>  left;
		TExpressionResult left_result;
		TSClassField* field;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetParameter :public TSOperation
	{
	public:
		TSParameter* parameter;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetLocal :public TSOperation
	{
	public:
		TSLocalVar* variable;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	class TGetThis :public TSOperation
	{
	public:
		TSClass* owner;
		TExpressionResult GetFormalParameter();
		void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	};
	
public:
	TSExpression(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TExpression* syntax_node)
		:TSStatement(TStatementType::Expression, use_owner, use_method, use_parent, (TStatement*)(syntax_node)){}
	void Build(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	TVariable* GetVar(TNameId name);
	TExpression* GetSyntax();
	TExpressionResult GetFormalParameter()
	{
		return first_op->GetFormalParameter();
	}
	//void Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value, int method_base);
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};