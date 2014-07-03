#include "SParameter.h"

TSParameter::TSParameter(TSClass* use_owner, TSMethod* use_method, TParameter* use_syntax_node, TType* use_type_syntax_node) : TSyntaxNode(use_syntax_node),
type(use_owner, use_type_syntax_node), owner(use_owner), method(use_method)
{
}

void TSParameter::Link()
{
	type.Link();
}

TSClass* TSParameter::GetClass()
{
	return type.GetClass();
}

bool TSParameter::IsEqualTo(const TSParameter& right)const
{
	return type.IsEqualTo(right.type) && GetSyntax()->IsRef() == right.GetSyntax()->IsRef();
}
