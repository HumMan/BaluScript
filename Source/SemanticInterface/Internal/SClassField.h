﻿#pragma once

#include "SType.h"
#include "Variable.h"
#include "TemplateRealizations.h"

class TSLocalVar;

class TSClassField :public TSyntaxNode<SyntaxApi::IClassField>, public TVariable, public TNodeWithOffset, public TSMultifield,
	public SemanticApi::ISClassField
{
	TSClass* owner;
	TSType type;
	bool linked;
public:
	TSClassField(TSClass* use_owner, SyntaxApi::IClassField* use_syntax);
	SemanticApi::ISClass* GetClass()const;
	SemanticApi::ISClass* GetOwner()const;
	void LinkSignature(SemanticApi::TGlobalBuildContext build_context);
	void LinkBody(SemanticApi::TGlobalBuildContext build_context);
	bool IsStatic()const;
};