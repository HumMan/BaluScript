#pragma once

#include "../../SyntaxInterface/SyntaxTreeApi.h"

#include "SSyntaxNode.h"

class TSParameter;
class TSMethod;
class TSClass;

class TSOverloadedMethod :TSyntaxNode<SyntaxApi::IOverloadedMethod>
{
	bool linked_signature,linked_body;
	std::vector<std::unique_ptr<TSMethod>> methods;
	TSClass* owner;
public:
	TSMethod* GetMethod(size_t i)const;
	size_t GetMethodsCount()const;
	TSOverloadedMethod(TSClass* use_owner, SyntaxApi::IOverloadedMethod* use_syntax);
	Lexer::TNameId GetName()const;
	TSMethod* FindParams(const std::vector<TSParameter*>& params)const;
	TSMethod* FindConversion(const std::vector<TSParameter*>& params, TSClass* ret_class)const;
	bool ParamsExists(SemanticApi::ISMethod* use_method)const;
	TSMethod* FindParams(SemanticApi::ISMethod* use_method)const;
	void GetMethods(std::vector<SemanticApi::ISMethod*> &result)const;
	void CheckForErrors(bool is_conversion = false);
	void LinkSignature(SemanticApi::TGlobalBuildContext build_context);
	void LinkBody(SemanticApi::TGlobalBuildContext build_context);
	void Build();
	void CalculateParametersOffsets();

	void CopyExternalMethodBindingsFrom(TSOverloadedMethod* source);
};	