
class TLocalVar:public TStatement,public TVariable
{
	TType type;
	TNameId name;
	TExpression* assign_expr;
	TVectorList<TExpression> params;
	int offset;
	bool is_static;
	void operator=(const TLocalVar& use_source);
public:
	TLocalVar(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id):
		TStatement(TStatementType::VarDecl,use_owner,use_method,use_parent,use_stmt_id),
		TVariable(false,TVariableType::LocalVar),
		type(use_owner),assign_expr(NULL),
		offset(-1),is_static(false)
	{
	}
	void AnalyzeSyntax(TLexer& source);
	TNameId GetName(){
		return name;
	}
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	~TLocalVar()
	{
		delete assign_expr;
	}
	TClass* GetClass(){
		return type.GetClass();
	}
	int GetOffset(){
		return offset;
	}
	bool IsStatic(){
		return is_static;
	}
	TFormalParam PushRefToStack(TNotOptimizedProgram &program)
	{
		TOpArray ops_array;
		if(is_static)
			program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,offset),ops_array);
		else
			program.Push(TOp(TOpcode::PUSH_LOCAL_REF,offset),ops_array);
		return TFormalParam(type.GetClass(),true,ops_array);
	}
	TStatement* GetCopy()
	{
		TLocalVar* copy=new TLocalVar(*this);
		if(assign_expr!=NULL)
			copy->assign_expr=new TExpression(*assign_expr);
		return copy;
	}
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
		type.InitOwner(use_owner);
		if(assign_expr!=NULL)assign_expr->InitOwner(use_owner,use_method,use_parent);
		for(int i=0;i<=params.GetHigh();i++)
			params[i]->InitOwner(use_owner,use_method,use_parent);
	}

};