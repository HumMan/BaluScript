#pragma once


class ISExpressionVisitor
{
public:
	virtual void Visit(SemanticApi::ISOperations::ISExpression_TMethodCall*) = 0;
	virtual void Visit(SemanticApi::ISOperations::ISExpression_TGetClass*) = 0;
	virtual void Visit(SemanticApi::ISOperations::ISExpression_TempObjectType*) = 0;
	virtual void Visit(SemanticApi::ISOperations::ISExpression_TCreateTempObject*) = 0;

	virtual void Visit(SemanticApi::ISOperations::IBool*) = 0;
	virtual void Visit(SemanticApi::ISOperations::ISExpression*) = 0;
	virtual void Visit(SemanticApi::ISOperations::IInt*) = 0;
	virtual void Visit(SemanticApi::ISOperations::IFloat*) = 0;
	virtual void Visit(SemanticApi::ISOperations::IString*) = 0;
	virtual void Visit(SemanticApi::ISOperations::IEnumValue*) = 0;
	virtual void Visit(SemanticApi::ISOperations::IGetMethods*) = 0;
	virtual void Visit(SemanticApi::ISOperations::IGetClassField*) = 0;
	virtual void Visit(SemanticApi::ISOperations::IGetParameter*) = 0;
	virtual void Visit(SemanticApi::ISOperations::IGetLocal*) = 0;
	virtual void Visit(SemanticApi::ISOperations::IGetThis*) = 0;
};