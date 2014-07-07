#include "SStatements.h"

#include "../Syntax/Statements.h"
#include "SLocalVar.h"
#include "SMethod.h"
#include "FormalParam.h"
#include "SExpression.h"
#include "SBytecode.h"

class TSStatementBuilder :public TStatementVisitor
{
	TSStatement* return_new_operation;
	TSClass* owner;
	TSMethod* method;
	TSStatements* parent;
public:
	TSStatement* GetResult()
	{
		return return_new_operation;
	}
	TSStatementBuilder( TSClass* owner, TSMethod* method,TSStatements* parent)
	{
		this->owner = owner;
		this->method = method;
		this->parent = parent;
	}
	void Visit(TExpression* op)
	{
		TSExpression* new_node = new TSExpression(owner, method, parent, op);
		new_node->Build();
		return_new_operation = new_node;
	}
	void Visit(TLocalVar* op)
	{
		TSLocalVar* new_node = new TSLocalVar(owner, method, parent, op);
		new_node->Build();
		return_new_operation = new_node;
	}
	void Visit(TReturn* op)
	{
	}
	void Visit(TStatements* op)
	{
		TSStatements* new_node = new TSStatements(owner, method, parent, op);
		new_node->Build();
		return_new_operation = new_node;
	}
	void Visit(TBytecode* op)
	{
		TSBytecode* new_node = new TSBytecode(owner, method, parent, op);
		new_node->Build();
		return_new_operation = new_node;
	}
	void Visit(TWhile* op)
	{
	}
	void Visit(TFor* op)
	{
	}
	void Visit(TIf* op)
	{
	}
};

TSStatements::TSStatements(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TStatements* use_syntax)
	:TSStatement(TStatementType::Statements,use_owner,use_method,use_parent,use_syntax)
{

}

void TSStatements::Build()
{
	for (const std::shared_ptr<TStatement>& st : ((TStatements*)GetSyntax())->statements)
	{
		TSStatementBuilder b(owner, method,this);
		st->Accept(&b);
		statements.push_back(std::shared_ptr<TSStatement>(b.GetResult()));
	}
}

TVariable* TSStatements::GetVar(TNameId name, int sender_id)
{
	for (int i = 0; i < var_declarations.size(); i++)
	{
		if (var_declarations[i].stmt_id <= sender_id
			&&var_declarations[i].pointer->GetName() == name)
			return var_declarations[i].pointer;
	}
	//for(int i=0;i<=statement.GetHigh();i++)
	//	//TODO из-за этого перебора тормоза при большом количестве операторных скобок + невозможность искать локальную переменную за не линейную скорость
	//{
	//	if(except_this&&statement[i]==sender)break;
	//	if(statement[i]->GetType()==TStatementType::VarDecl&&((TLocalVar*)(statement[i]))->GetName()==name)
	//		return ((TLocalVar*)(statement[i]));
	//	if(statement[i]==sender)break;
	//}
	if (parent != NULL)return parent->GetVar(name, GetSyntax()->stmt_id);
	else if (method != NULL)return  method->GetVar(name);
	else return NULL;
}