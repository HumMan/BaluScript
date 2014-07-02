#pragma once

#include "Type.h"
#include "../lexer.h"
#include "Accessible.h"

class TClassField:public TAccessible,public TTokenPos
{
	friend class TSClassField;
	TClass* owner;
	TNameId name;
	TType type;
	bool is_static;
	bool read_only;
public:
	bool IsReadOnly()const;
	void SetReadOnly(bool use_read_only);
	TClassField(TClass* use_owner);
	void AnalyzeSyntax(TLexer& source);
	void SetOffset(int use_offset);
	int GetOffset()const;
	TClass* GetOwner()const;
	TNameId GetName()const;
	bool IsStatic()const;
};