#pragma once

#include "../lexer.h"

#include "SSyntaxNode.h"
#include "SType.h"
#include "Variable.h"

#include "../Syntax/Parameter.h"

class TSMethod;
class TSLocalVar;

///<summary>Класс описывает параметр сигнатуры метода</summary>
class TSParameter :public TSyntaxNode<TParameter>, public TVariable,public TNodeWithOffset,public TNodeWithSize
{
	TSType type;
	TSClass* owner;
	TSMethod* method;
	bool is_ref;
public:
	TSParameter(TSClass* use_owner, TSMethod* use_method, TParameter* use_syntax_node, TType* use_type_syntax_node);
	TSParameter(TSClass* use_owner, TSMethod* use_method, TSClass* use_class, bool use_is_ref);
	void LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	TSClass* GetClass();
	bool IsEqualTo(const TSParameter& right)const;
	void CalculateSize();
	bool IsRef()const
	{
		return is_ref;
	}
};