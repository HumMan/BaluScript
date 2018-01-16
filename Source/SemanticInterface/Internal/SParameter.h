#pragma once

#include "../../SyntaxInterface/SyntaxTreeApi.h"

#include "SSyntaxNode.h"
#include "SType.h"
#include "Variable.h"
#include "ExpressionResult.h"

class TSMethod;

///<summary>����� ��������� �������� ��������� ������</summary>
class TSParameter :public TSyntaxNode<SyntaxApi::IParameter>, public TVariable,public TNodeWithOffset,public TNodeWithSize,
	public SemanticApi::ISParameter
{
	TSType type;
	TSClass* owner;
	TSMethod* method;

	bool is_ref;
public:
	TSParameter(TSClass* use_owner, TSMethod* use_method, SyntaxApi::IParameter* use_syntax_node, SyntaxApi::IType* use_type_syntax_node);
	TSParameter(TSClass* use_owner, TSMethod* use_method, TSClass* use_class, bool use_is_ref);
	void LinkBody(SemanticApi::TGlobalBuildContext build_context);
	void LinkSignature(SemanticApi::TGlobalBuildContext build_context);
	SemanticApi::ISClass* GetClass()const;
	bool IsEqualTo(const TSParameter& right)const;
	void CalculateSize();
	bool IsRef()const;
	SemanticApi::TFormalParameter AsFormalParameter()const;
};


class TSOperation;
class TExpressionResult;

class TActualParamWithConversion: public SemanticApi::IActualParamWithConversion
{
public:
	std::unique_ptr<TSOperation> expression;//��������� ���������� ����������
	TExpressionResult result;
	SemanticApi::ISMethod* copy_constr;//������������ ���� ���������� �������������� �� lvalue � rvalue
	bool ref_to_rvalue;
	SemanticApi::ISMethod* conversion;//������������ ���� ���������� �������������� ���������
	TActualParamWithConversion();
	void BuildConvert(TExpressionResult from_result, SemanticApi::TFormalParameter to_formal_param);
	//void RunConversion(std::vector<TStaticValue> &static_fields, TStackValue &value);
	SemanticApi::ISOperations::ISOperation* GetExpression()const;
	SemanticApi::ISMethod* GetCopyConstr()const;
	bool IsRefToRValue()const;
	SemanticApi::ISMethod* GetConverstion()const;
};

class TActualParameters: public SemanticApi::IActualParameters
{
	std::list<TActualParamWithConversion> input;
public:
	void Build(const std::vector<TSOperation*>& actual_params, const std::vector<SemanticApi::TFormalParameter>& formal_params);
	/*void Construct(std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context);
	void Destroy(std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context);*/
};