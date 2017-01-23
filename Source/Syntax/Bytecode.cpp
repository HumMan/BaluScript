
#include "Bytecode.h"

#include "../lexer.h"
#include <stdio.h>

#include "Method.h"
#include "Statements.h"

using namespace Lexer;

TBytecode::TBytecode(TClass* use_owner, TMethod* use_method, TStatements* use_parent, int use_stmt_id)
	:TStatement(TStatementType::Bytecode, use_owner, use_method, use_parent, use_stmt_id)
{
	//TODO
	//method->SetHasReturn(true);
}

void TBytecode::AnalyzeSyntax(Lexer::ILexer* source) {
	InitPos(source);
	source->GetToken(TResWord::Bytecode);
	source->GetToken(TTokenType::LBrace);
	while (source->Test(TTokenType::Bytecode)) {
		int type = source->Token();
		source->GetToken();
		int params_count = 0;
		int params[4];
		TBytecodeOp _op;
		while (source->Test(TValue::Bool) || source->Test(
				TValue::Int) || source->Test(
				TValue::Float) || source->Test(
				TTokenType::Identifier)) {
			if (source->Test(TTokenType::Identifier)) {
				if (source->NameId() != source->GetIdFromName(
						"CreateArrayElementClassId"))
					Error("Неизвестный идентификатор!");
				source->GetToken();
				source->GetToken(TTokenType::LParenth);
				source->TestToken(TTokenType::Identifier);
				//if(params_count<2||params_count>4)
				//	source->Error("Ошибка в выражении");
				//TODO разгрести это+ сделать нормальные константные выражения+ ссылки на локальные переменные
				_op.f[params_count] = TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID;
				_op.id[params_count] = source->NameId();
				source->GetToken();
				params_count++;
				source->GetToken(TTokenType::RParenth);
			} else {
				params[params_count++] = source->GetAttrib();
				source->GetToken();
			}
			if (params_count > 4)
				Error("Слишком много параметров!");
			if (!source->TestAndGet(TTokenType::Comma))
				break;
		}
		int params_count_info = GetBytecodeParamsCount((TOpcode::Enum) type);
		if (params_count != params_count_info) {
			Error("Неправильное количество параметров: для %s нужно %i параметра!", GetBytecodeString((TOpcode::Enum) type), params_count_info);
		}
		TOp op;
		int t = 0;
		op.type = (TOpcode::Enum) type;
		if (GetBytecodeParamExists((TOpcode::Enum) type, 0))
			op.f1 = params[t++];
		if (GetBytecodeParamExists((TOpcode::Enum) type, 1))
			op.f2 = params[t++];
		if (GetBytecodeParamExists((TOpcode::Enum) type, 2))
			op.v1 = params[t++];
		if (GetBytecodeParamExists((TOpcode::Enum) type, 3))
			op.v2 = params[t++];
		_op.op = op;
		code.push_back(_op);
		source->GetToken(TTokenType::Semicolon);
	}
	source->GetToken(TTokenType::RBrace);
}

void TBytecode::Accept(TStatementVisitor* visitor)
{
	visitor->Visit(this);
}

const std::vector<TBytecodeOp>& TBytecode::GetBytecode()
{
	return code;
}