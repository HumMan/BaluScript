﻿#include "ClassField.h"

#include "Class.h"
#include "Method.h"
#include "Statements.h"
#include "Type.h"

using namespace Lexer;
using namespace SyntaxInternal;

void TClassField::AnalyzeSyntax(Lexer::ILexer* source) {
	InitPos(source);
	type->AnalyzeSyntax(source);
	is_static = source->TestAndGet(TResWord::Static);
	TClassField* curr_field = this;
	do {
		if (curr_field != this) {
			curr_field->type = type;
			curr_field->is_static = is_static;
			*(TTokenPos*) curr_field = *(TTokenPos*) this;
		}
		curr_field->name = source->NameId();
		source->GetToken(TTokenType::Identifier);
		if (source->Test(TTokenType::Comma)) {
			//curr_field = new TClassField(owner);
			//owner->AddField(curr_field);
			curr_field = owner->EmplaceField(owner);
		}
	} while (source->TestAndGet(TTokenType::Comma));
	source->GetToken(TTokenType::Semicolon);
}

SyntaxApi::IType* TClassField::GetType()const
{
	return type.get();
}

bool TClassField::IsReadOnly()const
{
	return read_only;
}

void TClassField::SetReadOnly(bool use_read_only){
	read_only = use_read_only;
}
TClassField::TClassField(TClass* use_owner) :owner(use_owner), type(new TType(use_owner))
, is_static(false), read_only(false)
{
}

TClass* TClassField::GetOwner()const{
	return owner;
}
TNameId TClassField::GetName()const{
	return name;
}
bool TClassField::IsStatic()const{
	return is_static;
}