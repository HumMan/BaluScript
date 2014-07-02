#pragma once

#include "SParameter.h"
#include "SClassField.h"
#include "Variable.h"

class TSClass;
class TSStatements;
class TMethod;
class TSParameter;

class TSMethod:public TSyntaxNode<TMethod>
{
	friend class TSOverloadedMethod;
private:
	TSType ret;
	TSClass* owner;
	std::vector<std::shared_ptr<TSParameter>> parameters;

	std::shared_ptr<TSStatements> statements;

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

	TSClass* GetOwner()const;
	TOperator::Enum GetOperatorType();
	TSClass* GetRetClass();
	TSParameter* GetParam(int use_id);
	TVariable* GetVar(TNameId name);
	int GetParamsCount();
	bool HasParams(std::vector<std::shared_ptr<TSParameter>> &use_params)const;
	void CheckForErrors();

	void Build();

	void LinkSignature();
	void LinkBody();
};

