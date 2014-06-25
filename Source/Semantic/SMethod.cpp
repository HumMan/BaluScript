#include "../Syntax/Method.h"

#include <assert.h>

#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"

TFormalParam TMethod::BuildCall(TNotOptimizedProgram &program, std::vector<TFormalParam> &formal_params)
{
	assert(this!=NULL);
	if(member_type!=TClassMember::Conversion)
		BuildFormalParamsConversion(program,formal_params);
	if(is_bytecode==1)
		return TFormalParam(ret.GetClass(),ret_ref,ops_array);
	else 
	{
		TOpArray ops_array;
		if(IsBytecode())
		{
			int not_used=-1;
			ops_array+=statements->Build(program,not_used).GetOps();
		}
		else
			program.Push(TOp(TOpcode::CALL_METHOD,program.AddMethodToTable(this)),ops_array);
		return TFormalParam(ret.GetClass(),ret_ref,ops_array);
	}
}

void TMethod::Declare()
{
	params_size=0;
	for(int i=0;i<param.size();i++)
	{
		param[i]->SetOffset(params_size);
		params_size+=param[i]->GetSize();
		param[i]->GetClass()->BuildClass();
	}
	if(ret.GetClass()!=NULL)
		ret.GetClass()->BuildClass();
	CheckForErrors();
}

void TMethod::Build(TNotOptimizedProgram &program)
{
	if(!(is_extern||IsBytecode()))
	{
		int local_var_offset=params_size;
		ops_array+=statements->Build(program,local_var_offset).GetOps();
		if(ret.GetClass()!=NULL)
		{
			if(!has_return)
				Error("����� ������ ���������� ��������!");
			//TODO �� �������� � �������� ����������� ������ �������
			//program.Push(TOp(TOpcode::METHOD_HAS_NOT_RETURN_A_VALUE),ops_array);
			program.Push(TOp(TOpcode::RETURN,
				GetParamsSize()+!IsStatic(),GetRetSize()),ops_array);
			//TODO �������� ���������� � ������
			//TODO ����� �������� �������� �� return, �� ��� ���� � ��������� �� �������� ������ ���������� ���������� �� �� �������� ��� ���
		}
		else{
			ops_array+=BuildParametersDestructor(program);
			program.Push(TOp(TOpcode::RETURN,params_size+!is_static,0),ops_array);
		}
	}
}

TOpArray TMethod::BuildParametersDestructor(TNotOptimizedProgram &program)
{
	TOpArray ops_array;
	for(int i=0;i<param.size();i++)
	{
		if(!param[i]->IsRef()&&param[i]->GetClass()->HasDestr())
		{
			program.Push(TOp(TOpcode::PUSH_LOCAL_REF,param[i]->GetOffset()),ops_array);
			ops_array+=param[i]->GetClass()->GetDestructor()->BuildCall(program).GetOps();
		}
	}
	return ops_array;
}

int TMethod::GetRetSize()
{
	if(ret.GetClass()==NULL)return 0;
	return ret_ref?1:ret.GetClass()->GetSize();
}


void TMethod::BuildFormalParamConversion(TNotOptimizedProgram &program,TFormalParam& formal_par,  TClass* param_class,bool param_ref)
{	
	assert(!(param_ref&&!formal_par.IsRef()));//������ � FindMethod
	//���� ���������� �������������� ���� ����������� ��������� �� ��������� ���
	if(formal_par.GetClass()!=param_class)
	{
		TMethod* conversion=formal_par.GetClass()->GetConversion(formal_par.IsRef(),param_class);
		if(formal_par.IsRef()&&!param_ref)
		{
			if(conversion==NULL)
			{
				TMethod* copy_constr=formal_par.GetClass()->GetCopyConstr();
				program.Push(TOp(TOpcode::RVALUE,formal_par.GetClass()->GetSize(),program.AddMethodToTable(copy_constr))
					,formal_par.GetOps());
				formal_par.SetIsRef(false);
				conversion=formal_par.GetClass()->GetConversion(false,param_class);
			}
		}
		assert(conversion!=NULL);//������ � FindMethod
		std::vector<TFormalParam> conv_method_params;
		conv_method_params.push_back(formal_par);
		formal_par=formal_par.GetOps()+conversion->BuildCall(program,conv_method_params);
	}
	//���� � ����� ��������� ������, � � �������� ��������� ��������� ��������, �� ��������� ��������������
	else if(formal_par.IsRef()&&!param_ref)
	{
		TMethod* copy_constr=formal_par.GetClass()->GetCopyConstr();
		program.Push(TOp(TOpcode::RVALUE,formal_par.GetClass()->GetSize(),program.AddMethodToTable(copy_constr))
			,formal_par.GetOps());
	}
}

void TMethod::BuildFormalParamsConversion(TNotOptimizedProgram &program, std::vector<TFormalParam> &formal_params)
{
	assert(formal_params.size()==param.size());//������ � FindMethod
	for(int i=0;i<param.size();i++){
		BuildFormalParamConversion(program,formal_params[i],param[i]->GetClass(),param[i]->IsRef());			
	}
}

TFormalParam TMethod::BuildCall(TNotOptimizedProgram &program,TClass* par0,bool par0_ref,TOpArray& par0_ops,TClass* par1,bool par1_ref,TOpArray& par1_ops)
{
	std::vector<TFormalParam> params;
	params.resize(2);
	params[0]=TFormalParam(par0,par0_ref,par0_ops);
	params[1]=TFormalParam(par1,par1_ref,par1_ops);
	return BuildCall(program,params);
}

TFormalParam TMethod::BuildCall(TNotOptimizedProgram &program,TClass* par0,bool par0_ref,TOpArray& par0_ops)
{
	std::vector<TFormalParam> params;
	params.resize(1);
	params[0]=TFormalParam(par0,par0_ref,par0_ops);
	return BuildCall(program,params);
}

TFormalParam TMethod::BuildCall(TNotOptimizedProgram &program)
{
	std::vector<TFormalParam> params;
	return BuildCall(program,params);
}

TVariable* TMethod::GetVar(TNameId name)
{
	for(int i=0;i<param.size();i++)
		if(param[i]->GetName()==name)return param[i].get();
	return owner->GetField(name,false);
}

void TMethod::CheckForErrors()
{
	if(owner->GetOwner()==NULL&&!is_static)
		Error("������� ����� ����� ��������� ������ ����������� ������!");
	for(int i=0;i<param.size();i++)
	{
		if(!param[i]->GetName().IsNull())
			for(int k=0;k<i;k++)
			{
				if(param[i]->GetName()==param[k]->GetName())
					param[i]->Error("�������� � ����� ������ ��� ����������!");
			}
	}
	if(!method_name.IsNull())
	{
		if(owner->GetClass(method_name)!=NULL)
			Error("����� �� ����� ���� ������ ������!");
		if(owner->GetField(method_name,false)!=NULL)
			Error("���� ������ � ����� ������ ��� ����������!");
		//TODO ��������� ����� ������������� ������ � �.�. (������ ������ �� ���������� ����)
	}
	switch(member_type)
	{
	case TResWord::Func:	
		assert(!method_name.IsNull());
		break;
	case TResWord::Constr:	
		if(is_static)Error("����������� ������ ���� �� ���������!");
		break;
	case TResWord::Destr:
		if(is_static)Error("���������� ������ ���� �� ���������!");
		break;
	case TResWord::Operator:
		if(!is_static)Error("�������� ������ ���� ���������!");
		break;
	case TResWord::Conversion:
		if(!is_static)Error("�������� ���������� ���� ������ ���� ���������!");
		break;
	default:assert(false);
	}
	{
		//��������� ������������ �������� ���������� � ������������� ��������
		switch(member_type)
		{
		case TClassMember::Func:	
			break;
		case TClassMember::Constr:	
			if(ret.GetClass()!=NULL)Error("����������� �� ������ ���������� ��������!");
			break;
		case TClassMember::Destr:	
			if(ret.GetClass()!=NULL)Error("���������� �� ������ ���������� ��������!");
			if(param.size()!=0)Error("���������� �� ����� ����������!");
			break;
		case TClassMember::Operator:
			if(operator_type==TOperator::Not)//������� ���������
			{
				if(GetParamsCount()!=1)
					Error("������� �������� ������ ����� ���� ��������!");
				if(GetParam(0)->GetClass()!=owner)
					Error("���� �� ���� �� ���������� ��������� ������ ���� ������� ��� �������� �� ������������!");
			}
			else if(operator_type==TOperator::UnaryMinus)
			{
				if (!IsIn(GetParamsCount(), 1, 2))
					Error("� �������� ��������� ""-"" ������� ���� 1 ��������!");
				if(GetParam(0)->GetClass()!=owner
					&& (GetParamsCount() == 2 && GetParam(1)->GetClass() != owner))
					Error("���������� �������� ��������� ������ ���� ����� ��� �������� �� ������������!");
			}
			else if(operator_type==TOperator::ParamsCall||operator_type==TOperator::GetArrayElement)
			{
				if (GetParamsCount()<2)
					Error("�������� ������ ���������� ������ ����� ������� 2 ��������!");
				if(GetParam(0)->GetClass()!=owner)
					Error("������ �������� ��������� ������ ������ ���� ������ �������!");
			}
			else //��������� �������� ���������
			{
				if((operator_type==TOperator::Equal||operator_type==TOperator::NotEqual)&&ret.GetClass()!=owner->GetClass(source->GetIdFromName("bool")))
					Error("�������� ��������� ������ ���������� ���������� ��������!");
				if (GetParamsCount() != 2)
					Error("� ��������� ��������� ������ ���� 2 ���������!");
				if(GetParam(0)->GetClass()!=owner
					&& (GetParamsCount() == 2 && GetParam(1)->GetClass() != owner))
					Error("���� �� ���� �� ���������� ��������� ������ ���� ������� ��� �������� �� ������������!");
			}
			break;
		case TClassMember::Conversion:
			if (GetParamsCount() != 1
				||GetParam(0)->GetClass()!=owner)
				Error("�������� ���������� ���� ������ ����� ���� �������� � ����� ������ ������ � ������� �� ���������!");
			break;
		default:assert(false);
		}
	}
}