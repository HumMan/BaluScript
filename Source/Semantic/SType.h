#pragma once 

#include <memory>
#include "../lexer.h"

#include "../Syntax/Type.h"

#include "SSyntaxNode.h"

class TSClass;
class TSType;
class TSClassField;
class TSLocalVar;

struct TSType_TTemplateParameter
{
	bool is_value;
	int value;
	std::unique_ptr<TSType> type;
};

struct TSType_TClassName :TSyntaxNode<TType_TClassName>,TNodeSignatureLinked,TNodeBodyLinked
{
	TSClass* class_of_type;
	std::list<TSType_TTemplateParameter> template_params_classes;
	TSType_TClassName(TType_TClassName* use_syntax) :TSyntaxNode(use_syntax)
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

class TSType :public TSyntaxNode<TType>, public TNodeSignatureLinked, public TNodeBodyLinked
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
		if (classes.size() == 0)
			return NULL;
		return classes.back().class_of_type;
	}
};

