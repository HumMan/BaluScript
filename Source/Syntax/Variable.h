namespace TVariableType
{
	enum Enum
	{
		ClassField,
		Parameter,
		LocalVar
	};
}

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