#pragma once

template<class T>
class TSyntaxNode
{
	T* syntax_node;
public:
	//TSyntaxNode
	//{
	//}
	TSyntaxNode(T* use_node)
	{
		syntax_node = use_node;
	}
	//void SetSyntax(T* use_node)
	//{
	//	syntax_node = use_node;
	//}
	T* GetSyntax()const
	{
		return syntax_node;
	}
};