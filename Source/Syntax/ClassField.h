#pragma once

#include <memory>

#include "../lexer.h"
#include "Accessible.h"

class TClass;
class TType;

class TClassField :public TAccessible, public Lexer::TTokenPos, public TMultifield
{
	friend class TSClassField;
	TClass* owner;
	Lexer::TNameId name;
	std::shared_ptr<TType> type;
	bool is_static;
	bool read_only;
public:
	bool IsReadOnly()const;
	void SetReadOnly(bool use_read_only);
	TClassField(TClass* use_owner);
	void AnalyzeSyntax(Lexer::ILexer* source);
	void SetOffset(int use_offset);
	int GetOffset()const;
	TClass* GetOwner()const;
	Lexer::TNameId GetName()const;
	bool IsStatic()const;
};