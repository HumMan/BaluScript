#pragma once

#include "../lexer.h"
#include "SClass.h"
#include "SSyntaxNode.h"
#include "SType.h"

class TSMethod;

///<summary> ласс описывает параметр объ€влени€ метода</summary>
class TSParameter :public TSyntaxNode<TParameter>
{
	TSType type;
	TSClass* owner;
	TSMethod* method;
public:
	TSParameter(TSClass* use_owner, TSMethod* use_method, TParameter* use_syntax_node, TType* use_type_syntax_node);
	void Link();
	TSClass* GetClass();
	bool IsEqualTo(const TSParameter& right)const;
};