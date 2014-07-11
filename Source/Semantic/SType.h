#pragma once 

#include "../lexer.h"
#include <memory>

#include "../Syntax/Type.h"

#include "SSyntaxNode.h"

class TSClass;
class TSType;

struct TSType_TClassName :TSyntaxNode<TType::TClassName>,TNodeSignatureLinked,TNodeBodyLinked
{
	TSClass* class_of_type;
	std::list<TSType> template_params_classes;
	TSType_TClassName(TType::TClassName* use_syntax) :TSyntaxNode(use_syntax)
	{
		class_of_type = NULL;
	}
	TSClass* LinkSignature(TSClass* use_owner, TSClass* use_curr_class);
	void LinkBody();
};

struct TSType :public TSyntaxNode<TType>, public TNodeSignatureLinked, public TNodeBodyLinked
{
private:
	TSClass* owner;
	std::list<TSType_TClassName> classes;
	void LinkSignature(TSClass* use_curr_class);
public:
	bool IsEqualTo(const TSType& use_right)const;
	TSType(TSClass* use_owner,TType* use_syntax_node);
	void LinkSignature();
	void LinkBody();
	TSClass* GetClass()const
	{
		return classes.back().class_of_type;
	}
};