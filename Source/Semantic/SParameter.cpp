#include "SParameter.h"

#include "../Syntax/Statements.h"

TSParameter::TSParameter(TSClass* use_owner, TSMethod* use_method, TParameter* use_syntax_node, TType* use_type_syntax_node) 
	: TSyntaxNode(use_syntax_node)
	, TVariable(TVariableType::Parameter)
	, type(use_owner, use_type_syntax_node), owner(use_owner), method(use_method), is_ref(use_syntax_node->IsRef())
{
}

TSParameter::TSParameter(TSClass* use_owner, TSMethod* use_method, TSClass* use_class, bool use_is_ref)
	: TSyntaxNode(NULL)
	, TVariable(TVariableType::Parameter)
	, type(use_owner, use_class), owner(use_owner), method(use_method), is_ref(use_is_ref)
{

}

void TSParameter::LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	type.LinkBody(static_fields, static_variables);
}
void TSParameter::LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	type.LinkSignature(static_fields, static_variables);
}
TSClass* TSParameter::GetClass()
{
	return type.GetClass();
}

bool TSParameter::IsEqualTo(const TSParameter& right)const
{
	return type.IsEqualTo(right.type) && GetSyntax()->IsRef() == right.GetSyntax()->IsRef();
}

void TSParameter::CalculateSize()
{
	if (GetSyntax()->IsRef())
		SetSize(1);
	else
		SetSize(GetClass()->GetSize());
}
