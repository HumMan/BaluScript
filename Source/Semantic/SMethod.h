#pragma once

#include "SParameter.h"
#include "SClassField.h"
#include "Variable.h"

#include "RunContext.h"
#include "BuildContext.h"

class TSClass;
class TSStatements;
class TMethod;
class TSParameter;
class TStackValue;
class TSLocalVar;
class TStaticValue;

class TSpecialClassMethod
{
public:
	enum Type
	{
		NotSpecial,
		AutoDefConstr,
		AutoCopyConstr,
		AutoDestructor,
		AutoAssignOperator,

		Default,
		CopyConstr,
		Destructor
	};
private:
	Type type;
public:
	TSpecialClassMethod(Type use_type)
	{
		type = use_type;
	}
	Type GetType()
	{
		return type;
	}
};

typedef void(*TExternalSMethod)(TMethodRunContext run_context);

class TSMethod :public TSyntaxNode<TMethod>, public TSpecialClassMethod, public TNodeSignatureLinked, public TNodeBodyLinked
{
	friend class TSOverloadedMethod;
private:
	TSType ret;
	bool has_return;
	bool ret_ref;
	TSClass* owner;
	std::vector<std::unique_ptr<TSParameter>> parameters;
	int ret_size;
	int parameters_size;

	std::unique_ptr<TSStatements> statements;

	//TSMethod* pre_event;
	//TSMethod* post_event;

	bool is_external;
	TExternalSMethod external_func;
public:
	///<summary>������������ ��� ������ ������������� ���������� ������������ ��� �����������������/�������� ������</summary>
	//void SetPreEvent(TSMethod* use_event);
	//void SetPostEvent(TSMethod* use_event);
	
	//TSMethod* GetPreEvent();
	//TSMethod* GetPostEvent();

	void SetAsExternal(TExternalSMethod method)
	{
		external_func = method;
	}
	void CopyExternalMethodBindingsFrom(TSMethod* source);
	TSMethod(TSClass* use_owner, TSpecialClassMethod::Type special_method_type);
	void AddParameter(TSParameter* use_par);

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

	void LinkSignature(TGlobalBuildContext build_context);
	void LinkBody(TGlobalBuildContext build_context);

	void CalculateParametersOffsets();

	void Run(TMethodRunContext run_context);

	~TSMethod();
};

