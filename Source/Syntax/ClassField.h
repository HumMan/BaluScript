#pragma once

#include "../VirtualMachine/Op.h"
#include "../VirtualMachine/OpArray.h"
#include "Variable.h"
#include "Type.h"
#include "../lexer.h"
#include "Accessible.h"

class TClassField:public TAccessible,public TVariable,public TTokenPos
{
	TClass* owner;
	TNameId name;
	TType type;
	int offset;
	bool is_static;
	bool read_only;
public:
	virtual ~TClassField();
	TClass* GetClass();
	bool IsReadOnly()const;
	void InitOwner(TClass* use_owner);
	void SetReadOnly(bool use_read_only);
	TClassField(TClass* use_owner);
	void AnalyzeSyntax(TLexer& source);
	void SetOffset(int use_offset);
	int GetOffset()const;
	TClass* GetOwner()const;
	TNameId GetName()const;
	bool IsStatic()const;
	TFormalParam PushRefToStack(TNotOptimizedProgram &program);
};