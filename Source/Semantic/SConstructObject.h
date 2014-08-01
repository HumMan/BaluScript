#pragma once

#include "SType.h"
#include "SStatement.h"
#include "Variable.h"

class TSExpression;
class TFormalParam;
class TSExpression_TMethodCall;
class TOperation;

class TSConstructObject
{
	std::vector<std::unique_ptr<TOperation>> params;
	std::unique_ptr<TSExpression_TMethodCall> constructor_call;
	TSClass* object_type;
	TSMethod* method;
	TSStatements* parent;
	TSClass* owner;

	bool constr_copy_memcpy;
public:
	TSConstructObject(TSClass* use_owner, TSMethod* use_method, TSStatements* use_parent, TSClass* object_type);
	TFormalParam Build(TSLocalVar* local_var, TTokenPos* source, std::vector<std::unique_ptr<TExpression>>& params, std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void Construct(TStackValue& constructed_object, std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, bool& result_returned, TStackValue& result, TStackValue& object, std::vector<TStackValue>& local_variables);
	void Destruct(TStackValue& destroyed_object, std::vector<TStaticValue> &static_fields, std::vector<TStackValue>& local_variables);
};