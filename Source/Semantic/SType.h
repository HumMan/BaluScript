#pragma once 

#include "../lexer.h"
#include <memory>

#include "../Syntax/Type.h"

#include "SSyntaxNode.h"

class TSClass;

struct TSType_TClassName :TSyntaxNode<TType::TClassName>
{
	TSClass* class_of_type;
	TSClass* template_class;
	std::vector<std::list<TSType_TClassName>> template_params_classes;
	TSType_TClassName(TType::TClassName* use_syntax) :TSyntaxNode(use_syntax)
	{
	}
	TSClass* Link(std::list<TSType_TClassName>& classes, TSClass* use_owner, TSClass* use_curr_class);
};

struct TSType:public TSyntaxNode<TType>
{
private:
	TSClass* owner;
	std::list<TSType_TClassName> classes;
	//TSClass* Build(TSClass* use_curr_class,TType::TClassName* use_class_name);
	void Link(TSClass* use_curr_class);
public:
	bool IsEqualTo(const TSType& use_right)const;
	TSType(TSClass* use_owner,TType* use_syntax_node);
	void Link();
	TSClass* GetClass()const
	{
		return classes.back().class_of_type;
	}
};