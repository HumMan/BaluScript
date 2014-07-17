#pragma once

#include "../lexer.h"
#include "SClass.h"
#include "SSyntaxNode.h"
#include "SType.h"
#include "Variable.h"

class TSMethod;
class TSLocalVar;

///<summary>Класс описывает параметр сигнатуры метода</summary>
class TSParameter :public TSyntaxNode<TParameter>, public TVariable,public TNodeWithOffset,public TNodeWithSize
{
	TSType type;
	TSClass* owner;
	TSMethod* method;
public:
	TSParameter(TSClass* use_owner, TSMethod* use_method, TParameter* use_syntax_node, TType* use_type_syntax_node);
	void LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	TSClass* GetClass();
	bool IsEqualTo(const TSParameter& right)const;
	void CalculateSize();
};