#include "SyntaxTreeApi.h"

#include "SyntaxInternal/Class.h"
#include "SyntaxInternal/Method.h"

namespace SyntaxApi
{
	IClass * Analyze(Lexer::ILexer * lexer)
	{
		auto cl = new SyntaxInternal::TClass(nullptr);
		cl->AnalyzeSyntax(lexer);
		lexer->GetToken(Lexer::TTokenType::Done);
		return cl;
	}

	IClass* AnalyzeNestedClass(Lexer::ILexer * lexer, IClass* _parent)
	{
		auto parent = dynamic_cast<SyntaxInternal::TClass*>(_parent);
		auto cl = new SyntaxInternal::TClass(parent);
		cl->AnalyzeSyntax(lexer);
		lexer->GetToken(Lexer::TTokenType::Done);
		return cl;
	}
	IMethod* AnalyzeNestedMethod(Lexer::ILexer * lexer, IClass* _parent)
	{
		auto parent = dynamic_cast<SyntaxInternal::TClass*>(_parent);
		SyntaxInternal::TMethod* m = new SyntaxInternal::TMethod(parent);
		m->AnalyzeSyntax(lexer);
		lexer->GetToken(Lexer::TTokenType::Done);
		return m;
	}

	void Destroy(IClass* p)
	{
		delete (dynamic_cast<SyntaxInternal::TClass*>(p));
	}
	void Destroy(IMethod* p)
	{
		delete (dynamic_cast<SyntaxInternal::TMethod*>(p));
	}

}
