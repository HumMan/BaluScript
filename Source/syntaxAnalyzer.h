#pragma once

class TClass;
class TVariable;
class TMethod;

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

namespace TTypeOfAccess
{
	enum Enum
	{
		Private,
		Protected,
		Public
	};
}

namespace TStatementType
{
	enum Enum
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
}

namespace TVariableType
{
	enum Enum
	{
		ClassField,
		Parameter,
		LocalVar
	};
}

struct TOpArray
{
	TListItem<TOp>* first;
	TListItem<TOp>* last;
	TOpArray():first(NULL),last(NULL){}
	bool IsNull()const
	{
		assert((first==NULL)==(last==NULL));
		return first==NULL;
	}
	TOpArray operator+(const TOpArray& use_right)const
	{
		if(first==NULL)return use_right;
		if(use_right.first==NULL)return *this;

		last->next=use_right.first;
		use_right.first->prev=last;
		TOpArray result;
		result.first=first;
		result.last=use_right.last;
		return result;
	}
	void operator+=(const TOpArray& use_right)
	{
		assert((first==NULL)==(last==NULL));
		*this=*this+use_right;
	}
};

class TNotOptimizedProgram
{
private:
	//TList<TOp> instructions;
	TAllocator<TListItem<TOp>,1023> instr_alloc;
	int curr_label;

	TVector<char*> string_consts;
#ifndef NDEBUG
	TVector<TOp*> ops;
#endif
	TVector<TMethod*> methods_table;
	TVector<TClass*> array_element_classes;
public:

	TOpArray static_vars_init;
	TOpArray static_vars_destroy;

	int static_vars_size;

	TVectorList<TMethod> internal_methods;

	TNotOptimizedProgram();
	~TNotOptimizedProgram();
	void ListItems();
	void CreateOptimizedProgram(TProgram& optimized,TTime& time);
	int AddStringConst(TNameId string);
	int GetUniqueLabel();
	void Push(TOp use_op, TOpArray &ops_array);
	void PushFront(TOp use_op, TOpArray &ops_array);
	int AddMethodToTable(TMethod* use_method);
	int CreateArrayElementClassId(TClass* use_class);
	int FindMethod(TMethod* use_method);
};

class TFormalParam
{
	TClass* class_pointer;
	TClass* type;//указатель на тип (для доступа к статическим членам и функциям)
	bool is_ref;
	TOpArray ops;
	TVector<TMethod*> methods;//указатель на перегруженые методы
	bool need_push_this;//если вызывются методы класса из самого класса 
	void Init(){
		class_pointer=NULL;
		is_ref=false;
		need_push_this=false;
		type=NULL;
	}
public:
	TFormalParam(){
		Init();
	}
	TFormalParam(TVector<TMethod*> use_methods, bool use_need_push_this){
		Init();
		methods=use_methods;
		need_push_this=use_need_push_this;
	}
	TFormalParam(TClass* use_class,bool use_is_ref,TOpArray use_ops){
		Init();
		class_pointer=use_class;
		is_ref=use_is_ref;
		ops=use_ops;
	}
	TFormalParam(TClass* use_type){
		Init();
		type=use_type;
	}
	bool IsRef()const{
		return is_ref;
	}
	void SetIsRef(bool use_is_ref){
		is_ref=use_is_ref;
	}
	bool NeedPushThis()const{
		return need_push_this;
	}
	bool IsMethods()const{
		return methods.GetCount()!=0;
	}
	bool IsType()const{
		return type!=NULL;
	}
	TClass* GetType()const{
		return type;
	}
	TVector<TMethod*>& GetMethods(){
		return methods;
	}
	TClass* GetClass()const{
		return class_pointer;
	}
	bool IsVoid()const{
		return class_pointer==NULL&&(!IsMethods())&&type==NULL;//TODO в дальнейшем methods_pointer не должен считаться void
	}
	TOpArray& GetOps(){
		return ops;
	}
	TOpArray GetOps()const{
		return ops;
	}
};

inline static TFormalParam operator+(const TOpArray& use_left,const TFormalParam& use_right)
{
	TFormalParam result(use_right);
	result.GetOps()=use_left+use_right.GetOps();
	return result;
}

class TVoid:public TFormalParam
{
public:
	TVoid(){}
	TVoid(TOpArray use_ops):TFormalParam(NULL,false,use_ops){}
};

class TTokenPos
{
	int token_id;
public:
	TLexer* source;
	void InitPos(TLexer& use_source)
	{
		source=&use_source;
		token_id=use_source.GetCurrentToken();
	}
	void Error(char* s,...)
	{
		va_list args;
        va_start(args, s);
		source->Error(s,token_id,args);
	}
};

struct TType:public TTokenPos
{
	friend class TTypes;
	struct TClassName
	{
		TNameId name;
		TClass* class_pointer;
		TVectorList<TClassName> template_params;
		TSmartPointer<TClassName> member;
		TClass* Build(bool use_build_methods,TClass* use_curr_class,TClass* owner,TTokenPos& source,TNotOptimizedProgram* program);
		TClassName():class_pointer(NULL){}
	};
private:
	TClass* owner;
	TVector<int> dim;
	TClassName class_name;
	TClass* class_pointer;
	void AnalyzeSyntax(TClassName* use_class_name,TLexer& source);
public:
	bool IsEqualTo(const TType& use_right)const
	{
		assert(class_name.class_pointer!=NULL&&use_right.class_name.class_pointer!=NULL);
		return class_name.class_pointer==use_right.class_name.class_pointer;
	}
	void InitOwner(TClass* use_owner)
	{
		owner=use_owner;
	}
	void SetAs(TClass* use_class_pointer)
	{
		assert(class_name.class_pointer==NULL);
		class_name.class_pointer=use_class_pointer;
	}
	TClass* GetClass(bool use_build_methods=false,TNotOptimizedProgram* program=NULL);
	TType(TClass* use_owner);
	TType(TNameId use_class_name,TClass* use_owner);
	void AnalyzeSyntax(TLexer& source);
};

class TVariable
{
protected:
	bool need_push_this;
	TVariableType::Enum var_type;
public:
	TVariable(bool use_need_push_this,TVariableType::Enum use_var_type):need_push_this(use_need_push_this),var_type(use_var_type){}
	TVariableType::Enum GetType()const{
		return var_type;
	}
	virtual TFormalParam PushRefToStack(TNotOptimizedProgram &program)=0;
	bool NeedPushThis()const{
		return need_push_this;
	}
};

class TAccessible
{
protected:
	TTypeOfAccess::Enum access;
public:
	TAccessible():access(TTypeOfAccess::Public){}
	TTypeOfAccess::Enum GetAccess()const{
		return access;
	}
	void SetAccess(TTypeOfAccess::Enum use_access){
		access=use_access;
	}
};

class TClassField:public TAccessible,public TVariable,public TTokenPos
{
	TClass* owner;
	TNameId name;
	TType type;
	int offset;
	bool is_static;
	bool read_only;
public:
	virtual ~TClassField(){}
	TClass* GetClass(){
		return type.GetClass();
	}
	bool IsReadOnly()const{
		return read_only;
	}
	void InitOwner(TClass* use_owner)
	{
		owner=use_owner;
		type.InitOwner(use_owner);
	}
	void SetReadOnly(bool use_read_only){
		read_only=use_read_only;
	}
	TClassField(TClass* use_owner):TVariable(true,TVariableType::ClassField)
		,owner(use_owner),type(use_owner),offset(-1)
		,is_static(false),read_only(false)
	{
	}
	void AnalyzeSyntax(TLexer& source);
	void SetOffset(int use_offset){
		offset=use_offset;
	}
	int GetOffset()const{
		assert(offset!=-1);
		return offset;
	}
	TClass* GetOwner()const{
		return owner;
	}
	TNameId GetName()const{
		return name;
	}
	bool IsStatic()const{
		return is_static;
	}
	TFormalParam PushRefToStack(TNotOptimizedProgram &program)
	{
		assert(offset>=0);
		TOpArray ops_array;
		if(is_static)
			program.Push(TOp(TOpcode::PUSH_GLOBAL_REF,offset),ops_array);
		else
			program.Push(TOp(TOpcode::ADD_OFFSET,offset),ops_array);
		return TFormalParam(type.GetClass(),true,ops_array);
	}
};

class TParameter:public TVariable,public TTokenPos
{
	TType type;
	bool is_ref;
	TNameId name;
	TClass* owner;
	TMethod* method;
	int offset;
public:
	TParameter(TClass* use_owner,TMethod* use_method):TVariable(false,TVariableType::Parameter)
		,type(use_owner),is_ref(false)
		,owner(use_owner),method(use_method),offset(-1)
	{
	}
	virtual ~TParameter(){}
	void InitOwner(TClass* use_owner,TMethod* use_method)
	{
		owner=use_owner;
		method=use_method;
		type.InitOwner(use_owner);
	}
	bool IsEqualTo(const TParameter& right)const
	{
		return type.IsEqualTo(right.type)&&is_ref==right.is_ref;
	}
	void SetAs(bool use_is_ref,TClass* use_class_pointer)
	{
		is_ref=use_is_ref;
		type.SetAs(use_class_pointer);
	}
	void AnalyzeSyntax(TLexer& source);
	TNameId GetName()const{
		return name;
	}
	TClass* GetClass(){
		return type.GetClass();
	}
	TNameId GetClassName()const;
	void SetOffset(int use_offset){
		offset=use_offset;
	}
	int GetSize();
	int GetOffset(){
		return offset;
	}
	bool IsRef()const{
		return is_ref;
	}
	TFormalParam PushRefToStack(TNotOptimizedProgram &program)
	{
		TOpArray ops_array;
		if(is_ref)
			program.Push(TOp(TOpcode::PUSH_LOCAL_REF_COPY,offset),ops_array);
		else
			program.Push(TOp(TOpcode::PUSH_LOCAL_REF,offset),ops_array);
		return TFormalParam(type.GetClass(),true,ops_array);
	}
};

class TStatements;

class TStatement:public TTokenPos
{
protected:
	TMethod* method;
	TStatements* parent;
	TClass* owner;
	int stmt_id;//порядковый номер блока в родительском блоке
	TStatementType::Enum stmt_type;
public:
	void SetStmtId(int use_id){
		stmt_id=use_id;
	}
	TStatement(TStatementType::Enum use_stmt_type,TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id):
		method(use_method),parent(use_parent),owner(use_owner),
		stmt_id(use_stmt_id),stmt_type(use_stmt_type){}
	virtual ~TStatement(){}
	TStatementType::Enum GetType(){
		return stmt_type;
	}
	virtual TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset)=0;
	void _InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		owner=use_owner;
		method=use_method;
		parent=use_parent;
	}
	virtual void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)=0;
	/*{
		owner=use_owner;
		method=use_method;
		parent=use_parent;
	}*/
	virtual TOpArray BuildLocalsAndParamsDestructor(TNotOptimizedProgram &program,int &deallocate_size);
	virtual int BuildCurrLocalsDestructor(TOpArray& ops_array,TNotOptimizedProgram &program,bool deallocate_stack)
	{
		assert(false);
		return -1;
	}
	void TestBoolExpr(TNotOptimizedProgram &program,TFormalParam& compare_result,int go_false_label);
	virtual TStatement* GetCopy()=0;
};

class TLocalVar;

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

class TExpression:public TStatement
{
	//TODO доделать обратную польскую нотацию
	struct TPostfixOp
	{
		enum Type
		{
			OP_ID,
			OP_OPERATOR,
			OP_GET_MEMBER,
			OP_CALL_PARAMS,
			OP_GET_ARRAY_ELEM,
			OP_INT,
			OP_FLOAT,
			OP_BOOL,
			OP_LPARENTH,
			OP_RPARENTH,
			OP_LBRACKET,
			OP_RBRACKET
		};
		Type type;
		int operator_type;
		TNameId id;
		int int_val;
		float float_val;
	};
	TVector<TPostfixOp> stack;
	TVector<TPostfixOp> out;
	TVector<int> methods;
	void BuildPostfix();
	//
	class TOperation: public TTokenPos
	{
	public:
		virtual TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent)=0;
		virtual ~TOperation(){}
		virtual void InitOwner(TClass* use_owner)=0;//нужно для получения типов литералов
		virtual TOperation* GetCopy()=0;
	};
	class TBinOp:public TOperation
	{
		TOperation *left,*right;
		TOperator::Enum op;
	public:
		TBinOp(TOperation *use_left,TOperation *use_right,TOperator::Enum use_op):left(use_left),right(use_right),op(use_op){}
		~TBinOp()
		{
			delete left;
			delete right;
		}
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent);
		void InitOwner(TClass* use_owner)
		{
			left->InitOwner(use_owner);
			right->InitOwner(use_owner);
		}
		TOperation* GetCopy()
		{
			TBinOp* copy=new TBinOp(*this);
			copy->left=left->GetCopy();
			copy->right=right->GetCopy();
			return copy;
		}
	};
	class TUnaryOp:public TOperation
	{
		TOperation *left;
		TOperator::Enum op;
	public:
		TUnaryOp(TOperation *use_left,TOperator::Enum use_op):left(use_left),op(use_op){}
		~TUnaryOp()
		{
			delete left;
		}
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent);
		void InitOwner(TClass* use_owner)
		{
			left->InitOwner(use_owner);
		}
		TOperation* GetCopy()
		{
			TUnaryOp* copy=new TUnaryOp(*this);
			copy->left=left->GetCopy();
			return copy;
		}
	};
	class TIntValue:public TOperation
	{
		int val;
		TType type;
	public:
		TIntValue(int use_val,TNameId int_class_name,TClass* use_owner):val(use_val),type(int_class_name,use_owner){}
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent)
		{
			TOpArray ops_array;
			program.Push(TOp(TOpcode::PUSH,val),ops_array);
			return TFormalParam(type.GetClass(),false,ops_array);
		}
		void InitOwner(TClass* use_owner)
		{
			type.InitOwner(use_owner);
		}
		TOperation* GetCopy()
		{
			return new TIntValue(*this);
		}
	};
	class TFloatValue:public TOperation
	{
		float val;
		TType type;
	public:
		TFloatValue(float use_val,TNameId float_class_name,TClass* use_owner):val(use_val),type(float_class_name,use_owner){}
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent)
		{
			TOpArray ops_array;
			program.Push(TOp(TOpcode::PUSH,*(int*)&val),ops_array);
			return TFormalParam(type.GetClass(),false,ops_array);
		}
		void InitOwner(TClass* use_owner)
		{
			type.InitOwner(use_owner);
		}
		TOperation* GetCopy()
		{
			return new TFloatValue(*this);
		}
	};
	class TBoolValue:public TOperation
	{
		bool val;
		TType type;
	public:
		TBoolValue(bool use_val,TNameId bool_class_name,TClass* use_owner):val(use_val),type(bool_class_name,use_owner){}
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent)
		{
			TOpArray ops_array;
			program.Push(TOp(TOpcode::PUSH,val),ops_array);
			return TFormalParam(type.GetClass(),false,ops_array);
		}
		void InitOwner(TClass* use_owner)
		{
			type.InitOwner(use_owner);
		}
		TOperation* GetCopy()
		{
			return new TBoolValue(*this);
		}
	};
	class TStringValue:public TOperation
	{
		TNameId val;
		TType type;
	public:
		TStringValue(TNameId use_val,TNameId string_class_name,TClass* use_owner):val(use_val),type(string_class_name,use_owner){}
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent)
		{
			TOpArray ops_array;
			int id=program.AddStringConst(val);
			program.Push(TOp(TOpcode::PUSH_STRING_CONST,id),ops_array);
			return TFormalParam(type.GetClass(),false,ops_array);
		}
		void InitOwner(TClass* use_owner)
		{
			type.InitOwner(use_owner);
		}
		TOperation* GetCopy()
		{
			return new TStringValue(*this);
		}
	};
	class TCharValue:public TOperation
	{
		char val;
		TType type;
	public:
		TCharValue(char use_val,TNameId char_class_name,TClass* use_owner):val(use_val),type(char_class_name,use_owner){}
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent)
		{
			TOpArray ops_array;
			program.Push(TOp(TOpcode::PUSH,val),ops_array);
			return TFormalParam(type.GetClass(),false,ops_array);
		}
		void InitOwner(TClass* use_owner)
		{
			type.InitOwner(use_owner);
		}
		TOperation* GetCopy()
		{
			return new TCharValue(*this);
		}
	};
	class TGetMemberOp:public TOperation
	{
		TOperation *left;
		TNameId name;
	public:
		TGetMemberOp(TOperation *use_left,TNameId use_member):left(use_left),name(use_member){}
		~TGetMemberOp()
		{
			delete left;
		}
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent);
		void InitOwner(TClass* use_owner)
		{
			left->InitOwner(use_owner);
		}
		TOperation* GetCopy()
		{
			TGetMemberOp* copy=new TGetMemberOp(*this);
			copy->left=left->GetCopy();
			return copy;
		}
	};
	class TCallParamsOp:public TOperation
	{
		TOperation *left;
		TVectorList<TOperation> param;
		bool is_bracket;
	public:
		TCallParamsOp(){}
		TCallParamsOp(TOperation *use_left,bool use_is_bracket):left(use_left),is_bracket(use_is_bracket)
		{
		}
		void AddParam(TOperation* use_param)
		{
			param.Push(use_param);
		}
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent);
		~TCallParamsOp()
		{
			delete left;
		}
		void InitOwner(TClass* use_owner)
		{
			left->InitOwner(use_owner);
		}
		TOperation* GetCopy()
		{
			TCallParamsOp* copy=new TCallParamsOp();
			*((TOperation*)copy)=*this;
			copy->is_bracket=is_bracket;
			copy->left=left->GetCopy();
			for(int i=0;i<=param.GetHigh();i++)
				copy->param.Push(param[i]->GetCopy());
			return copy;
		}
	};
	class TId:public TOperation
	{
	public:
		TNameId name;
		TId(TNameId use_name):name(use_name){}
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent);
		void InitOwner(TClass* use_owner)
		{}
		TOperation* GetCopy()
		{
			return new TId(*this);
		}
	};

	class TThis:public TOperation
	{
	public:
		TFormalParam Build(TNotOptimizedProgram &program,TExpression* parent);
		void InitOwner(TClass* use_owner)
		{}
		TOperation* GetCopy()
		{
			return new TThis(*this);
		}
	};

	TOperation* first_op;

	TOperation* ParamsCall(TLexer& source,TOperation* curr_operation);
	TOperation* Factor(TLexer& source);
	TOperation* SpecialPostfix(TLexer& source,TOperation* curr_operation);
	TOperation* Expr(TLexer& source,int curr_prior_level);
	void operator=(const TExpression& use_source);
public:
	void AnalyzeSyntax(TLexer& source);
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
		if(first_op!=NULL)
		{
			first_op->InitOwner(use_owner);
		}
	}
	TStatement* GetCopy()
	{
		TExpression* copy=new TExpression(*this);
		//TODO из за таких путаниц с конструкторами запарки с утечками памяти
		return copy;
	}
	TExpression(const TExpression &use_source):TStatement(use_source)
	{
		first_op=use_source.first_op->GetCopy();
	}

	TExpression(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::Expression,use_owner,use_method,use_parent,use_stmt_id),first_op(NULL){}
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset)
	{
		if(first_op!=NULL)
		{
			return first_op->Build(program,this);
		}
		else return TVoid();
	}
	~TExpression()
	{
		if(first_op!=NULL)
			delete first_op;
	}
	TVariable* GetVar(TNameId name)
	{
		return parent->GetVar(name,stmt_id);
	}
};

class TBytecode:public TStatement
{
	
	struct TBytecodeOp
	{
		enum TOpParamType
		{
			VALUE,
			GET_ARR_ELEMENT_CLASS_ID
		};
		TOpParamType f[2];
		TNameId id[2];
		TOp op;
		TBytecodeOp()
		{
			f[0]=VALUE;
			f[1]=VALUE;
		}
	};
	TVector<TBytecodeOp> code;
	void operator=(const TBytecode& use_source);
public:
	TBytecode(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::Bytecode,use_owner,use_method,use_parent,use_stmt_id)
	{
		method->SetHasReturn(true);
	}
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
	}
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TStatement* GetCopy()
	{
		return new TBytecode(*this);
	}
};

class TReturn:public TStatement
{
	TExpression result;
	void operator=(const TReturn& use_source);
public:
	TReturn(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::Return,use_owner,use_method,use_parent,use_stmt_id)//TODO вроде здесь родитель и id не нужен вовсе
		,result(use_owner,use_method,use_parent,use_stmt_id)
	{}
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TStatement* GetCopy()
	{
		return new TReturn(*this);
	}
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
		result.InitOwner(use_owner,use_method,use_parent);
	}
};

class TIf:public TStatement
{
	TExpression bool_expr;
	TStatements statements,else_statements;
	void operator=(const TIf& use_source);
public:
	TIf(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::If,use_owner,use_method,use_parent,use_stmt_id)
		,bool_expr(use_owner,use_method,use_parent,use_stmt_id)
		,statements(use_owner,use_method,use_parent,use_stmt_id)
		,else_statements(use_owner,use_method,use_parent,use_stmt_id)
	{}
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TStatement* GetCopy()
	{
		return new TIf(*this);
	}
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
		bool_expr.InitOwner(use_owner,use_method,use_parent);
		statements.InitOwner(use_owner,use_method,use_parent);
		else_statements.InitOwner(use_owner,use_method,use_parent);
	}
};

class TFor:public TStatement
{
	TExpression compare;
	TStatements increment;
	TStatements statements;
	void operator=(const TFor& use_source);
public:
	TFor(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::For,use_owner,use_method,use_parent,use_stmt_id)
		,compare(use_owner,use_method,use_parent,use_stmt_id)
		,increment(use_owner,use_method,use_parent,use_stmt_id)
		,statements(use_owner,use_method,use_parent,use_stmt_id)
	{}
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TStatement* GetCopy()
	{
		return new TFor(*this);
	}
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
		compare.InitOwner(use_owner,use_method,use_parent);
		increment.InitOwner(use_owner,use_method,use_parent);
		statements.InitOwner(use_owner,use_method,use_parent);
	}
};

class TWhile:public TStatement
{
	TExpression compare;
	TStatements statements;
	void operator=(const TFor& use_source);
public:
	TWhile(TClass* use_owner,TMethod* use_method,TStatements* use_parent,int use_stmt_id)
		:TStatement(TStatementType::While,use_owner,use_method,use_parent,use_stmt_id)
		,compare(use_owner,use_method,use_parent,use_stmt_id)
		,statements(use_owner,use_method,use_parent,use_stmt_id)
	{}
	void AnalyzeSyntax(TLexer& source);
	TFormalParam Build(TNotOptimizedProgram &program,int& local_var_offset);
	TStatement* GetCopy()
	{
		return new TWhile(*this);
	}
	void InitOwner(TClass* use_owner,TMethod* use_method,TStatements* use_parent)
	{
		TStatement::_InitOwner(use_owner,use_method,use_parent);
		compare.InitOwner(use_owner,use_method,use_parent);
		statements.InitOwner(use_owner,use_method,use_parent);
	}
};

//TODO из-за statements тормоза при большом количестве кода вида, видимо приходится просматривать все statements имеющиеся до
//{
//TDynArray<int> dyn_test,dyn_test2;
//TDynArray<TDynArray<int>> dyn_dyn_test;
//}
//{
//TDynArray<int> dyn_test,dyn_test2;
//TDynArray<TDynArray<int>> dyn_dyn_test;
//}
//{
//TDynArray<int> dyn_test,dyn_test2;
//TDynArray<TDynArray<int>> dyn_dyn_test;
//}

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

class TTemplateRealizations
{
public:
	struct TTemplateRealization
	{
		TClass* template_pointer;
		TVectorList<TClass> realizations;
	};
	TVectorList<TTemplateRealization> templates;
	TVectorList<TClass>* FindTemplate(TClass* use_template)
	{
		for(int i=0;i<=templates.GetHigh();i++)
			if(templates[i]->template_pointer==use_template)
				return &templates[i]->realizations;
		return NULL;
	}
};

class TClass:public TTokenPos
{
	bool is_enum;
	bool is_template;
	bool methods_declared;//для всех методов настроены типы параметров
	bool methods_build;//построены тела методов
	bool auto_methods_build;//построены тела автоматическких методов
	int size;//если не "-1" то проверены типы членов класса на зацикливание и настроен размер класса

	struct TTemplateParam
	{
		TNameId name;
		TClass* class_pointer;
	};

	TVector<TTemplateParam> template_params;

	TTemplateRealizations* templates;

	TVectorList<TClassField> field;
	TVectorList<TOverloadedMethod> method;
	TOverloadedMethod constructor;
	TSmartPointer<TMethod> destructor;
	TOverloadedMethod operators[TOperator::End];
	TOverloadedMethod conversion;
	TVector<TNameId> enums;

	TVectorList<TClass> nested_class;

	TSmartPointer<TMethod> auto_def_constr;
	bool constr_override;//имеется пользовательский конструктор по умолчанию
	TSmartPointer<TMethod> auto_destr;

	TNameId name;
	TType parent;
	bool is_sealed;

	TClass* owner;

	void ValidateSizes(TVector<TClass*> &owners);
public:
	TClass(TClass* use_owner,TTemplateRealizations* use_templates);
	void InitOwner(TClass* use_owner)
	{
		owner=use_owner;
		//при копировании надо перенастраивать указатель owner у всех
		for(int i=0;i<=field.GetHigh();i++)field[i]->InitOwner(this);
		for(int i=0;i<=method.GetHigh();i++)method[i]->InitOwner(this);
		constructor.InitOwner(this);
		if(!destructor.IsNull())
			destructor->InitOwner(this);
		for(int i=0;i<TOperator::End;i++)operators[i].InitOwner(this);
		conversion.InitOwner(this);
		for(int i=0;i<=nested_class.GetHigh();i++)nested_class[i]->InitOwner(this);
	}
	
	void AnalyzeSyntax(TLexer& source);

	void CreateInternalClasses();

	void BuildClass();
	void BuildClass(TVector<TClass*> buff);
	void DeclareMethods();
	void CheckForErrors();
	void InitAutoMethods(TNotOptimizedProgram &program);
	void BuildMethods(TNotOptimizedProgram &program);

	void AccessDecl(TLexer& source,bool& readonly, TTypeOfAccess::Enum& access);

	bool IsTemplate(){
		return is_template;
	}
	bool IsEnum(){
		return is_enum;
	}
	int GetEnumId(TNameId use_enum)
	{
		assert(is_enum);
		for(int i=0;i<=enums.GetHigh();i++)
			if(enums[i]==use_enum)return i;
		return -1;
	}
	void SetTemplateParamClass(int id,TClass* use_class){
		template_params[id].class_pointer=use_class;
	}
	void SetIsTemplate(bool use_is_template){
		is_template=use_is_template;
	}
	TClass* GetTemplateParamClass(int id){
		return template_params[id].class_pointer;
	}
	int GetTemplateParamsHigh(){
		return template_params.GetHigh();
	}
	bool IsChildOf(TClass* use_parent);
	void AddMethod(TMethod* use_method,TNameId name);
	void AddOperator(TOperator::Enum op,TMethod* use_method);
	void AddConversion(TMethod* method);
	void AddConstr(TMethod* use_method);
	void AddDestr(TMethod* use_method);
	void AddNested(TClass* use_class);
	void AddField(TClassField* use_field);
	TClass* GetNested(TNameId name);
	TClass* GetClass(TNameId use_name);
	TNameId GetName(){
		return name;
	}
	int GetSize(){
		return size;
	}
	TClass* GetOwner(){
		return owner;
	}
	TClass* GetParent(){
		return parent.GetClass();
	}
	TTemplateRealizations* GetTemplates(){
		return templates;
	}
	TClassField* GetField(TNameId name,bool only_in_this);
	TClassField* GetField(TNameId name,bool is_static,bool only_in_this);
	bool GetConstructors(TVector<TMethod*> &result);
	TMethod* GetDefConstr();
	TMethod* GetCopyConstr();
	TMethod* GetDestructor();
	TMethod* GetAutoDestructor(){
		return auto_destr.GetPointer();
	}
	bool GetOperators(TVector<TMethod*> &result,TOperator::Enum op);
	TMethod* GetBinOp(TOperator::Enum op,TClass* left,bool left_ref,TClass* right, bool right_ref);
	bool GetMethods(TVector<TMethod*> &result,TNameId use_method_name);
	bool GetMethods(TVector<TMethod*> &result,TNameId use_method_name,bool is_static);
	TMethod* GetConversion(bool source_ref,TClass* target_type);

	bool HasDefConstr();
	bool HasCopyConstr();
	bool HasDestr();
	bool HasConversion(TClass* target_type);
};

void ValidateAccess(TTokenPos* field_pos,TClass* source,TClassField* target);
void ValidateAccess(TTokenPos* field_pos,TClass* source,TMethod* target);

TMethod* FindMethod(TTokenPos* source,TVector<TMethod*> &methods_to_call,TVector<TFormalParam> &formal_params,int& conv_needed);

inline bool IsEqualClasses(TFormalParam formal_par,TClass* param_class,bool param_ref,int& need_conv);

class TSyntaxAnalyzer
{
public:
	TLexer lexer;
	TClass* base_class;
	TNotOptimizedProgram program;
	TTemplateRealizations templates;
public:
	TSyntaxAnalyzer():base_class(NULL){}
	~TSyntaxAnalyzer()
	{
		if(base_class!=NULL)
			delete base_class;
	}
	void Compile(char* use_source,TTime& time);
	void GetProgram(TProgram& use_program,TTime& time)
	{
		program.CreateOptimizedProgram(use_program,time);
	}
	int GetMethod(char* use_method);
	TClassField* GetStaticField(char* use_var);
};
