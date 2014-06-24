
void TBytecode::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	source.GetToken(TTokenType::ResWord, TResWord::Bytecode);
	source.GetToken(TTokenType::LBrace);
	while (source.Test(TTokenType::Bytecode)) {
		int type = source.Token();
		source.GetToken();
		int params_count = 0;
		int params[4];
		TBytecodeOp _op;
		while (source.Test(TTokenType::Value, TValue::Bool) || source.Test(
				TTokenType::Value, TValue::Int) || source.Test(
				TTokenType::Value, TValue::Float) || source.Test(
				TTokenType::Identifier)) {
			if (source.Test(TTokenType::Identifier)) {
				if (source.NameId() != source.GetIdFromName(
						"CreateArrayElementClassId"))
					source.Error("����������� �������������!");
				source.GetToken();
				source.GetToken(TTokenType::LParenth);
				source.TestToken(TTokenType::Identifier);
				//if(params_count<2||params_count>4)
				//	source.Error("������ � ���������");
				//TODO ��������� ���+ ������� ���������� ����������� ��������+ ������ �� ��������� ����������
				_op.f[params_count] = TBytecodeOp::GET_ARR_ELEMENT_CLASS_ID;
				_op.id[params_count] = source.NameId();
				source.GetToken();
				params_count++;
				source.GetToken(TTokenType::RParenth);
			} else {
				params[params_count++] = source.GetAttrib();
				source.GetToken();
			}
			if (params_count > 4)
				source.Error("������� ����� ����������!");
			if (!source.TestAndGet(TTokenType::Comma))
				break;
		}
		int params_count_info = GetBytecodeParamsCount((TOpcode::Enum) type);
		if (params_count != params_count_info) {
			char buf[256];
			sprintf_s(
					buf,
					"������������ ���������� ����������: �� %s ���� %i ���������!",
					GetBytecodeString((TOpcode::Enum) type), params_count_info);
			source.Error(buf);
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
		code.Push(_op);
		source.GetToken(TTokenType::Semicolon);
	}
	source.GetToken(TTokenType::RBrace);
}
