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

class TSExpression_TypeDecl;

class TSExpression_TCreateTempObject : public TSOperation, public SemanticApi::ISOperations::ISExpression_TCreateTempObject
{
private:
	std::unique_ptr<TSExpression_TypeDecl> left;
	std::unique_ptr<TSConstructObject> construct_object;
public:
	TSExpression_TCreateTempObject(TSExpression_TypeDecl* left, TSConstructObject* construct_object);
	//void Build(const std::vector<SyntaxApi::IExpression*>& param_expressions);
	void Accept(ISExpressionVisitor*);

	SemanticApi::ISOperations::ISExpression_TypeDecl* GetLeft()const;
	SemanticApi::ISConstructObject* GetConstructObject()const;
};

class TSExpression_TypeDecl : public TSOperation, public SemanticApi::ISOperations::ISExpression_TypeDecl
{
public:
	TSExpression_TypeDecl(TSClass* owner, SyntaxApi::IType* syntax_node, SemanticApi::TGlobalBuildContext build_context);
	TSType type;
	void Accept(ISExpressionVisitor*);
};

class TSExpression :public TSStatement,public TSOperation, public SemanticApi::ISOperations::ISExpression
{
	std::unique_ptr<SemanticApi::ISOperations::ISOperation> first_op;
public:
	
	
	class TInt : public TSOperation, public SemanticApi::ISOperations::IInt
	{
		int val;
		TSType type;
	public:
		TInt(TSClass* owner, SyntaxApi::IType* syntax_node, int value, SemanticApi::TGlobalBuildContext build_context);		
		void Accept(ISExpressionVisitor*);
		int GetValue()const;
		const SemanticApi::ISType* GetType()const;
	};
	class TFloat : public TSOperation, public SemanticApi::ISOperations::IFloat
	{
		float val;
		TSType type;
	public:
		TFloat(TSClass* owner, SyntaxApi::IType* syntax_node, float value, SemanticApi::TGlobalBuildContext build_context);		
		void Accept(ISExpressionVisitor*);
		float GetValue()const;
		const SemanticApi::ISType* GetType()const;
	};
	class TBool : public TSOperation, public SemanticApi::ISOperations::IBool
	{
		bool val;
		TSType type;
	public:
		TBool(TSClass* owner, SyntaxApi::IType* syntax_node, bool value, SemanticApi::TGlobalBuildContext build_context);		
		void Accept(ISExpressionVisitor*);
		bool GetValue()const;
		const SemanticApi::ISType* GetType()const;
	};
	class TString : public TSOperation, public SemanticApi::ISOperations::IString
	{
		std::string val;
		TSType type;
	public:
		TString(TSClass* owner, SyntaxApi::IType* syntax_node, std::string value, SemanticApi::TGlobalBuildContext build_context);		
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
	TSExpression(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IExpression* syntax_node);
	TSExpression(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IExpression* syntax_node, SemanticApi::ISOperations::ISOperation* first_op);
	void Build(SemanticApi::TGlobalBuildContext build_context);
	SemanticApi::IVariable* GetVar(Lexer::TNameId name);
	SyntaxApi::IExpression* GetSyntax();
	SemanticApi::ISOperations::ISOperation* GetFirstOp()const;
	void Accept(ISExpressionVisitor*);
	void Accept(SemanticApi::ISStatementVisitor* visitor);
};