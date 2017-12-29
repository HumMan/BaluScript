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

class TSOperation : public Lexer::TTokenPos
{
public:
	///<summary>Получить тип возвращаемого подвыражением значения</summary>
	virtual TExpressionResult GetFormalParameter() = 0;
	virtual ~TSOperation(){}
	//virtual void Run(TExpressionRunContext run_context) = 0;
};

class TSExpression_TMethodCall : public TSOperation
{
	TActualParameters params;
	TSMethod* invoke;
	
	bool is_static;
	bool with_external_object;
public:
	std::unique_ptr<TSOperation> left;
	enum TMethodCallType
	{
		Method,
		ObjectConstructor,
		//ObjectConstructorInitWithAssign,
		Operator,
		//DefaultAssignOperator
	}type;
	TSExpression_TMethodCall(TMethodCallType type)
	{
		invoke = nullptr;
		this->type = type;
	}
	void Build(const std::vector<TSOperation*>& param_expressions, TSMethod* method);
	//void Build(const std::vector<TSOperation*>& param_expressions);
	TExpressionResult GetFormalParameter();
	//void Run(TExpressionRunContext run_context);
};

class TSExpression_TGetClass : public TSOperation
{
public:
	std::unique_ptr<TSExpression_TGetClass> left;
	TSClass* get_class;
	TExpressionResult GetFormalParameter();
	//void Run(TExpressionRunContext run_context);
};

class TSExpression_TempObjectType;

class TSExpression_TCreateTempObject : public TSOperation
{
	
public:
	std::unique_ptr<TSExpression_TempObjectType> left;
	std::unique_ptr<TSConstructObject> construct_object;
	void Build(const std::vector<SyntaxApi::IExpression*>& param_expressions);
	TExpressionResult GetFormalParameter();
	//void Run(TExpressionRunContext run_context);
};

class TSExpression_TempObjectType : public TSOperation
{
public:
	TSExpression_TempObjectType(TSClass* owner, SyntaxApi::IType* syntax_node);
	TSType type;
	TExpressionResult GetFormalParameter();
	//void Run(TExpressionRunContext run_context);
};

class TSExpression :public TSStatement,public TSOperation
{
	std::unique_ptr<TSOperation> first_op;
public:
	
	
	class TInt : public TSOperation
	{
	public:
		TInt(TSClass* owner, SyntaxApi::IType* syntax_node);
		int val;
		TSType type;
		TExpressionResult GetFormalParameter();
		//void Run(TExpressionRunContext run_context);
	};
	class TFloat : public TSOperation
	{
	public:
		TFloat(TSClass* owner, SyntaxApi::IType* syntax_node);
		float val;
		TSType type;
		TExpressionResult GetFormalParameter();
		//void Run(TExpressionRunContext run_context);
	};
	class TBool : public TSOperation
	{
	public:
		TBool(TSClass* owner, SyntaxApi::IType* syntax_node);
		bool val;
		TSType type;
		TExpressionResult GetFormalParameter();
		//void Run(TExpressionRunContext run_context);
	};
	class TString : public TSOperation
	{
	public:
		TString(TSClass* owner, SyntaxApi::IType* syntax_node);
		std::string val;
		TSType type;
		TExpressionResult GetFormalParameter();
		////void Run(TExpressionRunContext run_context);
	};
	class TEnumValue : public TSOperation
	{
	public:
		TEnumValue(TSClass* owner, TSClass* type);
		int val;
		TSClass* type;
		TExpressionResult GetFormalParameter();
		//void Run(TExpressionRunContext run_context);
	};
	class TGetMethods :public TSOperation
	{
	public:
		std::unique_ptr<TSOperation> left;
		TExpressionResult left_result;
		TExpressionResult result;
		TExpressionResult GetFormalParameter();
		//void Run(TExpressionRunContext run_context);
	};
	class TGetClassField :public TSOperation
	{
	public:
		std::unique_ptr<TSOperation>  left;
		TExpressionResult left_result;
		TSClassField* field;
		TExpressionResult GetFormalParameter();
		//void Run(TExpressionRunContext run_context);
	};
	class TGetParameter :public TSOperation
	{
	public:
		TSParameter* parameter;
		TExpressionResult GetFormalParameter();
		//void Run(TExpressionRunContext run_context);
	};
	class TGetLocal :public TSOperation
	{
	public:
		TSLocalVar* variable;
		TExpressionResult GetFormalParameter();
		//void Run(TExpressionRunContext run_context);
	};
	class TGetThis :public TSOperation
	{
	public:
		TSClass* owner;
		TExpressionResult GetFormalParameter();
		//void Run(TExpressionRunContext run_context);
	};
	
public:
	TSExpression(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, SyntaxApi::IExpression* syntax_node)
		:TSStatement(SyntaxApi::TStatementType::Expression, use_owner, use_method, use_parent, (SyntaxApi::IStatement*)(syntax_node)){}
	void Build(SemanticApi::TGlobalBuildContext build_context);
	SemanticApi::IVariable* GetVar(Lexer::TNameId name);
	SyntaxApi::IExpression* GetSyntax();
	TExpressionResult GetFormalParameter()
	{
		if (first_op != nullptr)
			return first_op->GetFormalParameter();
		else 
			return TExpressionResult();
	}
	//void Run(TStatementRunContext run_context);
	//void Run(TExpressionRunContext run_context);
};