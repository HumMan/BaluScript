#pragma once

#include <vector>
#include <assert.h>

#include <common.h>

#include "../SemanticTreeApi.h"

class TSClass;
class TSMethod;


class TExpressionResult
{
	//если возвращается значение - то его тип и ссылочность
	TSClass* result_value_type;
	bool is_ref;

	TSClass* type;//указатель на тип (для доступа к статическим членам и функциям)

	std::vector<SemanticApi::ISMethod*> methods;//указатель на перегруженые методы - все доступные из области видимости (если выражение является методом)
	void Init();
public:
	TExpressionResult();
	TExpressionResult(std::vector<SemanticApi::ISMethod*> use_methods, bool use_need_push_this);
	TExpressionResult(TSClass* use_class, bool use_is_ref);
	TExpressionResult(TSClass* use_type);
	bool IsRef()const;
	void SetIsRef(bool use_is_ref);
	bool IsMethods()const;
	bool IsType()const;
	TSClass* GetType()const;
	std::vector<SemanticApi::ISMethod*>& GetMethods();
	TSClass* GetClass()const;
	bool IsVoid()const;
};

