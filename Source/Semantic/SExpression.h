#pragma once

#include <list>

namespace VariableType
{
	enum Enum
	{
		ClassField,
		Static,
		Parameter,
		Local,
		This
	};
}

class TSExpression :public TSStatement
{
	class TOperation : public TTokenPos
	{
	public:
		virtual TFormalParam Build() = 0;
		virtual ~TOperation(){}
	};
	class TIntValue :public TOperation
	{
		int val;
		TType type;
	public:
		TIntValue(int use_val, TNameId int_class_name, TClass* use_owner) :val(use_val), type(int_class_name, use_owner){}
		TFormalParam Build();
	};
	
	class TGetMemberOp :public TOperation
	{
		TOperation *left;
		TNameId name;
	public:
		TGetMemberOp(TOperation *use_left, TNameId use_member) :left(use_left), name(use_member){}
		~TGetMemberOp()
		{
			delete left;
		}
		TFormalParam Build();
	};
	class TMethodCall :public TOperation
	{
		TOperation *left;
		std::vector<TOperation*> param;
	public:
		TMethodCall(){}
		void AddParam(TOperation* use_param)
		{
			param.push_back(use_param);
		}
		TFormalParam Build();
	};
	
	class TVariable :public TOperation
	{
	public:
		TNameId name;
		VariableType::Enum variable_type;

		TVariable(TNameId use_name){}
		TFormalParam Build();
	};
public:
	TSExpression(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TExpression* syntax_node)
		:TSStatement(TStatementType::Expression, use_owner, use_method, use_parent, (TStatement*)syntax_node){}
	void Build();
};