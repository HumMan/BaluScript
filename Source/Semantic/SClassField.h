#pragma once

#include "SType.h"
#include "Variable.h"
#include "TemplateRealizations.h"

#include "BuildContext.h"

class TSLocalVar;

class TSClassField :public TSyntaxNode<SyntaxApi::IClassField>, public TVariable, public TNodeWithOffset, public TSMultifield
{
	TSClass* owner;
	TSType type;
	bool linked;
public:
	TSClassField(TSClass* use_owner, SyntaxApi::IClassField* use_syntax);
	TSClass* GetClass()const;
	TSClass* GetOwner()const;
	void LinkSignature(TGlobalBuildContext build_context);
	void LinkBody(TGlobalBuildContext build_context);
};