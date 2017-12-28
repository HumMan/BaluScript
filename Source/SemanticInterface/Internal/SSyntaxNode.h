#pragma once

#include "../SemanticTreeApi.h"

template<class T>
class TSyntaxNode: public virtual SemanticApi::ISSyntaxNode<T>
{
	T* syntax_node;
public:
	TSyntaxNode()
	{
		syntax_node = nullptr;
	}
	TSyntaxNode(T* use_node)
	{
		syntax_node = use_node;
	}
	T* GetSyntax()const
	{
		return syntax_node;
	}
};

class TNodeWithSize: public virtual SemanticApi::ISNodeWithSize
{
	size_t size;
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
	size_t GetSize()const
	{
		assert(size_initialized);
		return size;
	}
	bool IsSizeInitialized()const
	{
		return size_initialized;
	}
};


class TNodeSignatureLinked: public virtual SemanticApi::ISNodeSignatureLinked
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
	bool IsSignatureLinked()const
	{
		return signature_linked;
	}
};

class TNodeBodyLinked : public virtual SemanticApi::ISNodeBodyLinked
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
	bool IsBodyLinked()const
	{
		return body_linked;
	}
};

class TNodeWithAutoMethods : public virtual SemanticApi::ISNodeWithAutoMethods
{
	bool auto_methods_initialized;
public:
	TNodeWithAutoMethods()
	{
		auto_methods_initialized = false;
	}
	void SetAutoMethodsInitialized()
	{
		assert(!auto_methods_initialized);
		auto_methods_initialized = true;
	}
	bool IsAutoMethodsInitialized()const
	{
		return auto_methods_initialized;
	}
};