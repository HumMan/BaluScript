
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
