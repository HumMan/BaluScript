#include "semanticAnalyzer.h"

#include "lexer.h"
#include "Semantic/FormalParam.h"
#include "Semantic/SClassField.h"
#include "Syntax/ClassField.h"
#include "Syntax/Statements.h"
#include "Semantic/SClass.h"
#include "Semantic/SLocalVar.h"

bool IsEqualClasses(TFormalParam formal_par,TSClass* param_class,bool param_ref,int& need_conv)
//============== На выходе =========================================
//результат - равенство классов или возможность приведения класса
{
	need_conv=0;
	if((!formal_par.IsRef())&&param_ref)return false;		
	if(formal_par.IsMethods()||formal_par.IsType())return false;
	if(param_class!=formal_par.GetClass())
	{
		if(!formal_par.GetClass()->HasConversion(param_class))return false;
		if(param_ref&&!formal_par.GetClass()->IsNestedIn(param_class))return false;
		need_conv+=1;	
	}
	if(formal_par.IsRef()&&!param_ref)need_conv+=1;
	return true;
}

TSMethod* FindMethod(TTokenPos* source, std::vector<TSMethod*> &methods_to_call,const std::vector<TFormalParam> &formal_params, int& conv_needed)
{
	for(int k=0;k<formal_params.size();k++){
		if(formal_params[k].IsVoid())
			source->Error("Параметр метода должен иметь тип отличный от void!");
	}
	int i,k;
	int min_conv_method=-1,temp_conv,conv;
	conv_needed=-1;
	for (i=0;i<methods_to_call.size();i++)
	{			
		if(formal_params.size()==0&&methods_to_call[i]->GetParamsCount()==0){
			conv_needed=0;
			return methods_to_call[i];
		}
		if(formal_params.size()!=methods_to_call[i]->GetParamsCount())goto end_search;
		temp_conv=0;
		conv=0;
		for(k=0;k<formal_params.size();k++){
			TSParameter* p=methods_to_call[i]->GetParam(k);
			if (!IsEqualClasses(formal_params[k], p->GetClass(), p->IsRef(), conv))goto end_search;
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
	return NULL;
}


void ValidateAccess(TTokenPos* field_pos, TSClass* source, TSClassField* target)
{
	if (target->GetSyntax()->GetAccess() == TTypeOfAccess::Public)return;
	if (source == target->GetOwner())return;
	if (target->GetSyntax()->GetAccess() == TTypeOfAccess::Protected&&!source->IsNestedIn(target->GetOwner()))
		field_pos->Error("Данное поле класса доступно только из классов наследников (protected)!");
	else if (target->GetSyntax()->GetAccess() == TTypeOfAccess::Private&&source != target->GetOwner())
		field_pos->Error("Данное поле класса доступно только из класса в котором оно объявлено (private)!");
}

void ValidateAccess(TTokenPos* field_pos, TSClass* source, TSMethod* target)
{
	if (target->GetType() != TSpecialClassMethod::NotSpecial)return;
	if (target->GetSyntax()->GetAccess() == TTypeOfAccess::Public)return;
	if (source == target->GetOwner())return;
	if (target->GetSyntax()->GetAccess() == TTypeOfAccess::Protected&&!source->IsNestedIn(target->GetOwner()))
		field_pos->Error("Данный метод доступен только из классов наследников (protected)!");
	else if (target->GetSyntax()->GetAccess() == TTypeOfAccess::Private&&source != target->GetOwner())
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
		if (def_constr != NULL)
		{
			std::vector<TStackValue> constr_formal_params;
			TStackValue without_result, var_object(true, v->GetClass());
			var_object.SetAsReference(static_objects[v->GetOffset()].get());
			def_constr->Run(static_objects, constr_formal_params, without_result, var_object);
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