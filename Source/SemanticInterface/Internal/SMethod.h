#pragma once

#include "SParameter.h"
#include "SClassField.h"
#include "Variable.h"

#include "SClass.h"

class TSClass;
class TSStatements;
class TSParameter;

class TSpecialClassMethod: public virtual SemanticApi::ISpecialClassMethod
{
public:
	
private:
	SemanticApi::SpecialClassMethodType type;
public:
	TSpecialClassMethod(SemanticApi::SpecialClassMethodType use_type)
	{
		type = use_type;
	}
	SemanticApi::SpecialClassMethodType GetType()const
	{
		return type;
	}
};

class TSMethod :public TSyntaxNode<SyntaxApi::IMethod>, public TSpecialClassMethod, public TNodeSignatureLinked, public TNodeBodyLinked,
	public SemanticApi::ISMethod
{
private:
	class TPrivate;
	std::unique_ptr<TPrivate> _this;
public:
	void SetAsExternal(SemanticApi::TExternalSMethod method);
	void CopyExternalMethodBindingsFrom(TSMethod* source);
	SemanticApi::TExternalSMethod GetExternal()const;

	bool IsExternal()const;

	void LinkSignatureForSpecialMethod();
	void LinkSignatureForMethodFind();

	TSMethod(TSClass* use_owner, SemanticApi::SpecialClassMethodType special_method_type);
	void AddParameter(TSParameter* use_par);

	TSMethod(TSClass* use_owner,SyntaxApi::IMethod* use_syntax);

	size_t GetParametersSize()const;
	size_t GetReturnSize()const;

	TSClass* GetOwner()const;
	TSClass* GetRetClass()const;
	TSParameter* GetParam(size_t use_id)const;
	std::vector<TSParameter*> GetParameters()const;
	SemanticApi::IVariable* GetVar(Lexer::TNameId name)const;
	size_t GetParamsCount()const;
	bool HasParams(const std::vector<TSParameter*> &use_params)const;
	void CheckForErrors();

	void Build();

	void LinkSignature(SemanticApi::TGlobalBuildContext build_context);
	void LinkBody(SemanticApi::TGlobalBuildContext build_context);

	void CalculateParametersOffsets();

	bool IsReturnRef()const;

	SemanticApi::ISStatements* GetStatements()const;

	SyntaxApi::TClassMember GetMemberType()const;
	bool IsStatic()const;

	Lexer::TOperator GetOperatorType()const;

	SyntaxApi::IMethod* IGetSyntax()const;

	~TSMethod();
};

