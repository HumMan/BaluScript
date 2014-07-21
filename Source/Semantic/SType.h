#pragma once 

#include "../lexer.h"
#include <memory>

#include "../Syntax/Type.h"

#include "SSyntaxNode.h"

class TSClass;
class TSType;
class TSClassField;
class TSLocalVar;

struct TSType_TClassName :TSyntaxNode<TType::TClassName>,TNodeSignatureLinked,TNodeBodyLinked
{
	TSClass* class_of_type;
	std::list<TSType> template_params_classes;
	TSType_TClassName(TType::TClassName* use_syntax) :TSyntaxNode(use_syntax)
	{
		class_of_type = NULL;
	}
	TSType_TClassName(TSClass* use_class) :TSyntaxNode(NULL)
	{
		class_of_type = use_class;
		SetSignatureLinked();
		SetBodyLinked();
	}
	TSClass* LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables, TSClass* use_owner, TSClass* use_curr_class);
	void LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
};

struct TSType :public TSyntaxNode<TType>, public TNodeSignatureLinked, public TNodeBodyLinked
{
private:
	TSClass* owner;
	std::list<TSType_TClassName> classes;
	void LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables, TSClass* use_curr_class);
public:
	bool IsEqualTo(const TSType& use_right)const;
	TSType(TSClass* use_owner,TType* use_syntax_node);
	///<summary>Используется при создании автоматических методов</summary>
	TSType(TSClass* use_owner, TSClass* use_class);
	void LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	TSClass* GetClass()const
	{
		return classes.back().class_of_type;
	}
};