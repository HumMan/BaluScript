
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