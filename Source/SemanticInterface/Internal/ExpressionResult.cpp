#include <string.h>

#include "ExpressionResult.h"

#include "SClass.h"
#include "SMethod.h"
#include "SStatements.h"

void TExpressionResult::Init()
{
	result_value_type = nullptr;
	is_ref = false;
	type = nullptr;
}
TExpressionResult::TExpressionResult()
{
	Init();
}
TExpressionResult::TExpressionResult(std::vector<SemanticApi::ISMethod*> use_methods, bool use_need_push_this)
{
	Init();
	methods = use_methods;
}
TExpressionResult::TExpressionResult(TSClass* use_class, bool use_is_ref)
{
	Init();
	result_value_type = use_class;
	is_ref = use_is_ref;
}
TExpressionResult::TExpressionResult(TSClass* use_type)
{
	Init();
	type = use_type;
}
bool TExpressionResult::IsRef()const
{
	return is_ref;
}
void TExpressionResult::SetIsRef(bool use_is_ref)
{
	is_ref = use_is_ref;
}
bool TExpressionResult::IsMethods()const
{
	return methods.size() != 0;
}
bool TExpressionResult::IsType()const
{
	return type != nullptr;
}
SemanticApi::ISClass* TExpressionResult::GetType()const
{
	return type;
}
std::vector<SemanticApi::ISMethod*>& TExpressionResult::GetMethods()
{
	return methods;
}
SemanticApi::ISClass* TExpressionResult::GetClass()const
{
	return result_value_type;
}
bool TExpressionResult::IsVoid()const
{
	return result_value_type == nullptr && (!IsMethods()) && type == nullptr;//TODO в дальнейшем methods_pointer не должен считаться void
}