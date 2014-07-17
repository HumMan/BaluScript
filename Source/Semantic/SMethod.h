#pragma once

#include "SParameter.h"
#include "SClassField.h"
#include "Variable.h"

class TSClass;
class TSStatements;
class TMethod;
class TSParameter;
class TStackValue;
class TSLocalVar;
class TStaticValue;

class TSMethod:public TSyntaxNode<TMethod>
{
	friend class TSOverloadedMethod;
private:
	TSType ret;
	TSClass* owner;
	std::vector<std::unique_ptr<TSParameter>> parameters;
	int ret_size;
	int parameters_size;

	std::unique_ptr<TSStatements> statements;

	TSMethod* pre_event;
	TSMethod* post_event;

	bool linked_signature, linked_body;
public:
	///<summary>������������ ��� ������ ������������� ���������� ������������ ��� �����������������/�������� ������</summary>
	void SetPreEvent(TSMethod* use_event);
	void SetPostEvent(TSMethod* use_event);
	
	TSMethod* GetPreEvent();
	TSMethod* GetPostEvent();
	TSMethod(TSClass* use_owner,TMethod* use_syntax);

	int GetParametersSize()
	{
		return parameters_size;
	}
	int GetReturnSize()
	{
		return ret_size;
	}

	TSClass* GetOwner()const;
	TSClass* GetRetClass();
	TSParameter* GetParam(int use_id);
	TVariable* GetVar(TNameId name);
	int GetParamsCount();
	bool HasParams(std::vector<std::unique_ptr<TSParameter>> &use_params)const;
	void CheckForErrors();

	void Build();

	void LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);

	void CalculateParametersOffsets();

	void Run(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& result, TStackValue& object);
};

