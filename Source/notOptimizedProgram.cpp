#include "baluScript.h"

TNotOptimizedProgram::~TNotOptimizedProgram()
{
	for(int i=0;i<=string_consts.GetHigh();i++)
		delete string_consts[i];
}

TNotOptimizedProgram::TNotOptimizedProgram()
{
	curr_label=0;
	static_vars_size=0;
}

int TNotOptimizedProgram::GetUniqueLabel()
{
	return curr_label++;
}
void TNotOptimizedProgram::Push(TOp use_op, TOpArray &ops_array)
{
	TListItem<TOp>* op=instr_alloc.New();
	op->value=use_op;
	if(ops_array.IsNull())
	{
		ops_array.first=op;
		ops_array.last=op;
	}else
	{
		ops_array.last->next=op;
		op->prev=ops_array.last;
		op->next=NULL;
		ops_array.last=op;
	}
	ListItems();
}
void TNotOptimizedProgram::PushFront(TOp use_op, TOpArray &ops_array)
{
	TListItem<TOp>* op=instr_alloc.New();
	op->value=use_op;
	if(ops_array.IsNull())
	{
		ops_array.first=op;
		ops_array.last=op;
	}else
	{
		ops_array.first->prev=op;
		op->next=ops_array.first;
		op->prev=NULL;
		ops_array.first=op;
	}
	ListItems();
}

void TNotOptimizedProgram::ListItems()
{
#ifdef _DEBUG
	ops.SetCount(0);
	for(int i=0;i<=methods_table.GetHigh();i++)
	{
		if(methods_table[i]->IsExternal())continue;
		TListItem<TOp>* c=methods_table[i]->GetOps().first;
		TListItem<TOp>* last;
		if(c==0)continue;//����� � ������ �����, �� ������������
		do{
			ops.Push(&c->value);
			last=c;
			c=c->next;
		}while(c!=NULL);
		assert(last==methods_table[i]->GetOps().last);
	}
#endif
}

void ConnectOps(TListItem<TOp>* &first_instr, TListItem<TOp>* &last_instr, const TOpArray& ops)
{
	assert(ops.first->prev==NULL&&ops.last->next==NULL);
	if(ops.IsNull())return;
	if(first_instr==NULL)
		first_instr=ops.first;
	else
		last_instr->next=ops.first;
	last_instr=ops.last;
}

struct TMethodPrePostEvents
{
	TListItem<TOp>* event_ops;
	int method_id;
	int first_op;
	bool is_pre_event;
};

void TNotOptimizedProgram::CreateOptimizedProgram(TProgram& optimized,TTime& time)
{
	unsigned long long t=time.GetTime();

	TVector<TMethodPrePostEvents> method_prepost_events;

	optimized.array_class_methods.SetHigh(array_element_classes.GetHigh());
	for(int i=0;i<=array_element_classes.GetHigh();i++)
	{
		TClass* arr_element=array_element_classes[i];
		TMethod* el_assign_op[2];
		bool el_has_assign_op=false;
		for(int t=0;t<2;t++)
		{
			el_assign_op[t]=arr_element->GetBinOp(TOperator::Assign,arr_element,true,arr_element,t);
			if(el_assign_op[t]!=NULL&&el_assign_op[t]->IsBytecode())
				el_assign_op[t]=NULL;
			el_has_assign_op=el_has_assign_op||(el_assign_op[t]!=NULL);
		}

		TMethod* el_equal_op[2][2];
		bool el_has_equal_op=false;
		for(int t=0;t<2;t++)
		{
			for(int k=0;k<2;k++)
			{
				el_equal_op[t][k]=arr_element->GetBinOp(TOperator::Equal,arr_element,t,arr_element,k);
				if(el_equal_op[t][k]!=NULL&&el_equal_op[t][k]->IsBytecode())
					el_equal_op[t][k]=NULL;
				el_has_equal_op=el_has_equal_op||(el_equal_op[t][k]!=NULL);
			}
		}

		TProgram::TArrayClassMethod temp;

		temp.el_size=arr_element->GetSize();
		temp.el_count=-1;//TODO
		temp.el_def_constr=AddMethodToTable(arr_element->GetDefConstr());
		temp.el_copy_constr=AddMethodToTable(arr_element->GetCopyConstr());
		temp.el_destr=AddMethodToTable(arr_element->GetDestructor());
		for(int t=0;t<2;t++)
			temp.el_assign_op[t]=AddMethodToTable(el_assign_op[t]);	
		for(int t=0;t<4;t++)
			temp.el_equal_op[0][t]=AddMethodToTable(el_equal_op[0][t]);	

		optimized.array_class_methods[i]=temp;
	}

	printf("Array elements creating = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();
	//
	ListItems();

	TSmartPointer<TMethod> t0(new TMethod((TClass*)NULL));
	methods_table.Push(t0.GetPointer());
	t0->SetAs(static_vars_init,NULL,false,true);
	t0->CalcParamSize();
	AddMethodToTable(t0.GetPointer());

	optimized.static_vars_init=methods_table.GetHigh();

	TSmartPointer<TMethod> t1(new TMethod((TClass*)NULL));
	methods_table.Push(t1.GetPointer());
	t1->SetAs(static_vars_destroy,NULL,false,true);
	t1->CalcParamSize();
	AddMethodToTable(t1.GetPointer());

	optimized.static_vars_destroy=methods_table.GetHigh();

	int methods_before_prepost_event=methods_table.GetCount();

	for(int i=0;i<=methods_table.GetHigh();i++)
	{
		AddMethodToTable(methods_table[i]->GetPreEvent());
		AddMethodToTable(methods_table[i]->GetPostEvent());
	}

	for(int i=methods_before_prepost_event;i<=methods_table.GetHigh();i++)
	{
		TMethod *pre=methods_table[i]->GetPreEvent();
		TMethod *post=methods_table[i]->GetPostEvent();

		//TODO � method_prepost_events ����� ��������� ���������� ������
		if(pre!=NULL)
		{
			TMethodPrePostEvents temp;
			temp.event_ops=pre->GetOps().first;
			temp.method_id=i;
			temp.first_op=-1;
			temp.is_pre_event=true;
			method_prepost_events.Push(temp);
			AddMethodToTable(pre);
		}
		if(post!=NULL)
		{
			TMethodPrePostEvents temp;
			temp.event_ops=post->GetOps().first;
			temp.method_id=i;
			temp.first_op=-1;
			temp.is_pre_event=false;
			method_prepost_events.Push(temp);
			AddMethodToTable(post);
		}
	}

	//������������ ��������
	//TODO
	//����� ������� �������������� ��������� � ����� ������� � ���� ������
	//����� ����� ���� ��������� ��������� ���� ������� �������� ��������������

	//TODO ����� �� ����� ���������� � �����
	for(int i=0;i<=methods_table.GetHigh();i++)
		if(methods_table[i]->GetOps().first!=NULL)
			assert(methods_table[i]->GetOps().first->value.type!=TOpcode::LABEL);
	//����������� ��� �������� ������� � ������ ����������� ������
	TListItem<TOp>* first_instr=NULL;
	TListItem<TOp>* last_instr=NULL;
	for(int i=0;i<=methods_table.GetHigh();i++)
	{
		TListItem<TOp>* c=methods_table[i]->GetOps().first;
		if(c==NULL)continue;//����� � ������ �����, �� ������������
		assert(c->value.type!=TOpcode::LABEL);//TODO ����� ������ �.�. ��������� ���������
		ConnectOps(first_instr,last_instr,methods_table[i]->GetOps());
	}

	printf("Ops list creating = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();
	//////////////
	int ops_count=0;
	{
		//��������� ����� � ���������� �� �������
		int* labels=curr_label>0?new int[curr_label]:NULL;
		//if(curr_label>0) �.�. ��� ���� ���������� ���������� ����������
		{
			//TODO � ������ ����� �� ����� ���������� � �����
			TListItem<TOp>* curr=first_instr;
			if(curr!=NULL){
				int i=0;
				do{
					while(curr->value.type==TOpcode::LABEL){
						assert(curr!=NULL);//��������� �� ����� ����������� ������, ����������� ����� RETURN
						labels[curr->value.v1]=i;
						(curr->next->prev=curr->prev)->next=curr->next;
						curr=curr->next;
					}
					i++;
				}while((curr=curr->next)!=NULL);
				ops_count=i;
			}
		}
		
		printf("Labels analyzing = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
		t=time.GetTime();

		optimized.instructions.SetCount(ops_count);
		optimized.methods_table.SetCount(methods_table.GetCount());
		TListItem<TOp> *curr=first_instr;
		if(curr!=NULL){
			int curr_method=0,curr_instr=0,curr_event=0;
			do{
				//����������� �������� ������������ ��������
				switch(curr->value.type)
				{
				case TOpcode::GOTO:
				case TOpcode::GOFALSE:
				case TOpcode::GOTRUE:
				case TOpcode::GLOBAL_TESTANDSET:
					curr->value.v1=labels[curr->value.v1];
				default:;
				}
				//����������� ��������
				optimized.instructions[curr_instr]=curr->value;
				//
				if(curr_method>=methods_before_prepost_event)
				{
					if(method_prepost_events[curr_event].event_ops==curr)
					{
						TProgram::TMethod& t=optimized.methods_table[curr_method];
						t.is_external=methods_table[curr_method]->IsExternal();
						t.is_static=methods_table[curr_method]->IsStatic();
						t.params_size=methods_table[curr_method]->GetParamsSize();
						t.pre_event=-1;
						t.post_event=-1;
						t.return_size=methods_table[curr_method]->GetRetSize();
						curr_event++;
						curr_method++;
						t.first_op=curr_instr;
						assert(!t.is_external);//����� ������ �� ������ ���� ��������
					}
				}
				else
				{
					while(methods_table[curr_method]->IsExternal())
					{
						TProgram::TMethod& t=optimized.methods_table[curr_method];
						t.is_external=methods_table[curr_method]->IsExternal();
						t.is_static=methods_table[curr_method]->IsStatic();
						t.params_size=methods_table[curr_method]->GetParamsSize();
						t.pre_event=-1;
						t.post_event=-1;
						t.return_size=methods_table[curr_method]->GetRetSize();
						curr_method++;
					}
					if(methods_table[curr_method]->GetOps().first==curr)
					{
						TProgram::TMethod& t=optimized.methods_table[curr_method];
						t.is_external=methods_table[curr_method]->IsExternal();
						t.is_static=methods_table[curr_method]->IsStatic();
						t.params_size=methods_table[curr_method]->GetParamsSize();
						t.pre_event=-1;
						t.post_event=-1;
						t.return_size=methods_table[curr_method]->GetRetSize();
						curr_method++;
						if(!t.is_external)
							t.first_op=curr_instr;
					}
				}
				curr_instr++;
			}while((curr=curr->next)!=NULL);
			assert(curr==NULL);//������ ����� �� ��������� ����������
			assert(curr_method==methods_table.GetCount());
		}
		if(labels!=NULL)delete[] labels;
	}

	printf("Ops generating = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();

	optimized.string_consts.SetCount(string_consts.GetCount());
	for(int i=0;i<=string_consts.GetHigh();i++)
	{
		optimized.string_consts[i]=string_consts[i];
	}
	optimized.static_vars_size=static_vars_size;
	printf("Strings copying = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();
}

int TNotOptimizedProgram::AddStringConst(TNameId string)
{
	//TODO
	/*for(int i=0;i<=string_consts.GetHigh();i++){
	if(strcmp(string_consts[i],string)==0)
	return i;
	}
	int len=strlen(string)+1;
	char* t=new char[len];
	strcpy_s(t,len,string);
	string_consts.Push(t);*/
	return string_consts.GetHigh();
}
int TNotOptimizedProgram::AddMethodToTable(TMethod* use_method)
{
	if(use_method==NULL)return -1;
	int i=FindMethod(use_method);
	if(i==-1)
	{
		methods_table.Push(use_method);
		return methods_table.GetHigh();
	}else return i;
}
int TNotOptimizedProgram::CreateArrayElementClassId(TClass* use_class)
{
	for(int i=0;i<=array_element_classes.GetHigh();i++)
		if(array_element_classes[i]==use_class)return i;
	array_element_classes.Push(use_class);
	return array_element_classes.GetHigh();
}
int TNotOptimizedProgram::FindMethod(TMethod* use_method)
{
	for(int i=0;i<=methods_table.GetHigh();i++)
		if(methods_table[i]==use_method)return i;
	return -1;
}
