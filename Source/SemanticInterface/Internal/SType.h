#pragma once 

#include "../../SyntaxInterface/SyntaxTreeApi.h"

#include "SSyntaxNode.h"

class TSClass;
class TSType;

class TSType_TTemplateParameter
{
public:
	bool is_value;
	int value;
	std::unique_ptr<TSType> type;
};

class TSType_TClassName :TSyntaxNode<SyntaxApi::IType_TClassName>, TNodeSignatureLinked, TNodeBodyLinked
{
public:
	TSClass* class_of_type;
	std::list<TSType_TTemplateParameter> template_params_classes;
	TSType_TClassName(SyntaxApi::IType_TClassName* use_syntax) :TSyntaxNode(use_syntax)
	{
		class_of_type = nullptr;
	}
	TSType_TClassName(TSClass* use_class) :TSyntaxNode(nullptr)
	{
		class_of_type = use_class;
		SetSignatureLinked();
		SetBodyLinked();
	}
	//link_template_realizations используется при поиске метода по сигнатуре, запрещает генерацию реализаций шаблонов
	TSClass* LinkSignature(SemanticApi::TGlobalBuildContext build_context, TSClass* use_owner, TSClass* use_curr_class, bool link_template_realizations = true);
	void LinkBody(SemanticApi::TGlobalBuildContext build_context);
};

class TSType :public TSyntaxNode<SyntaxApi::IType>, public TNodeSignatureLinked, public TNodeBodyLinked, public SemanticApi::ISType
{
private:
	TSClass* owner;
	std::list<TSType_TClassName> classes;
	void LinkSignature(SemanticApi::TGlobalBuildContext build_context, TSClass* use_curr_class, bool link_template_realizations = true);
public:
	bool IsEqualTo(const TSType& use_right)const;
	TSType(TSClass* use_owner,SyntaxApi::IType* use_syntax_node);
	///<summary>Используется при создании автоматических методов</summary>
	TSType(TSClass* use_owner, TSClass* use_class);
	void LinkSignatureForSpecialMethod();
	void LinkSignatureForMethodFind();
	void LinkSignature(SemanticApi::TGlobalBuildContext build_context);
	void LinkBody(SemanticApi::TGlobalBuildContext build_context);
	SemanticApi::ISClass* GetClass()const;
};

