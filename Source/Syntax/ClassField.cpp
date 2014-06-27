
#include "ClassField.h"
#include "Class.h"
#include "Method.h"
#include "Statements.h"

void TClassField::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	type.AnalyzeSyntax(source);
	is_static = source.TestAndGet(TTokenType::ResWord, TResWord::Static);
	if (is_static)
		need_push_this = false;
	TClassField* curr_field = this;
	do {
		if (curr_field != this) {
			curr_field->type = type;
			curr_field->is_static = is_static;
			curr_field->need_push_this = need_push_this;
			*(TTokenPos*) curr_field = *(TTokenPos*) this;
		}
		curr_field->name = source.NameId();
		source.GetToken(TTokenType::Identifier);
		if (source.Test(TTokenType::Comma)) {
			//curr_field = new TClassField(owner);
			//owner->AddField(curr_field);
			curr_field = owner->AddField(owner);
		}
	} while (source.TestAndGet(TTokenType::Comma));
	source.GetToken(TTokenType::Semicolon);
}

TClassField::~TClassField()
{
}
TClass* TClassField::GetClass()
{
	return type.GetClass();
}
bool TClassField::IsReadOnly()const
{
	return read_only;
}
void TClassField::InitOwner(TClass* use_owner)
{
	owner = use_owner;
	type.InitOwner(use_owner);
}
void TClassField::SetReadOnly(bool use_read_only){
	read_only = use_read_only;
}
TClassField::TClassField(TClass* use_owner) :TVariable(true, TVariableType::ClassField)
, owner(use_owner), type(use_owner), offset(-1)
, is_static(false), read_only(false)
{
}

void TClassField::SetOffset(int use_offset){
	offset = use_offset;
}
int TClassField::GetOffset()const{
	assert(offset != -1);
	return offset;
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
TFormalParam TClassField::PushRefToStack(TNotOptimizedProgram &program)
{
	assert(offset >= 0);
	TOpArray ops_array;
	if (is_static)
		program.Push(TOp(TOpcode::PUSH_GLOBAL_REF, offset), ops_array);
	else
		program.Push(TOp(TOpcode::ADD_OFFSET, offset), ops_array);
	return TFormalParam(type.GetClass(), true, ops_array);
}