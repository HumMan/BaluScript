namespace TClassMember
{
	enum Enum
	{
		Func=TResWord::Func,
		Constr,
		Destr,
		Operator,
		Conversion,
		Enumeration
	};
}

class TMethod:public TTokenPos,public TAccessible
{
	friend class TOverloadedMethod;
private:
	TType ret;
	bool ret_ref;
	TClass* owner;
	bool is_static;
	bool is_extern;
	TOpArray ops_array;
	TVectorList<TParameter> param;

	int params_size;

	TStatements statements;
	bool has_return;
	
	bool declared; //настроены типы параметров и возвращаемого значения
	bool build;    //сгенерировано тело метода
	bool one_instruction; //метод состоит из одной инструкции(без операторных скобок, напр.: "func M:int return 5;"

	TNameId method_name;
	TOperator::Enum operator_type;
	TClassMember::Enum member_type;

	TMethod* pre_event;
	TMethod* post_event;

	int is_bytecode;//0 - not_used; 1 - true; 2 - false
	//если 0 то определяется через statements
	
	//TODO
	//bool only_inline;// метод можно только встраивать, но не вызывать

public:
	void InitOwner(TClass* use_owner)
	{
		owner=use_owner;
		ret.InitOwner(use_owner);
		for(int i=0;i<=param.GetHigh();i++)
			param[i]->InitOwner(use_owner,this);
		statements.InitOwner(use_owner,this,NULL);
	}
	void SetPreEvent(TMethod* use_event)
		//используется для вызова автом. конструктора для пользов/внешнего метода
	{
		//assert(!use_event->IsBytecode());
		pre_event=use_event;//TODO проверить на правильность метода
	}
	void SetPostEvent(TMethod* use_event)
		//используется для вызова автом. дестр. для пользов/внешнего метода
	{
		post_event=use_event;//TODO проверить на правильность метода
	}
	void SetAs(TOpArray use_ops,TClass* use_ret_class_pointer,bool use_ret_ref,bool use_is_static,int use_is_bytecode=0)
	{
		is_bytecode=use_is_bytecode;
		ops_array=use_ops;
		ret.SetAs(use_ret_class_pointer);
		ret_ref=use_ret_ref;
		is_static=use_is_static;
		is_extern=false;
		build=true;
	}
	TMethod* GetPreEvent(){
		return pre_event;
	}
	TMethod* GetPostEvent(){
		return post_event;
	}
	TMethod(TClass* use_owner,TClassMember::Enum use_member_type=TClassMember::Func)
		:ret(use_owner),ret_ref(false),owner(use_owner)
		 ,is_static(false),is_extern(false)
		,params_size(-1)
		,statements(use_owner,this,NULL,-1)
		,has_return(false)
		,declared(false),build(false)
		,one_instruction(false)
		,operator_type(TOperator::End)
		,member_type(use_member_type)
		,pre_event(NULL),post_event(NULL)
		,is_bytecode(0)
		{}
	void ParametersDecl(TLexer& source);
	void AnalyzeSyntax(TLexer& source,bool realization=true);//realization - используется при получении идентификатора метода (т.к. только прототип без тела метода)
	TVariable* GetVar(TNameId name);

	void SetHasReturn(bool use_has_return){
		has_return=use_has_return;
	}
	TNameId GetName(){
		return method_name;
	}
	TClass* GetOwner()const{
		return owner;
	}
	TOperator::Enum GetOperatorType(){
		assert(member_type==TClassMember::Operator);
		return operator_type;
	}
	TClassMember::Enum GetMemberType()const{
		return member_type;
	}
	TOpArray GetOps()const{
		return ops_array;
	}
	TClass* GetRetClass()
	{
		return ret.GetClass();
	}
	int GetParamsSize(){
		assert(params_size!=-1);
		return params_size;
	}
	int GetRetSize();
	TParameter* GetParam(int use_id)
	{
		return param[use_id];
	}
	int GetParamsHigh(){
		return param.GetHigh();
	}
	bool IsReturnRef()const
	{
		return ret_ref;
	}
	bool IsStatic(){
		return is_static;
	}
	bool IsExternal(){
		return is_extern;
	}
	bool IsBytecode()
	{
		if(is_bytecode==0)
			return one_instruction
			&&statements.GetHigh()==0
			&&statements.GetStatement(0)->GetType()==TStatementType::Bytecode;
		else return is_bytecode;
	}
	void AddParam(TParameter* use_param)
	{
		param.Push(use_param);
	}
	void CalcParamSize()
	{
		params_size=0;
		for(int i=0;i<=param.GetHigh();i++)
			params_size+=param[i]->GetSize();
	}
	bool HasParams(TVectorList<TParameter> &use_params)const
	{
		if(use_params.GetHigh()!=param.GetHigh())return false;
		for(int i=0;i<=param.GetHigh();i++)
			if(!param[i]->IsEqualTo(*(use_params[i])))return false;
		return true;
	}
	void CheckForErrors();
	TOpArray BuildParametersDestructor(TNotOptimizedProgram &program);
	void Build(TNotOptimizedProgram &program);
	void Declare();
	void BuildFormalParamConversion(TNotOptimizedProgram &program,TFormalParam& formal_par, TClass* param_class,bool param_ref);
	void BuildFormalParamsConversion(TNotOptimizedProgram &program,TVector<TFormalParam> &formal_params);
	TFormalParam BuildCall(TNotOptimizedProgram &program,TVector<TFormalParam> &formal_params);
	TFormalParam BuildCall(TNotOptimizedProgram &program,TClass* par0,bool par0_ref,TOpArray& par0_ops,TClass* par1,bool par1_ref,TOpArray& par1_ops);
	TFormalParam BuildCall(TNotOptimizedProgram &program,TClass* par0,bool par0_ref,TOpArray& par0_ops);
	TFormalParam BuildCall(TNotOptimizedProgram &program);
};


class TOverloadedMethod
{
private:
	TNameId name;
public:
	TVectorList<TMethod> method;
	TOverloadedMethod(){}
	TOverloadedMethod(TNameId use_name):name(use_name){}
	void InitOwner(TClass* use_owner)
	{
		for(int i=0;i<=method.GetHigh();i++)
			method[i]->InitOwner(use_owner);
	}
	TNameId GetName()const{
		return name;
	}
	TMethod* FindParams(TVectorList<TParameter>& params)
	{
		for (int i=0;i<=method.GetHigh();i++)
			if(method[i]->HasParams(params))return method[i];			
		return NULL;
	}
	TMethod* FindConversion(TVectorList<TParameter>& params,TClass* ret_class)
	{
		for (int i=0;i<=method.GetHigh();i++)
			if(method[i]->HasParams(params)&&method[i]->GetRetClass()==ret_class)return method[i];			
		return NULL;
	}
	bool ParamsExists(TMethod* use_method)
	{
		return FindParams(use_method->param)!=NULL;
	}
	TMethod* FindParams(TMethod* use_method)
	{
		return FindParams(use_method->param);
	}
	void Declare()
	{
		for(int i=0;i<=method.GetHigh();i++)method[i]->Declare();
	}
	void Build(TNotOptimizedProgram &program)
	{
		for(int i=0;i<=method.GetHigh();i++)method[i]->Build(program);
	}
	void GetMethods(TVector<TMethod*> &result)
	{
		for(int i=0;i<=method.GetHigh();i++)
			result.Push(method[i]);
	}
	void CheckForErrors(bool is_conversion=false)
	{
		for (int i=0;i<=method.GetHigh();i++)
		{
			method[i]->CheckForErrors();
			if(is_conversion)
			{
				if(FindConversion(method[i]->param,method[i]->GetRetClass())!=method[i])
					method[i]->Error("Метод с такими параметрами уже существует!");
			}else
			{
				if(FindParams(method[i]->param)!=method[i])
					method[i]->Error("Метод с такими параметрами уже существует!");
			}
		}
	}
};	