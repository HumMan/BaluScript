#include "SCommon.h"

#include "../../lexer.h"

#include "SClassField.h"
#include "SClass.h"
#include "SLocalVar.h"

#include "SExpression.h"

//actual_parameter - параметр который был передан методу
//formal_parameter - параметр указаный в сигнатуре метода
bool IsEqualClasses(TExpressionResult actual_parameter, SemanticApi::TFormalParameter formal_parameter, int& need_conv)
//============== На выходе =========================================
//результат - равенство классов или возможность преобразования (conversion) класса
{
	need_conv = 0;
	if (actual_parameter.IsMethods() || actual_parameter.IsType())return false;
	if (formal_parameter.GetClass() != actual_parameter.GetClass())
	{
		if (!dynamic_cast<TSClass*>(actual_parameter.GetClass())->HasConversion(formal_parameter.GetClass()))
			return false;
		//TODO
		//if (formal_parameter.IsRef() && !actual_parameter.GetClass()->IsNestedIn(formal_parameter.GetClass()))
		need_conv += 1;
	}
	if (actual_parameter.IsRef() && !formal_parameter.IsRef())need_conv += 1;
	return true;
}

SemanticApi::ISMethod* FindMethod(Lexer::ITokenPos* source, std::vector<SemanticApi::ISMethod*> &methods_to_call, const std::vector<TExpressionResult> &actual_params)
{
	int conv_needed;
	for (size_t k = 0; k < actual_params.size(); k++) {
		if (actual_params[k].IsVoid())
			source->Error("Параметр метода должен иметь тип отличный от void!");
	}
	int min_conv_method = -1, temp_conv, conv;
	conv_needed = -1;
	for (size_t i = 0; i < methods_to_call.size(); i++)
	{
		if (actual_params.size() == 0 && methods_to_call[i]->GetParamsCount() == 0) {
			conv_needed = 0;
			return methods_to_call[i];
		}
		if (actual_params.size() != methods_to_call[i]->GetParamsCount())goto end_search;
		temp_conv = 0;
		conv = 0;
		for (size_t k = 0; k < actual_params.size(); k++) {
			SemanticApi::ISParameter* p = methods_to_call[i]->GetParam(k);
			if (!IsEqualClasses(actual_params[k], p->AsFormalParameter(), conv))goto end_search;
			else temp_conv += conv;
		}
		if (temp_conv < conv_needed || conv_needed == -1)
		{
			conv_needed = temp_conv;
			min_conv_method = i;
		}end_search:
		continue;
	}
	if (min_conv_method >= 0)
		return methods_to_call[min_conv_method];
	return nullptr;
}


void ValidateAccess(Lexer::ITokenPos* field_pos, SemanticApi::ISClass* source, TSClassField* target)
{
	if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Public)return;
	if (source == target->GetOwner())return;
	if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Protected && !dynamic_cast<TSClass*>(source)->IsNestedIn(dynamic_cast<TSClass*>(target->GetOwner())))
		field_pos->Error("Данное поле класса доступно только из классов наследников (protected)!");
	else if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Private&&source != target->GetOwner())
		field_pos->Error("Данное поле класса доступно только из класса в котором оно объявлено (private)!");
}

void ValidateAccess(Lexer::ITokenPos* field_pos, SemanticApi::ISClass* source, SemanticApi::ISMethod* _target)
{
	auto target = dynamic_cast<TSMethod*>(_target);
	if (target->GetType() != SemanticApi::SpecialClassMethodType::NotSpecial)return;
	if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Public)return;
	if (source == target->GetOwner())return;
	if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Protected && !dynamic_cast<TSClass*>(source)->IsNestedIn(target->GetOwner()))
		field_pos->Error("Данный метод доступен только из классов наследников (protected)!");
	else if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Private&&source != target->GetOwner())
		field_pos->Error("Данный метод доступен только из класса в котором он объявлен (private)!");
}

SemanticApi::ISMethod* FindBinaryOperator(TSOperation *left, TSOperation *right,
	SyntaxApi::IExpression* syntax_node, TSClass* owner, Lexer::TOperator operator_type)
{
	std::vector<TSOperation*> param_expressions;
	param_expressions.push_back(left);
	param_expressions.push_back(right);

	std::vector<TExpressionResult> param;
	std::vector<SyntaxApi::IMethod*> bin_operators;

	param.resize(2);
	param[0] = left->GetFormalParameter();
	param[1] = right->GetFormalParameter();

	if (param[0].IsVoid())
		syntax_node->Error("К левому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");
	if (param[1].IsVoid())
		syntax_node->Error("К правому операнду нельзя применить бинарный оператор (нужен тип отличающийся от void)!");

	std::vector<SemanticApi::ISMethod*> operators;

	//список доступных операторов получаем из левого операнда
	param[0].GetClass()->GetOperators(operators, operator_type);
	auto bin_operator = FindMethod(syntax_node, operators, param);
	if (operator_type >= Lexer::TOperator::Assign &&
		operator_type <= Lexer::TOperator::OrA &&
		!param[0].IsRef())
		syntax_node->Error("Для присваиваниия требуется ссылка, а не значение!");

	if (bin_operator != nullptr)
	{
		ValidateAccess(syntax_node, owner, bin_operator);
	}
	else syntax_node->Error("Бинарного оператора для данных типов не существует!");

	return bin_operator;
}

void InitializeStaticClassFieldsOffset(std::vector<TSClassField*> static_fields)
{
	int curr_offset = 0;
	for (TSClassField* v : static_fields)
	{
		v->SetOffset(curr_offset);
		curr_offset++;
	}
}
void InitializeStaticVariablesOffset(std::vector<TSLocalVar*> static_variables)
{
	int curr_offset = 0;
	for (TSLocalVar* v : static_variables)
	{
		v->SetOffset(curr_offset);
		curr_offset++;
	}
}
