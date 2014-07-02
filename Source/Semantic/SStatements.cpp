#include "SStatements.h"

#include "../Syntax/Statements.h"
#include "SLocalVar.h"
#include "SMethod.h"

TSStatements::TSStatements(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TStatements* use_syntax)
	:TSStatement(TStatementType::Statements,use_owner,use_method,use_parent,use_syntax)
{

}

void TSStatements::Link()
{
	for (const std::shared_ptr<TStatement>& st : ((TStatements*)GetSyntax())->statements)
	{
		statements.push_back(std::shared_ptr<TSStatements>(new TSStatements(owner, method, this, (TStatements*)st.get())));
			//st.Link(GetSyntax().)
	}
}

TSStatement* TSStatements::CreateNode(TStatement* use_syntax_node)
{
	switch (use_syntax_node->GetType())
	{
	case TStatementType::VarDecl:
	{
		TSLocalVar* t = new TSLocalVar(owner, method, this, (TLocalVar*)use_syntax_node);
		var_declarations.push_back(TVarDecl(statements.size() - 1, t));
		return t;
	}break;
	case TStatementType::For:
		break;
	case TStatementType::If:
		break;
	case TStatementType::While:
		break;
	case TStatementType::Return:
		break;
	case TStatementType::Expression:
		break;
	case TStatementType::Bytecode:
		break;
	case TStatementType::Statements:
		break;
	default:
		assert(false);
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