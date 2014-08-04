#pragma once

#include "../lexer.h"

#include "SSyntaxNode.h"
#include "SType.h"
#include "Variable.h"
#include "FormalParam.h"

#include "../Syntax/Parameter.h"

class TSMethod;
class TSLocalVar;

class TFormalParameter
{
	TSClass* type;
	bool is_ref;
public:
	TFormalParameter()
	{
		type = NULL;
		is_ref = false;
	}
	TFormalParameter(TSClass* type, bool is_ref)
	{
		assert(type != NULL);
		this->type = type;
		this->is_ref = is_ref;
	}
	TSClass* GetClass()
	{
		assert(type != NULL);
		return type;
	}
	bool IsRef()
	{
		assert(type != NULL);
		return is_ref;
	}
};

///<summary>Класс описывает параметр сигнатуры метода</summary>
class TSParameter :public TSyntaxNode<TParameter>, public TVariable,public TNodeWithOffset,public TNodeWithSize
{
	TSType type;
	TSClass* owner;
	TSMethod* method;

	bool is_ref;
public:
	TSParameter(TSClass* use_owner, TSMethod* use_method, TParameter* use_syntax_node, TType* use_type_syntax_node);
	TSParameter(TSClass* use_owner, TSMethod* use_method, TSClass* use_class, bool use_is_ref);
	void LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	TSClass* GetClass();
	bool IsEqualTo(const TSParameter& right)const;
	void CalculateSize();
	bool IsRef()const;
	TFormalParameter AsFormalParameter()const;
};


class TOperation;
class TExpressionResult;

class TActualParamWithConversion
{
public:
	std::unique_ptr<TOperation> expression;//выражение являющееся параметром
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
	void Build(const std::vector<TOperation*>& actual_params, const std::vector<TFormalParameter>& formal_params);
	std::vector<TStackValue> method_call_formal_params;
	void Construct(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	void Destroy(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
};