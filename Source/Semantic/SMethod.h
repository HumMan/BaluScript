#pragma once

#include "SParameter.h"
#include "SClassField.h"

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
	std::vector<std::shared_ptr<TSParameter>> param;

	std::shared_ptr<TSStatements> statements;

	TSMethod* pre_event;
	TSMethod* post_event;
public:
	///<summary>Используется для вызова автоматически созданного конструктора для пользовательского/внешнего метода</summary>
	void SetPreEvent(TSMethod* use_event);
	void SetPostEvent(TSMethod* use_event);
	
	TSMethod* GetPreEvent();
	TSMethod* GetPostEvent();
	TSMethod(TSClass* use_owner);

	TSClass* GetOwner()const;
	TOperator::Enum GetOperatorType();
	TSClass* GetRetClass();
	TSParameter* GetParam(int use_id);
	int GetParamsCount();
	bool HasParams(std::vector<std::shared_ptr<TSParameter>> &use_params)const;
	void CheckForErrors();
};

