#pragma once

#include "SType.h"
#include "Variable.h"
#include "TemplateRealizations.h"

class TClassField;
class TSLocalVar;

class TSClassField :public TSyntaxNode<TClassField>, public TVariable, public TNodeWithOffset, public TSMultifield
{
	TSClass* owner;
	TSType type;
	bool linked;
public:
	TSClassField(TSClass* use_owner, TClassField* use_syntax);
	TSClass* GetClass()const;
	TSClass* GetOwner()const;
	void LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
};