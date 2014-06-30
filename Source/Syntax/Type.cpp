#include "Type.h"

#include "Class.h"
#include "OverloadedMethod.h"
#include "Method.h"
#include "Statements.h"

TType::TType(TNameId use_class_name, TClass *use_owner) :
	owner(use_owner) {
	class_name.name = use_class_name;
	class_name.member = NULL;
}

TType::TType(TClass *use_owner) :
	owner(use_owner) {
}

void TType::AnalyzeSyntax(TClassName* use_class_name, TLexer& source) {
	source.TestToken(TTokenType::Identifier);
	use_class_name->name = source.NameId();
	use_class_name->member = NULL;
	source.GetToken(TTokenType::Identifier);
	if (source.Test(TTokenType::Operator, TOperator::Less)) {
		source.GetToken();
		while (!source.TestAndGet(TTokenType::Operator, TOperator::Greater)) {
			use_class_name->template_params.emplace_back();
			TClassName* template_param = &(use_class_name->template_params.back());
			AnalyzeSyntax(template_param, source);
			if (!source.Test(TTokenType::Operator, TOperator::Greater))
				source.GetToken(TTokenType::Comma);
		}
	}
	if (source.Test(TTokenType::Dot)) {
		source.GetToken();
		use_class_name->member = std::unique_ptr<TClassName>(new TClassName());
		AnalyzeSyntax(use_class_name->member.get(), source);
	}
}

void TType::AnalyzeSyntax(TLexer& source) {
	InitPos(source);
	AnalyzeSyntax(&class_name, source);
	if (source.Test(TTokenType::LBracket) || source.Test(TTokenType::Operator,
			TOperator::GetArrayElement)) {
		while (true) {
			if (source.TestAndGet(TTokenType::LBracket)) {
				if (source.Test(TTokenType::Value, TValue::Int)) {
					//dim.push_back(source.Int());
					source.GetToken();
				}
				source.GetToken(TTokenType::RBracket);
			} else if (source.TestAndGet(TTokenType::Operator,
					TOperator::GetArrayElement)) {
				//dim.push_back(0);
			} else
				break;
		}
	}
}