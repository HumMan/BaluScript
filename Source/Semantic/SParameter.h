#pragma once

#include "../SyntaxTree/SyntaxTreeApi.h"

#include "SSyntaxNode.h"
#include "SType.h"
#include "Variable.h"
#include "FormalParam.h"

#include "BuildContext.h"
#include "RunContext.h"

class TSMethod;

class TFormalParameter
{
	TSClass* type;
	bool is_ref;
public:
	TFormalParameter()
	{
		type = nullptr;
		is_ref = false;
	}
	TFormalParameter(TSClass* type, bool is_ref)
	{
		assert(type != nullptr);
		this->type = type;
		this->is_ref = is_ref;
	}
	TSClass* GetClass()
	{
		assert(type != nullptr);
		return type;
	}
	bool IsRef()
	{
		assert(type != nullptr);
		return is_ref;
	}
};

///<summary>Класс описывает параметр сигнатуры метода</summary>
class TSParameter :public TSyntaxNode<SyntaxApi::IParameter>, public TVariable,public TNodeWithOffset,public TNodeWithSize
{
	TSType type;
	TSClass* owner;
	TSMethod* method;

	bool is_ref;
public:
	TSParameter(TSClass* use_owner, TSMethod* use_method, SyntaxApi::IParameter* use_syntax_node, SyntaxApi::IType* use_type_syntax_node);
	TSParameter(TSClass* use_owner, TSMethod* use_method, TSClass* use_class, bool use_is_ref);
	void LinkBody(TGlobalBuildContext build_context);
	void LinkSignature(TGlobalBuildContext build_context);
	TSClass* GetClass();
	bool IsEqualTo(const TSParameter& right)const;
	void CalculateSize();
	bool IsRef()const;
	TFormalParameter AsFormalParameter()const;
};


class TSOperation;
class TExpressionResult;

class TActualParamWithConversion
{
public:
	std::unique_ptr<TSOperation> expression;//выражение являющееся параметром
	TExpressionResult result;
	TSMethod* copy_constr;//используется если необходимо преобразование из lvalue в rvalue
	bool ref_to_rvalue;
	TSMethod* conversion;//используется если необходимо преобразование параметра
	TActualParamWithConversion();
	void BuildConvert(TExpressionResult from_result, TFormalParameter to_formal_param);
	void RunConversion(std::vector<TStaticValue> &static_fields, TStackValue &value);
};

class TActualParameters
{
	std::list<TActualParamWithConversion> input;
public:
	void Build(const std::vector<TSOperation*>& actual_params, const std::vector<TFormalParameter>& formal_params);
	void Construct(std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context);
	void Destroy(std::vector<TStackValue> &method_call_formal_params, TStatementRunContext run_context);
};