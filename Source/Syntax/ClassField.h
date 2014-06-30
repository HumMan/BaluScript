#pragma once

#include "Variable.h"
#include "Type.h"
#include "../lexer.h"
#include "Accessible.h"

class TClassField:public TAccessible,public TVariable,public TTokenPos
{
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