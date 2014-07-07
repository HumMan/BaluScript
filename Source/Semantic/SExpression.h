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
	};
	class TMethodCall : public TOperation
	{
		struct TFormalParamWithConversions
		{
			TOperation* expression;
			TFormalParam result;
			TSMethod* copy_constr;
			TSMethod* conversion;
		};
		std::list<TFormalParamWithConversions> input;
		TSMethod* invoke;
	public:
		TMethodCall();
		void Build(const std::vector<TOperation*>& param_expressions, const std::vector<TFormalParam>& params, TSMethod* method);
		TFormalParam GetFormalParameter();
	};
	class TInt : public TOperation
	{
	public:
		int val;
		TFormalParam GetFormalParameter();
	};

	class TGetMethods :public TOperation
	{
	public:
		TOperation* left;
		TFormalParam left_result;
		TFormalParam result;
		TFormalParam GetFormalParameter();
	};
	class TGetClassField :public TOperation
	{
	public:
		TOperation* left;
		TFormalParam left_result;
		TSClassField* field;
		TGetClassField();
		TFormalParam GetFormalParameter();
	};
	class TGetParameter :public TOperation
	{
	public:
		TSParameter* parameter;
		TGetParameter();
		TFormalParam GetFormalParameter();
	};
	class TGetLocal :public TOperation
	{
	public:
		TSLocalVar* variable;
		TGetLocal();
		TFormalParam GetFormalParameter();
	};
	class TGetThis :public TOperation
	{
	public:
		TGetThis();
		TFormalParam GetFormalParameter();
	};
	class TGetClass :public TOperation
	{
	public:
		TGetClass* left;
		TSClass* get_class;
		TGetClass();
		TFormalParam GetFormalParameter();
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
};