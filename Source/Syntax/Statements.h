
class TStatements:public TStatement
{
	struct TVarDecl
	{
		int stmt_id;
		TLocalVar* pointer;
		TVarDecl(){}
		TVarDecl(int use_stmt_id,TLocalVar* use_pointer):stmt_id(use_stmt_id),pointer(use_pointer){}
	};
	int curr_local_var_offset;
	int last_locals_offset;
	TVector<TStatement*> statement;
	TVector<TVarDecl> var_declarations;
	void operator=(const TStatements& use_source);
public:
	void Add(TStatement* use_statement){
		statement.Push(use_statement);
		use_statement->SetStmtId(statement.GetHigh());//TODO не нужно
	}
	void AddVar(TLocalVar* use_var);
	TStatement* GetStatement(int i)
	{
		return statement[i];
	}
	TStatement* GetCopy()
	{
		TStatements* copy=new TStatements(*this);
		return copy;
	}
	TStatements(const TStatements& use_source);
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
		for(int i=0;i<=statement.GetHigh();i++)
			statement[i]->InitOwner(use_owner,use_method,this);
	}
	int GetHigh()
	{
		return statement.GetHigh();
	}
	TStatements(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::Statements,use_owner,use_method,use_parent,use_stmt_id){}
	~TStatements()
	{
		for(int i=0;i<=statement.GetHigh();i++)delete statement[i];
	}
	void AnalyzeStatement(TLexer& source,bool end_semicolon);
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TVariable* GetVar(TNameId name,int sender_id);
	TOpArray BuildLocalsAndParamsDestructor(TNotOptimizedProgram &program,int &deallocate_size);
	int BuildCurrLocalsDestructor(TOpArray& ops_array,TNotOptimizedProgram &program,bool deallocate_stack);
};
