#pragma once

namespace SemanticApi
{

	class ISStatementVisitor
	{
	public:
		virtual void Visit(SemanticApi::ISIf* node) = 0;
		virtual void Visit(SemanticApi::ISOperations::ISExpression* node) = 0;
		virtual void Visit(SemanticApi::ISFor* node) = 0;
		virtual void Visit(SemanticApi::ISLocalVar* node) = 0;
		virtual void Visit(SemanticApi::ISReturn* node) = 0;
		virtual void Visit(SemanticApi::ISStatements* node) = 0;
		virtual void Visit(SemanticApi::ISWhile* node) = 0;
		virtual void Visit(SemanticApi::ISBytecode* node) = 0;
	};
}