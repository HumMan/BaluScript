﻿#pragma once

#include "../lexer.h"
#include <list>
#include <vector>
#include <memory>

#include "SSyntaxNode.h"

#include "BuildContext.h"

class TSParameter;
class TSMethod;
class TSClass;
class TOverloadedMethod;
class TSClassField;
class TSLocalVar;

class TSOverloadedMethod :TSyntaxNode<TOverloadedMethod>
{
	bool linked_signature,linked_body;
	std::vector<std::unique_ptr<TSMethod>> methods;
	TSClass* owner;
public:
	TSMethod* GetMethod(int i)const;
	int GetMethodsCount()const;
	TSOverloadedMethod(TSClass* use_owner, TOverloadedMethod* use_syntax);
	Lexer::TNameId GetName()const;
	TSMethod* FindParams(const std::vector<TSParameter*>& params)const;
	TSMethod* FindConversion(const std::vector<TSParameter*>& params, TSClass* ret_class)const;
	bool ParamsExists(TSMethod* use_method)const;
	TSMethod* FindParams(TSMethod* use_method)const;
	void GetMethods(std::vector<TSMethod*> &result)const;
	void CheckForErrors(bool is_conversion = false);
	void LinkSignature(TGlobalBuildContext build_context);
	void LinkBody(TGlobalBuildContext build_context);
	void Build();
	void CalculateParametersOffsets();

	void CopyExternalMethodBindingsFrom(TSOverloadedMethod* source);
};	