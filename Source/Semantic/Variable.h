#pragma once

namespace TVariableType
{
	enum Enum
	{
		ClassField,
		Parameter,
		Local,
		This
	};
}

class TVariable
{
protected:
	TVariableType::Enum var_type;
public:
	TVariable(TVariableType::Enum use_var_type) :var_type(use_var_type){}
	TVariableType::Enum GetType()const{
		return var_type;
	}
};