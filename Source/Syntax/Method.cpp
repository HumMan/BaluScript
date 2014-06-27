#include "Method.h"

#include "Statements.h"

void TMethod::ParametersDecl(TLexer& source) {
	//—читываем все параметры метода и возвращаемое значение
	if (source.TestAndGet(TTokenType::Operator, TOperator::ParamsCall)) {
	} else if (source.Test(TTokenType::LParenth)) {
		source.GetToken(TTokenType::LParenth);
		while (source.Test(TTokenType::Identifier)) {
			TParameter* t = new TParameter(owner, this);
			param.push_back(std::unique_ptr<TParameter>(t));
			t->AnalyzeSyntax(source);
			if (source.Test(TTokenType::Comma))
				source.GetToken();
			else
				break;
		}
		source.GetToken(TTokenType::RParenth);
	}
	if (source.TestAndGet(TTokenType::Colon)) {
		ret_ref = source.TestAndGet(TTokenType::Ampersand);//TODO проверка ссылки после типа а то не пон¤тно почему ошибка
		ret.AnalyzeSyntax(source);
	}
}

void TMethod::AnalyzeSyntax(TLexer& source, bool realization) {
	InitPos(source);
	source.TestToken(TTokenType::ResWord);
	member_type = (TClassMember::Enum) source.Token();
	if (!IsIn(member_type, TClassMember::Func, TClassMember::Conversion))
		source.Error(
				"ќжидалось объ¤вление метода,конструктора,деструктора,оператора или приведени¤ типа!");
	source.GetToken();
	is_extern = source.TestAndGet(TTokenType::ResWord, TResWord::Extern);
	is_static = source.TestAndGet(TTokenType::ResWord, TResWord::Static);
	switch (member_type) {
	case TResWord::Func:
	case TResWord::Constr:
	case TResWord::Destr:
	case TResWord::Operator:
	case TResWord::Conversion:
		break;
	default:
		source.Error(
				"ќжидалось объ¤вление метода,конструктора,деструктора,оператора или приведени¤ типа!");
	}
	//
	if (!realization) {
		if (source.NameId() != owner->GetName())
			Error("ќжидалось им¤ базового класса!");
		source.GetToken();
		while (source.Test(TTokenType::Dot)) {
			source.GetToken(TTokenType::Dot);
			if (!source.Test(TTokenType::Identifier))
				break;
			TClass* t = owner->GetNested(source.NameId());
			if (t != NULL) {
				source.GetToken();
				owner = t;
			} else
				break;
		}
	}
	//
	if (member_type == TClassMember::Func) {
		source.TestToken(TTokenType::Identifier);
		method_name = source.NameId();
		source.GetToken();
	} else if (member_type == TClassMember::Operator) {
		source.TestToken(TTokenType::Operator);
		operator_type = (TOperator::Enum) source.Token();
		source.GetToken();
	}
	{
		ParametersDecl(source);
		if (realization) {
			switch (member_type) {
			case TClassMember::Func:
				owner->AddMethod(this, method_name);
				break;
			case TClassMember::Constr:
				owner->AddConstr(this);
				break;
			case TClassMember::Destr:
				owner->AddDestr(this);
				break;
			case TClassMember::Operator:
				if (operator_type == TOperator::Minus || operator_type
						== TOperator::UnaryMinus) {
					if (GetParamsCount() == 1)
						operator_type = TOperator::UnaryMinus;
					else
						operator_type = TOperator::Minus;
				}
				owner->AddOperator(operator_type, this);
				break;
			case TClassMember::Conversion:
				owner->AddConversion(this);
				break;
			default:
			assert(false);
			}
		}
	}
	if (is_extern && realization) {
		if (!source.Test(TTokenType::Semicolon))
			source.Error("ƒл¤ внешнего метода тело должно отсутствовать!");
		source.GetToken(TTokenType::Semicolon);
	} else if (realization) {
		one_instruction = !source.Test(TTokenType::LBrace);
		statements->AnalyzeSyntax(source);
	}
}

void TMethod::InitOwner(TClass* use_owner)
{
	owner = use_owner;
	ret.InitOwner(use_owner);
	for (int i = 0; i<param.size(); i++)
		param[i]->InitOwner(use_owner, this);
	statements->InitOwner(use_owner, this, NULL);
}
void TMethod::SetPreEvent(TMethod* use_event)

{
	//assert(!use_event->IsBytecode());
	pre_event = use_event;//TODO проверить на правильность метода
}
void TMethod::SetPostEvent(TMethod* use_event)
//используетс¤ дл¤ вызова автом. дестр. дл¤ пользов/внешнего метода
{
	post_event = use_event;//TODO проверить на правильность метода
}
void TMethod::SetAs(TOpArray use_ops, TClass* use_ret_class_pointer, bool use_ret_ref, bool use_is_static, int use_is_bytecode)
{
	is_bytecode = use_is_bytecode;
	ops_array = use_ops;
	ret.SetAs(use_ret_class_pointer);
	ret_ref = use_ret_ref;
	is_static = use_is_static;
	is_extern = false;
	build = true;
}
TMethod* TMethod::GetPreEvent(){
	return pre_event;
}
TMethod* TMethod::GetPostEvent(){
	return post_event;
}
TMethod::TMethod(TClass* use_owner, TClassMember::Enum use_member_type)
	:ret(use_owner), ret_ref(false), owner(use_owner)
	, is_static(false), is_extern(false)
	, params_size(-1)
	, statements(new TStatements(use_owner, this, NULL, -1))
	, has_return(false)
	, declared(false), build(false)
	, one_instruction(false)
	, operator_type(TOperator::End)
	, member_type(use_member_type)
	, pre_event(NULL), post_event(NULL)
	, is_bytecode(0)
{
}

void TMethod::SetHasReturn(bool use_has_return){
	has_return = use_has_return;
}
TNameId TMethod::GetName(){
	return method_name;
}
TClass* TMethod::GetOwner()const{
	return owner;
}
TOperator::Enum TMethod::GetOperatorType(){
	assert(member_type == TClassMember::Operator);
	return operator_type;
}
TClassMember::Enum TMethod::GetMemberType()const{
	return member_type;
}
TOpArray TMethod::GetOps()const{
	return ops_array;
}
TClass* TMethod::GetRetClass()
{
	return ret.GetClass();
}
int TMethod::GetParamsSize(){
	assert(params_size != -1);
	return params_size;
}
TParameter* TMethod::GetParam(int use_id)
{
	return param[use_id].get();
}
int TMethod::GetParamsCount(){
	return param.size();
}
bool TMethod::IsReturnRef()const
{
	return ret_ref;
}
bool TMethod::IsStatic(){
	return is_static;
}
bool TMethod::IsExternal(){
	return is_extern;
}
bool TMethod::IsBytecode()
{
	if (is_bytecode == 0)
		return one_instruction
		&&statements->GetHigh() == 0
		&& statements->GetStatement(0)->GetType() == TStatementType::Bytecode;
	else return is_bytecode;
}
void TMethod::AddParam(TParameter* use_param)
{
	param.push_back(std::unique_ptr<TParameter>(use_param));
}
void TMethod::CalcParamSize()
{
	params_size = 0;
	for (int i = 0; i<param.size(); i++)
		params_size += param[i]->GetSize();
}
bool TMethod::HasParams(std::vector<std::unique_ptr<TParameter>> &use_params)const
{
	if (use_params.size() != param.size())
		return false;
	for (int i = 0; i<param.size(); i++)
		if (!param[i]->IsEqualTo(*(use_params[i])))
			return false;
	return true;
}
