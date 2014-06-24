
namespace TTypeOfAccess
{
	enum Enum
	{
		Private,
		Protected,
		Public
	};
}

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