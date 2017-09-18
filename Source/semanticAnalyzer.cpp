#include "semanticAnalyzer.h"

#include "lexer.h"
#include "Semantic/FormalParam.h"
#include "Semantic/SClassField.h"
#include "Syntax/ClassField.h"
#include "Syntax/Statements.h"
#include "Syntax/Method.h"
#include "Semantic/SClass.h"
#include "Semantic/SLocalVar.h"

//actual_parameter - параметр который был передан методу
//formal_parameter - параметр указаный в сигнатуре метода
bool IsEqualClasses(TExpressionResult actual_parameter, TFormalParameter formal_parameter, int& need_conv)
//============== На выходе =========================================
//результат - равенство классов или возможность преобразования (conversion) класса
{
	need_conv=0;
	if (actual_parameter.IsMethods() || actual_parameter.IsType())return false;
	if (formal_parameter.GetClass() != actual_parameter.GetClass())
	{
		if (!actual_parameter.GetClass()->HasConversion(formal_parameter.GetClass()))return false;
		//TODO
		//if (formal_parameter.IsRef() && !actual_parameter.GetClass()->IsNestedIn(formal_parameter.GetClass()))return false;
		need_conv+=1;	
	}
	if (actual_parameter.IsRef() && !formal_parameter.IsRef())need_conv += 1;
	return true;
}

TSMethod* FindMethod(Lexer::ITokenPos* source, std::vector<TSMethod*> &methods_to_call, const std::vector<TExpressionResult> &actual_params)
{
	int conv_needed;
	for (size_t k = 0; k<actual_params.size(); k++){
		if (actual_params[k].IsVoid())
			source->Error("Параметр метода должен иметь тип отличный от void!");
	}
	int min_conv_method=-1,temp_conv,conv;
	conv_needed=-1;
	for (size_t i = 0; i<methods_to_call.size(); i++)
	{			
		if (actual_params.size() == 0 && methods_to_call[i]->GetParamsCount() == 0){
			conv_needed=0;
			return methods_to_call[i];
		}
		if (actual_params.size() != methods_to_call[i]->GetParamsCount())goto end_search;
		temp_conv=0;
		conv=0;
		for (size_t k = 0; k<actual_params.size(); k++){
			TSParameter* p=methods_to_call[i]->GetParam(k);
			if (!IsEqualClasses(actual_params[k], p->AsFormalParameter(), conv))goto end_search;
			else temp_conv+=conv;
		}
		if(temp_conv<conv_needed||conv_needed==-1)
		{
			conv_needed=temp_conv;
			min_conv_method=i;
		}end_search:
		continue;
	}
	if(min_conv_method>=0)
		return methods_to_call[min_conv_method];		
	return nullptr;
}


void ValidateAccess(Lexer::ITokenPos* field_pos, TSClass* source, TSClassField* target)
{
	if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Public)return;
	if (source == target->GetOwner())return;
	if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Protected&&!source->IsNestedIn(target->GetOwner()))
		field_pos->Error("Данное поле класса доступно только из классов наследников (protected)!");
	else if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Private&&source != target->GetOwner())
		field_pos->Error("Данное поле класса доступно только из класса в котором оно объявлено (private)!");
}

void ValidateAccess(Lexer::ITokenPos* field_pos, TSClass* source, TSMethod* target)
{
	if (target->GetType() != TSpecialClassMethod::NotSpecial)return;
	if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Public)return;
	if (source == target->GetOwner())return;
	if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Protected&&!source->IsNestedIn(target->GetOwner()))
		field_pos->Error("Данный метод доступен только из классов наследников (protected)!");
	else if (target->GetSyntax()->GetAccess() == SyntaxApi::TTypeOfAccess::Private&&source != target->GetOwner())
		field_pos->Error("Данный метод доступен только из класса в котором он объявлен (private)!");
}


void InitializeStaticClassFields(std::vector<TSClassField*> static_fields, std::vector<TStaticValue> &static_objects)
{
	for (TSClassField* v : static_fields)
	{
		v->SetOffset(static_objects.size());
		static_objects.emplace_back(false,v->GetClass());
		TSMethod* def_constr = v->GetClass()->GetDefConstr();
		static_objects[v->GetOffset()].Initialize();
		if (def_constr != nullptr)
		{
			std::vector<TStackValue> constr_formal_params;
			TStackValue without_result, var_object(true, v->GetClass());
			var_object.SetAsReference(static_objects[v->GetOffset()].get());
			def_constr->Run(TMethodRunContext(&static_objects, &constr_formal_params, &without_result, &var_object));
		}
		
	}
}
void InitializeStaticVariables(std::vector<TSLocalVar*> static_variables, std::vector<TStaticValue> &static_objects)
{
	for (TSLocalVar* v : static_variables)
	{
		v->SetOffset(static_objects.size());
		static_objects.emplace_back(false, v->GetClass());
	}
}