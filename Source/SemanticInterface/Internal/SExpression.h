#pragma once

#include <list>

#include "SStatement.h"
#include "SType.h"
#include "SParameter.h"

class TVariable;
class TSClassField;
class TSParameter;
class TSLocalVar;
class TSConstructObject;

class TSOperation : public Lexer::TTokenPos, public virtual SemanticApi::ISOperations::ISOperation
{
protected:
	TExpressionResult expression_result;
public:
	virtual ~TSOperation(){}
	const SemanticApi::IExpressionResult* GetFormalParam()const;
	TExpressionResult GetFormalParameter()const;
};

class TSExpression_TMethodCall : public TSOperation, public SemanticApi::ISOperations::ISExpression_TMethodCall
{
	TActualParameters params;
	TSMethod* invoke;
	
	bool is_static;
	bool with_external_object;
	
public:
	std::unique_ptr<TSOperation> left;
	SemanticApi::TMethodCallType type;
	TSExpression_TMethodCall(SemanticApi::TMethodCallType type)
	{
		invoke = nullptr;
		this->type = type;
	}
	void Build(const std::vector<TSOperation*>& param_expressions, TSMethod* method);
	//void Build(const std::vector<TSOperation*>& param_expressions);
	
	void Accept(ISExpressionVisitor*);

	const SemanticApi::IActualParameters* GetParams()const;
	SemanticApi::TMethodCallType GetType()const;
	SemanticApi::ISMethod* GetInvoke()const;
	SemanticApi::ISOperations::ISOperation* GetLeft()const;
};

class TSExpression_TGetClass : public TSOperation, public SemanticApi::ISOperations::ISExpression_TGetClass
{
private:
	std::unique_ptr<TSExpression_TGetClass> left;
	TSClass* get_class;
public:
	TSExpression_TGetClass(TSExpression_TGetClass* left, TSClass* get_class);
	void Accept(ISExpressionVisitor*);
};

class TSExpression_TempObjectType;

class TSExpression_TCreateTempObject : public TSOperation, public SemanticApi::ISOperations::ISExpression_TCreateTempObject
{
private:
	std::unique_ptr<TSExpression_TempObjectType> left;
	std::unique_ptr<TSConstructObject> construct_object;
public:
	TSExpression_TCreateTempObject(TSExpression_TempObjectType* left, TSConstructObject* construct_object);
	//void Build(const std::vector<SyntaxApi::IExpression*>& param_expressions);
	void Accept(ISExpressionVisitor*);

	SemanticApi::ISOperations::ISExpression_TempObjectType* GetLeft()const;
	SemanticApi::ISConstructObject* GetConstructObject()const;
};

class TSExpression_TempObjectType : public TSOperation, public SemanticApi::ISOperations::ISExpression_TempObjectType
{
public:
	TSExpression_TempObjectType(TSClass* owner, SyntaxApi::IType* syntax_node);
	TSType type;
	void Accept(ISExpressionVisitor*);
};

class TSExpression :public TSStatement,public TSOperation, public SemanticApi::ISOperations::ISExpression
{
	std::unique_ptr<TSOperation> first_op;
public:
	
	
	class TInt : public TSOperation, public SemanticApi::ISOperations::IInt
	{
	public:
		TInt(TSClass* owner, SyntaxApi::IType* syntax_node);
		int val;
		TSType type;
		void Accept(ISExpressionVisitor*);
		int GetValue()const;
		const SemanticApi::ISType* GetType()const;
	};
	class TFloat : public TSOperation, public SemanticApi::ISOperations::IFloat
	{
	public:
		TFloat(TSClass* owner, SyntaxApi::IType* syntax_node);
		float val;
		TSType type;
		void Accept(ISExpressionVisitor*);
		float GetValue()const;
		const SemanticApi::ISType* GetType()const;
	};
	class TBool : public TSOperation, public SemanticApi::ISOperations::IBool
	{
	public:
		TBool(TSClass* owner, SyntaxApi::IType* syntax_node);
		bool val;
		TSType type;
		void Accept(ISExpressionVisitor*);
		bool GetValue()const;
		const SemanticApi::ISType* GetType()const;
	};
	class TString : public TSOperation, public SemanticApi::ISOperations::IString
	{
	public:
		TString(TSClass* owner, SyntaxApi::IType* syntax_node);
		std::string val;
		TSType type;
		void Accept(ISExpressionVisitor*);
		std::string GetValue()const;
		const SemanticApi::ISType* GetType()const;
	};
	class TEnumValue : public TSOperation, public SemanticApi::ISOperations::IEnumValue
	{
	public:
		TEnumValue(TSClass* owner, TSClass* type);
		int val;
		TSClass* type;
		void Accept(ISExpressionVisitor*);
		SemanticApi::ISClass* GetType()const;
		int GetValue()const;
	};
	class TGetMethods :public TSOperation, public SemanticApi::ISOperations::IGetMethods
	{
	private:
		std::unique_ptr<TSOperation> left;
		TExpressionResult left_result;
		TExpressionResult result;
	public:
		TGetMethods(TSOperation* left, TExpressionResult left_result, TExpressionResult result);
		void Accept(ISExpressionVisitor*);
		SemanticApi::ISOperations::ISOperation* GetLeft()const;
	};
	class TGetClassField :public TSOperation, public SemanticApi::ISOperations::IGetClassField
	{
	private:
		std::unique_ptr<TSOperation>  left;
		TExpressionResult left_result;
		TSClassField* field;
	public:
		TGetClassField(TSOperation* left, TExpressionResult left_result, TSClassField* field);
		void Accept(ISExpressionVisitor*);
		SemanticApi::ISClassField* GetField()const;
		SemanticApi::ISOperations::ISOperation* GetLeft()const;
	};
	class TGetParameter :public TSOperation, public SemanticApi::ISOperations::IGetParameter
	{
	private:
		TSParameter* parameter;
	public:		
		TGetParameter(TSParameter* parameter);
		void Accept(ISExpressionVisitor*);
		SemanticApi::ISParameter* GetParameter()const;
	};
	class TGetLocal :public TSOperation, public SemanticApi::ISOperations::IGetLocal
	{
	private:
		TSLocalVar* variable;
	public:
		TGetLocal(TSLocalVar* variable);
		void Accept(ISExpressionVisitor*);
		SemanticApi::ISLocalVar* GetVariable()const;
	};
	class TGetThis :public TSOperation, public SemanticApi::ISOperations::IGetThis
	{
	private:
		TSClass* owner;
	public:
		TGetThis(TSClass* owner);
		void Accept(ISExpressionVisitor*);
		SemanticApi::ISClass* GetOwner()const;
	};
	
public:
	TSExpression(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IExpression* syntax_node)
		:TSStatement(SyntaxApi::TStatementType::Expression, use_owner, use_method, use_parent, (SyntaxApi::IStatement*)(syntax_node)){}
	void Build(SemanticApi::TGlobalBuildContext build_context);
	SemanticApi::IVariable* GetVar(Lexer::TNameId name);
	SyntaxApi::IExpression* GetSyntax();
	const SemanticApi::IExpressionResult* GetFormalParam()
	{
		assert(first_op != nullptr);
		return first_op->GetFormalParam();
	}
	const TExpressionResult GetFormalParameter()
	{
		assert(first_op != nullptr);
		return first_op->GetFormalParameter();
	}
	SemanticApi::ISOperations::ISOperation* GetFirstOp()const;
	void Accept(ISExpressionVisitor*);
};