#include "notOptimizedProgram.h"

#include "Syntax/Method.h"
#include "Syntax/Statements.h"

TNotOptimizedProgram::~TNotOptimizedProgram()
{
	for(int i=0;i<string_consts.size();i++)
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
	ops.resize(0);
	for(int i=0;i<methods_table.size();i++)
	{
		if(methods_table[i]->IsExternal())continue;
		TListItem<TOp>* c=methods_table[i]->GetOps().first;
		TListItem<TOp>* last;
		if(c==0)continue;//метод с пустым телом, но используемый
		do{
			ops.push_back(&c->value);
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

/// <summary> 
/// Используется при задании дополнительных операций в начале или в конце метода - например, 
/// вызов конструктором членов класса перед телом конструктора и аналогично для деструктора
/// </summary>
struct TMethodPrePostEvents
{
	TListItem<TOp>* event_ops;
	int method_id;
	int first_op;
	
	bool is_pre_event;
};

int TNotOptimizedProgram::CreateStaticVarsInitMethod()
{
	static_vars_init_method = std::unique_ptr<TMethod>(new TMethod((TClass*)NULL));
	methods_table.push_back(static_vars_init_method.get());
	static_vars_init_method->SetAs(static_vars_init, NULL, false, true);
	static_vars_init_method->CalcParamSize();
	AddMethodToTable(static_vars_init_method.get());
	return methods_table.size() - 1;
}
int TNotOptimizedProgram::CreateStaticVarsDestroyMethod()
{
	static_vars_destroy_method = std::unique_ptr<TMethod>(new TMethod((TClass*)NULL));
	methods_table.push_back(static_vars_destroy_method.get());
	static_vars_destroy_method->SetAs(static_vars_destroy, NULL, false, true);
	static_vars_destroy_method->CalcParamSize();
	AddMethodToTable(static_vars_destroy_method.get());
	return methods_table.size() - 1;
}

void TNotOptimizedProgram::InitArrayClassMethods(TProgram& optimized)
{
	optimized.array_class_methods.resize(array_element_classes.size());
	for (int i = 0; i<array_element_classes.size(); i++)
	{
		TClass* arr_element = array_element_classes[i];
		TMethod* el_assign_op[2];
		bool el_has_assign_op = false;
		for (int t = 0; t<2; t++)
		{
			el_assign_op[t] = arr_element->GetBinOp(TOperator::Assign, arr_element, true, arr_element, t);
			if (el_assign_op[t] != NULL&&el_assign_op[t]->IsBytecode())
				el_assign_op[t] = NULL;
			el_has_assign_op = el_has_assign_op || (el_assign_op[t] != NULL);
		}

		TMethod* el_equal_op[2][2];
		bool el_has_equal_op = false;
		for (int t = 0; t<2; t++)
		{
			for (int k = 0; k<2; k++)
			{
				el_equal_op[t][k] = arr_element->GetBinOp(TOperator::Equal, arr_element, t, arr_element, k);
				if (el_equal_op[t][k] != NULL&&el_equal_op[t][k]->IsBytecode())
					el_equal_op[t][k] = NULL;
				el_has_equal_op = el_has_equal_op || (el_equal_op[t][k] != NULL);
			}
		}

		TArrayClassMethod temp;

		temp.el_size = arr_element->GetSize();
		temp.el_count = -1;//TODO
		temp.el_def_constr = AddMethodToTable(arr_element->GetDefConstr());
		temp.el_copy_constr = AddMethodToTable(arr_element->GetCopyConstr());
		temp.el_destr = AddMethodToTable(arr_element->GetDestructor());
		for (int t = 0; t<2; t++)
			temp.el_assign_op[t] = AddMethodToTable(el_assign_op[t]);
		for (int t = 0; t<4; t++)
			temp.el_equal_op[0][t] = AddMethodToTable(el_equal_op[0][t]);

		optimized.array_class_methods[i] = temp;
	}
}

void TNotOptimizedProgram::InitPrePostEvents(std::vector<TMethodPrePostEvents>& method_prepost_events, int methods_before_prepost_event)
{
	for (int i = 0; i<methods_table.size(); i++)
	{
		if (AddMethodToTable(methods_table[i]->GetPreEvent()) != -1)
		{
			TMethodPrePostEvents temp;
			temp.event_ops = methods_table[i]->GetPreEvent()->GetOps().first;
			temp.method_id = i;
			temp.first_op = -1;
			temp.is_pre_event = true;
			method_prepost_events.push_back(temp);
		}

		if (AddMethodToTable(methods_table[i]->GetPostEvent()) != -1)
		{
			TMethodPrePostEvents temp;
			temp.event_ops = methods_table[i]->GetPostEvent()->GetOps().first;
			temp.method_id = i;
			temp.first_op = -1;
			temp.is_pre_event = true;
			method_prepost_events.push_back(temp);
		}
	}
}

void TNotOptimizedProgram::CreateOptimizedProgram(TProgram& optimized,TTime& time)
{
	unsigned long long t=time.GetTime();

	

	InitArrayClassMethods(optimized);

	printf("Array elements creating = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();
	//
	ListItems();

	optimized.static_vars_init = CreateStaticVarsInitMethod();
	optimized.static_vars_destroy = CreateStaticVarsDestroyMethod();

	std::vector<TMethodPrePostEvents> method_prepost_events;
	int methods_before_prepost_event=methods_table.size();

	InitPrePostEvents(method_prepost_events, methods_before_prepost_event);

	ListItems();

	//оптимизируем комманды
	//TODO
	//лучше сначала оптимизировать пометодно а потом сливать в один список
	//метки можно тоже пометодно создавать если сделать переходы относительными

	//TODO метод не может начинаться с метки
	for(int i=0;i<methods_table.size();i++)
		if(methods_table[i]->GetOps().first!=NULL)
			assert(methods_table[i]->GetOps().first->value.type!=TOpcode::LABEL);
	//преобразуем все комманды методов в единый непрерывный список
	TListItem<TOp>* first_instr=NULL;
	TListItem<TOp>* last_instr=NULL;
	for(int i=0;i<methods_table.size();i++)
	{
		TListItem<TOp>* c=methods_table[i]->GetOps().first;
		if(c==NULL)continue;//метод с пустым телом, но используемый
		assert(c->value.type!=TOpcode::LABEL);//TODO будет ошибка т.к. ккомманда вырежется
		ConnectOps(first_instr,last_instr,methods_table[i]->GetOps());
	}

	printf("Ops list creating = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();
	//////////////


	int ops_count=0;
	//if(false)
	{
		//считываем метки и запоминаем их позиции
		int* labels=curr_label>0?new int[curr_label]:NULL;
		//if(curr_label>0) т.к. нам надо подсчитать количество инструкций
		{
			//TODO а разьве метод не может начинаться с метки
			TListItem<TOp>* curr=first_instr;
			if(curr!=NULL){
				int i=0;
				do{
					while(curr->value.type==TOpcode::LABEL){
						assert(curr!=NULL);//программа не может прерываться меткой, обязательно будет RETURN
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

		optimized.instructions.resize(ops_count);
		optimized.methods_table.resize(methods_table.size());
		TListItem<TOp> *curr=first_instr;
		if(curr!=NULL){
			int curr_method=0,curr_instr=0,curr_event=0;
			do{
				//настраиваем комманды использующие переходы
				switch(curr->value.type)
				{
				case TOpcode::GOTO:
				case TOpcode::GOFALSE:
				case TOpcode::GOTRUE:
				case TOpcode::GLOBAL_TESTANDSET:
					curr->value.v1=labels[curr->value.v1];
				default:;
				}
				//упаковываем комманды
				optimized.instructions[curr_instr]=curr->value;
				//
				//if(curr_method>=methods_before_prepost_event)
				//{
				//	if(method_prepost_events[curr_event].event_ops==curr)
				//	{
				//		TProgram::TMethod& t=optimized.methods_table[curr_method];
				//		t.is_external=methods_table[curr_method]->IsExternal();
				//		t.is_static=methods_table[curr_method]->IsStatic();
				//		t.params_size=methods_table[curr_method]->GetParamsSize();
				//		t.pre_event=-1;
				//		t.post_event=-1;
				//		t.return_size=methods_table[curr_method]->GetRetSize();
				//		curr_event++;
				//		curr_method++;
				//		t.first_op=curr_instr;
				//		assert(!t.is_external);//такие методы не должны быть внешними
				//	}
				//}
				//else
				if (curr_method < methods_table.size())
					
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
						t.pre_event=-1; //TODO индексы тоже надо
						t.post_event=-1;
						t.return_size=methods_table[curr_method]->GetRetSize();
						curr_method++;
						if (!t.is_external)
						{
							t.first_op = curr_instr;
							//
							//TOpArray ops = methods_table[curr_method]->GetOps();
							//TODO довить проверку что команды curr соответствуют командам GetOps() метода
						}
					}
				}
				curr_instr++;
			}while((curr=curr->next)!=NULL);
			assert(curr==NULL);//должны дойти до последней инструкции
			assert(curr_method==methods_table.size());
		}
		if(labels!=NULL)delete[] labels;
	}

	printf("Ops generating = %.3f ms\n",time.TimeDiff(time.GetTime(),t)*1000);
	t=time.GetTime();

	optimized.string_consts.resize(string_consts.size());
	for(int i=0;i<string_consts.size();i++)
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
	return string_consts.size()-1;
}
int TNotOptimizedProgram::AddMethodToTable(TMethod* use_method)
{
	if(use_method==NULL)return -1;
	int i=FindMethod(use_method);
	if(i==-1)
	{
		methods_table.push_back(use_method);
		return methods_table.size()-1;
	}else return i;
}
int TNotOptimizedProgram::CreateArrayElementClassId(TClass* use_class)
{
	for(int i=0;i<array_element_classes.size();i++)
		if(array_element_classes[i]==use_class)return i;
	array_element_classes.push_back(use_class);
	return array_element_classes.size()-1;
}
int TNotOptimizedProgram::FindMethod(TMethod* use_method)
{
	for(int i=0;i<methods_table.size();i++)
		if(methods_table[i]==use_method)return i;
	return -1;
}
