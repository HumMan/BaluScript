#pragma once

#include "SParameter.h"
#include "SClassField.h"
#include "Variable.h"

#include "RunContext.h"
#include "BuildContext.h"

class TSClass;
class TSStatements;
class TSParameter;

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

class BALUSCRIPT_DLL_INTERFACE TSMethod :public TSyntaxNode<SyntaxApi::IMethod>, public TSpecialClassMethod, public TNodeSignatureLinked, public TNodeBodyLinked
{
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
	///<summary>Используется для вызова автоматически созданного конструктора для пользовательского/внешнего метода</summary>
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

	TSMethod(TSClass* use_owner, SyntaxApi::IMethod* use_syntax);

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
	std::vector<TSParameter*> GetParameters()const;
	TVariable* GetVar(Lexer::TNameId name);
	int GetParamsCount();
	bool HasParams(const std::vector<TSParameter*> &use_params)const;
	void CheckForErrors();

	void Build();

	void LinkSignature(TGlobalBuildContext build_context);
	void LinkBody(TGlobalBuildContext build_context);

	void CalculateParametersOffsets();

	void Run(TMethodRunContext run_context);

	~TSMethod();
};

