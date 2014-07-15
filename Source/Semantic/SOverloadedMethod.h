#pragma once

#include "../lexer.h"
#include <list>
#include <vector>
#include <memory>

#include "SSyntaxNode.h"

class TSParameter;
class TSMethod;
class TSClass;
class TOverloadedMethod;

class TSOverloadedMethod :TSyntaxNode<TOverloadedMethod>
{
	friend class TSClass;
	bool linked_signature,linked_body;
	std::vector<std::unique_ptr<TSMethod>> methods;
	TSClass* owner;
public:
	TSOverloadedMethod(TSClass* use_owner, TOverloadedMethod* use_syntax);
	TNameId GetName()const;
	TSMethod* FindParams(std::vector<std::unique_ptr<TSParameter>>& params);
	TSMethod* FindConversion(std::vector<std::unique_ptr<TSParameter>>& params, TSClass* ret_class);
	bool ParamsExists(TSMethod* use_method);
	TSMethod* FindParams(TSMethod* use_method);
	void GetMethods(std::vector<TSMethod*> &result);
	void CheckForErrors(bool is_conversion = false);
	void LinkSignature();
	void LinkBody();
	void Build();
	void CalculateParametersOffsets();
};	