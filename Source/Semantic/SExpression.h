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

class TSExpression :public TSStatement
{
	class TOperation;
	TOperation* first_op;
public:
	class TOperation : public TTokenPos
	{
	public:
		///<summary>Получить тип возвращаемого подвыражением значения</summary>
		virtual TFormalParam GetFormalParameter() = 0;
		virtual ~TOperation(){}
		virtual void Run(int* &sp) = 0;
	};
	class TMethodCall : public TOperation
	{
		struct TOperationWithConversions
		{
			TOperation* expression;//выражение являющееся параметром
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
		void Build(const std::vector<TOperation*>& param_expressions, const std::vector<TFormalParam>& params, TSMethod* method);
		TFormalParam GetFormalParameter();
		void Run(int* &sp);
	};
	class TInt : public TOperation
	{
	public:
		TInt(TSClass* owner, TType* syntax_node);
		int val;
		TSType type;
		TFormalParam GetFormalParameter();
		void Run(int* &sp);
	};
	class TFloat : public TOperation
	{
	public:
		TFloat(TSClass* owner, TType* syntax_node);
		float val;
		TSType type;
		TFormalParam GetFormalParameter();
		void Run(int* &sp);
	};
	class TGetMethods :public TOperation
	{
	public:
		TOperation* left;
		TFormalParam left_result;
		TFormalParam result;
		TFormalParam GetFormalParameter();
		void Run(int* &sp);
	};
	class TGetClassField :public TOperation
	{
	public:
		TOperation* left;
		TFormalParam left_result;
		TSClassField* field;
		TFormalParam GetFormalParameter();
		void Run(int* &sp);
	};
	class TGetParameter :public TOperation
	{
	public:
		TSParameter* parameter;
		TFormalParam GetFormalParameter();
		void Run(int* &sp);
	};
	class TGetLocal :public TOperation
	{
	public:
		TSLocalVar* variable;
		TFormalParam GetFormalParameter();
		void Run(int* &sp);
	};
	class TGetThis :public TOperation
	{
	public:
		TSClass* owner;
		TFormalParam GetFormalParameter();
		void Run(int* &sp);
	};
	class TGetClass : public TOperation
	{
	public:
		TGetClass* left;
		TSClass* get_class;
		TFormalParam GetFormalParameter();
		void Run(int* &sp);
	};
public:
	TSExpression(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TExpression* syntax_node)
		:TSStatement(TStatementType::Expression, use_owner, use_method, use_parent, (TStatement*)syntax_node){}
	void Build();
	TVariable* GetVar(TNameId name);
	TExpression* GetSyntax()
	{
		return (TExpression*)TSStatement::GetSyntax();
	}
	TFormalParam GetFormalParam()
	{
		return first_op->GetFormalParameter();
	}
	void Run(std::vector<TStackValue> &stack, bool& result_returned, TStackValue* return_value, int method_base);
};