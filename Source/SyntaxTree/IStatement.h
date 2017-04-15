#pragma once

namespace SyntaxApi
{
	enum class TStatementType
	{
		VarDecl,
		For,
		If,
		While,
		Return,
		Expression,
		Bytecode,
		Statements
	};

	class IStatement: public virtual Lexer::ITokenPos
	{
	public:
		virtual int GetStmtId()const = 0;
		virtual IMethod* GetMethod()const = 0;
		virtual IStatements* GetParent()const = 0;
		virtual IClass* GetOwner()const = 0;
		virtual TStatementType GetType()const = 0;
		virtual void Accept(IStatementVisitor* visitor) = 0;
	};

	class IStatementVisitor
	{
	public:
		virtual void Visit(IBytecode* op) = 0;
		virtual void Visit(IExpression* op) = 0;
		virtual void Visit(IFor* op) = 0;
		virtual void Visit(IIf* op) = 0;
		virtual void Visit(ILocalVar* op) = 0;
		virtual void Visit(IReturn* op) = 0;
		virtual void Visit(IStatements* op) = 0;
		virtual void Visit(IWhile* op) = 0;
	};
}