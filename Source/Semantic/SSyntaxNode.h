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

class TNodeWithSize
{
	int size;
	bool size_initialized;
public:
	TNodeWithSize()
	{
		size = 0;
		size_initialized = false;
	}
	void SetSize(int use_size)
	{
		assert(!size_initialized);
		size = use_size;
		size_initialized = true;
	}
	int GetSize()
	{
		assert(size_initialized);
		return size;
	}
	bool IsSizeInitialized()
	{
		return size_initialized;
	}
};


class TNodeSignatureLinked
{
	bool signature_linked;
public:
	TNodeSignatureLinked()
	{
		signature_linked = false;
	}
	void SetSignatureLinked()
	{
		assert(!signature_linked);
		signature_linked = true;
	}
	bool IsSignatureLinked()
	{
		return signature_linked;
	}
};

class TNodeBodyLinked
{
	bool body_linked;
public:
	TNodeBodyLinked()
	{
		body_linked = false;
	}
	void SetBodyLinked()
	{
		assert(!body_linked);
		body_linked = true;
	}
	bool IsBodyLinked()
	{
		return body_linked;
	}
};
