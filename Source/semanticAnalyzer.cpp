#include "syntaxAnalyzer.h"

#include "Syntax/ClassField.h"
#include "Syntax/Class.h"
#include "Syntax/Method.h"
#include "Syntax/Statements.h"

bool IsEqualClasses(TFormalParam formal_par,TClass* param_class,bool param_ref,int& need_conv)
//============== На выходе =========================================
//результат - равенство классов или возможность приведения класса
{
	need_conv=0;
	if((!formal_par.IsRef())&&param_ref)return false;		
	if(formal_par.IsMethods()||formal_par.IsType())return false;
	if(param_class!=formal_par.GetClass())
	{
		if(!formal_par.GetClass()->HasConversion(param_class))return false;
		if(param_ref&&!formal_par.GetClass()->IsChildOf(param_class))return false;
		need_conv+=1;	
	}
	if(formal_par.IsRef()&&!param_ref)need_conv+=1;
	return true;
}

TMethod* FindMethod(TTokenPos* source, std::vector<TMethod*> &methods_to_call,const std::vector<TFormalParam> &formal_params, int& conv_needed)
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
			TParameter* p=methods_to_call[i]->GetParam(k);
			if(!IsEqualClasses(formal_params[k],p->GetClass(),p->IsRef(),conv))goto end_search;
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

int TSyntaxAnalyzer::GetMethod(char* use_method)
{
	lexer.ParseSource(use_method);
	TMethod* method_decl=new TMethod(base_class);
	method_decl->AnalyzeSyntax(lexer,false);
	method_decl->Declare();
	std::vector<TMethod*> methods;
	TMethod* method=NULL;
	switch(method_decl->GetMemberType())
	{
	case TClassMember::Func:	
		method_decl->GetOwner()->GetMethods(methods,method_decl->GetName());
		break;
	case TClassMember::Constr:	
		method_decl->GetOwner()->GetConstructors(methods);
		break;
	case TClassMember::Destr:	
		method=method_decl->GetOwner()->GetDestructor();
		break;
	case TClassMember::Operator:
		method_decl->GetOwner()->GetOperators(methods,method_decl->GetOperatorType());
		break;
	case TClassMember::Conversion:
		method=method_decl->GetOwner()->GetConversion(method_decl->GetParam(0)->IsRef(),method_decl->GetRetClass());
		break;
	default:assert(false);
	}
	TClassMember::Enum temp=method_decl->GetMemberType();
	if(temp==TClassMember::Func||
			temp==TClassMember::Constr||
			temp==TClassMember::Operator)
	{
		for (int i=0;i<methods.size();i++)
		{
			if(method_decl->GetParamsCount()==0&&methods[i]->GetParamsCount()==0){
				method=methods[i];
				break;
			}
			if (method_decl->GetParamsCount() != methods[i]->GetParamsCount())continue;
			for (int k = 0; k < method_decl->GetParamsCount(); k++)
			{
				if(!methods[i]->GetParam(k)->IsEqualTo(*method_decl->GetParam(k)))
					goto end_search;
			}
			method=methods[i];
			i=methods.size();end_search:continue;
		}
	}

	if(method!=NULL)
	{
		if(method_decl->IsStatic()!=method->IsStatic())
			lexer.Error("Метод отличается по статичности!");
		if(method_decl->IsExternal()!=method->IsExternal())
			lexer.Error("Несоответствует классификатор extern!");
		if(method_decl->GetRetClass()!=method->GetRetClass()
			||method_decl->IsReturnRef()!=method->IsReturnRef())
			lexer.Error("Метод возвращает другое значение!");
		delete method_decl;
		return program.AddMethodToTable(method);
	}
	else
		lexer.Error("Такого метода не существует!");
	delete method_decl;
	return -1;
}

TClassField* TSyntaxAnalyzer::GetStaticField(char* use_var)
{
	lexer.ParseSource(use_var);
	if(lexer.NameId()!=base_class->GetName())
		lexer.Error("Ожидалось имя класса!");
	lexer.GetToken();
	TClassField* result=NULL;
	TClass* curr_class=base_class;
	while(lexer.Test(TTokenType::Dot))
	{
		lexer.GetToken(TTokenType::Dot);
		lexer.TestToken(TTokenType::Identifier);
		TClass* t=curr_class->GetNested(lexer.NameId());
		if(t==NULL)
		{
			result=curr_class->GetField(lexer.NameId(),true);
		}else curr_class=t;
	}
	if(result==NULL)lexer.Error("Статического члена класса с таким именем не существует!");
	//if(!result->IsStatic())lexer->Error("Член класса с таким именем не является статическим!");
	return result;
}
