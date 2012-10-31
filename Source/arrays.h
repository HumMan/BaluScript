#pragma once

template<class T,int size>
class TVec;

template<class T>
class TSmartPointer
{
private:
	T* p;
public:
	TSmartPointer()
	{
		p=NULL;
	}
	TSmartPointer(const TSmartPointer& use_p)
	{
		if(use_p.IsNull())p=NULL;
		else
			p=new T(*(use_p.p));
	}
	TSmartPointer(T* use_p)
	{
		p=use_p;
	}
	void operator=(const TSmartPointer& use_p)
	{
		if(use_p.IsNull())p=NULL;
		else
		{
			assert(p==NULL);
			p=new T(*(use_p.p));
		}
	}
	~TSmartPointer()
	{
		delete p;
	}
	T* GetPointer()
	{
		return p;
	}
	T* operator->()
	{
		return p;
	}
	bool IsNull()const
	{
		return p==NULL;
	}
	void operator=(T* use_p)
	{
		assert(p==NULL);
		p=use_p;
	}
};

template<class T,int block_size>
class TAllocator
{
private:
	struct TBlock
	{
		T values[block_size];
		TBlock* prev;
	};
	int count;
	TBlock* end;
#ifdef _DEBUG
	int total_count;
#endif
public:
	TAllocator():count(0),end(NULL)
#ifdef _DEBUG
		,total_count(0)
#endif
	{}
	~TAllocator()
	{
		TBlock* curr=end;
		while(curr!=NULL)
		{
			TBlock* temp=curr;
			curr=curr->prev;
			delete temp;
		}
	}
	T* New()
	{
		if(end!=NULL)
		{
			if(count==block_size)
			{
				TBlock* b=new TBlock();
				count=0;
				b->prev=end;
				end=b;
			}
		}else
		{
			end=new TBlock();
			count=0;
			end->prev=NULL;
		}
#ifdef _DEBUG
		total_count++;
#endif
		return &end->values[count++];
	}
};

template<class T>
class TListItem
{
public:
	T value;
	TListItem<T> *prev;
	TListItem<T> *next;
	TListItem<T>():prev(NULL),next(NULL){}
};

template<class T>
class TList
{
private:
	TListItem<T>* start;
	TListItem<T>* end;
	int high;
	bool CheckHigh()
	{
		TListItem<T>* curr=start;
		int t=-1;
		while(curr!=NULL)
		{
			t++;
			curr=curr->next;
		}
		return high==t;
	}
public:
	TList():start(NULL),end(NULL),high(-1){}
	TListItem<T>* GetEnd(){
		assert(start!=NULL);
		return end;
	}
	TListItem<T>* GetStart(){
		return start;
	}
	int GetHigh()const
	{
		return high;
	}
	int GetCount()const
	{
		return high+1;
	}
	TListItem<T>* Find(int id)const
	{
		TListItem<T> *curr=start;
		int i=0;
		while(curr!=NULL)
		{
			if(i==id)return curr;
			curr=curr->next;
			i++;
		}
		return NULL;
	}
	int Find(TListItem<T>* v)const
	{
		TListItem<T> *curr=start;
		int i=0;
		while(curr!=NULL)
		{
			if(curr==v)return i;
			curr=curr->next;
			i++;
		}
		return -1;
	}
	bool Exists(TListItem<T>* v)const
	{
		TListItem<T> *curr=start;
		while(curr!=NULL)
		{
			if(curr==v)return true;
			curr=curr->next;
		}
		return false;
	}
	void Delete(TListItem<T>* v)
	{
		assert(Exists(v));
		if(v->prev==NULL)
		{
			start=v->next;
			if(start!=NULL)start->prev=NULL;
		}
		else if(v->next==NULL)
		{
			end=v->prev;
			if(end!=NULL)end->next=NULL;
		}
		else
		{
			v->prev->next=v->next;
			v->next->prev=v->prev;
		}
		high--;
		assert(CheckHigh());
	}
	void Delete(T* v)
	{
		Delete((TListItem<T>*) v);
	}
	void Insert(TListItem<T>* v,TListItem<T>* before)
	{	
		assert(CheckHigh());
		if(before!=NULL)assert(Exists(before));
		high++;
		if(start==NULL)
		{
			assert(before==NULL);
			start=v;
			end=v;
			v->next=NULL;
			v->prev=NULL;
		}
		else if(before==start)
		{
			start->prev=v;
			v->next=start;
			v->prev=NULL;
			start=v;
		}
		else if(before==NULL)
		{
			assert(end->next==NULL);
			end->next=v;
			v->prev=end;
			v->next=NULL;
			end=v;
		}
		else{
			assert(end->next==NULL);
			v->next=before;
			v->prev=before->prev;
			before->prev=v;
			v->prev->next=v;
		}
		assert(CheckHigh());
	}
	void Push(TListItem<T>* v)
	{
		Insert(v,NULL);
	}
};

template<class T,int capacity_percent=50>
class TVector
{
protected:
	T *v;
	int v_high;
	int v_max;
	int v_min;
	int reserve;
	bool NeedResize(int new_high)
	{
		return (new_high<v_min||new_high>v_max);
	}
	void Resize(int new_high);
public:
	void Init()
	{
		v=NULL;
		v_high=-1;
		v_max=-1;
		v_min=-1;
		reserve=5;
	}
	TVector():v(NULL),v_high(-1),v_max(-1),v_min(-1),reserve(5)
	{
		assert(capacity_percent>=0&&capacity_percent<=200);
	}	
	void SetReserve(int i)
	{
		reserve=i;
	}
	TVector(const TVector<T>& arr)
	{
		Init();
		Resize(arr.GetHigh());
		for(int i=0;i<=v_high;i++)
			v[i]=arr.v[i];
	}
	~TVector()	
	{
		if(v!=NULL)
		{
			delete[] v;
			v=NULL;
		}
	}
	void operator=(const TVector<T>& arr)
	{
		Resize(arr.GetHigh());
		for(int i=0;i<=v_high;i++)
			v[i]=arr.v[i];
	}
	bool operator==(const TVector<T>& v1)const
	{
		int i;
		if(v_high!=v1.v_high)return false;
		for(i=0;i<=v_high;++i)
				if(v[i]!=v1.v[i])return false;
		return true;
	}
	T operator[](int id)const					
	{
		assert(v_high!=-1&&id>=0&&id<=v_high);
		return v[id];
	}
	T& operator[](int id)								
	{
		assert(v_high!=-1&&id>=0&&id<=v_high);
		return v[id];
	}	
	int GetCount()	const
	{
		return v_high+1;
	}
	int GetHigh()const
	{
		return v_high;
	}
	void Swap(int i,int k)
	{
		assert(v_high!=-1&&i>=0&&i<=v_high);
		assert(k>=0&&k<=v_high);
		::Swap(v[i],v[k]);
	}
	void Empty()
	{
		if(v!=NULL)
		{
			delete[] v;
			Init();
		}
	}
	void SetHigh(int new_high)
	{
		if(NeedResize(new_high))Resize(new_high);
		else v_high=new_high;
	}
	void SetCount(int new_count)
	{
		int new_high=new_count-1;
		if(NeedResize(new_high))Resize(new_high);
		else v_high=new_high;
	}
	void Push(const T &val)
	{
		T temp(val);
		if(NeedResize(v_high+1))
		{
			Resize(v_high+1);
			//проверка того не добавляем ли мы элемент массива переданный по ссылке в этот же массив
			assert(v==0||!(&val>=v&&(&val<=v+v_max)));
		}
		else v_high+=1;
		v[v_high]=val;
	}
	T* Push()
	{ 
		Push(T());
		return &v[v_high];
	}
	void Insert(const T &val,int after_index)
	{
		assert(v==0||!(&val>=v&&(&val<=v+v_max)));
		assert(IsInMinMax(after_index,0,v_high));
		if(NeedResize(v_high+1))Resize(v_high+1);
		else v_high+=1;
		if(after_index!=v_high)
			for(int i=v_high;i>after_index+1;i--)
				v[i]=v[i-1];
		v[after_index+1]=val;
	}
	void InsertCount(int count,int after_index)
	{
		assert(count>=0);
		if(count==0)return;
		assert(IsInMinMax(after_index,0,v_high));
		if(NeedResize(v_high+count))Resize(v_high+count);
		else v_high+=count;
		if(after_index!=v_high)
			for(int i=v_high;i>after_index+count;i--)
				v[i]=v[i-count];
	}
        void InsertCount(const TVector<TVec<int,2> > &count_after_pair)
	{
		
		int total_count_to_insert=0;
		for(int i=0;i<=count_after_pair.GetHigh();i++)
			total_count_to_insert+=count_after_pair[i][0];

		int t=count_after_pair.GetHigh(),off=total_count_to_insert;

		if(total_count_to_insert>0)
		{
			Inc(total_count_to_insert);
			for(int i=v_high;i>0;i--)
			{
				if(i-off==count_after_pair[t][1])
				{
					i-=count_after_pair[t][0];
					off-=count_after_pair[t][0];
					t--;
				}
				v[i]=v[i-off];
				if(t<0)break;
			}
		}
	}
	
	void Inc(int count)
	{ 
		if(NeedResize(v_high+count))Resize(v_high+count);
		else v_high+=count;
	}
	void Delete(int id)
	{
		assert(v_high!=-1&&id>=0&&id<=v_high);
		if(v_high!=id)
			for(int i=id+1;i<=v_high;i++)
				v[i-1]=v[i];
		if(NeedResize(v_high-1))SetHigh(v_high-1);
		else v_high-=1;
	}
	void Delete(const TVector<int> &indices_to_del)
	{
		int t=0,off=0;
		if(indices_to_del.GetCount()>0)
		{
			for(int i=0;i<=v_high;i++)
			{
				if(t<=indices_to_del.GetHigh()&&i==indices_to_del[t])
				{
					off++;
					t++;
				}else if(off!=0&&(i-off)>=0)
				{
					v[i-off]=v[i];
				}
			}
			Inc(-indices_to_del.GetCount());
		}
	}
	int Find(const T& val)const
	{
		for(int i=0;i<=v_high;i++)
			if(v[i]==val)return i;
		return -1;
	}
	T& GetTop(int id)
	{
		assert(id>=0&&id<=v_high);
		return v[v_high-id];
	}
	void Pop()
	{
		Inc(-1);
	}
	T GetPop()
	{
		assert(v_high>=0);
		return v[v_high--];
	}
};

template<class T,int capacity_percent>
void TVector<T,capacity_percent>::Resize(int new_high)
{
	if(v_high==new_high)return;
	if(new_high<-1)assert(false);
	
	int r=int(new_high*(float(capacity_percent)/100))+reserve;//TODO умножаем большое число на большое получаем переполнение, поэтому float
	v_max=new_high+r;
	v_min=new_high-r;
	if(v_min<-1)v_min=-1;

	T *new_arr=(v_max==-1?NULL:(new T[v_max+1]));
	if(v!=NULL){
		for(int i=0;i<=v_high&&i<=new_high;i++)
			new_arr[i]=v[i];
		delete[] v;
	}
	v=new_arr;
	v_high=new_high;
}

template<class T>
class TVectorList
{
protected:
	TVector<T*> v;
public:
	TVectorList(){};
	TVectorList(const TVectorList<T>& arr)
	{
		v.SetHigh(arr.GetHigh());
		for(int i=0;i<=GetHigh();i++){
			v[i]=new T(*arr.v[i]);
			//*v[i]=*arr.v[i];
		}
	}
	~TVectorList()
	{
		for(int i=0;i<=v.GetHigh();i++)
			delete v[i];
	}
	void operator=(const TVectorList<T>& arr)
	{
		//SetHigh(arr.GetHigh());
		//for(int i=0;i<=v.GetHigh();i++) *v[i]=*arr.v[i];

		int last_high=v.GetHigh();
		int new_high=arr.GetHigh();
		if(new_high<last_high)
		{
			for(int i=0;i<=new_high;i++)
				*v[i]=*arr.v[i];
			for(int i=new_high+1;i<=last_high;i++)
				delete v[i];
		}
		v.SetHigh(new_high);
		if(new_high>last_high)
		{
			for(int i=0;i<=last_high;i++)
				*v[i]=*arr.v[i];
			for(int i=last_high+1;i<=new_high;i++)
				v[i]=new T(*arr.v[i]);
		}
		//TODO а где присваивание элементов? DONE проверить
		
	}
	bool operator==(const TVectorList<T>& arr)const
	{
		if(GetHigh()!=arr.GetHigh())return false;
		for(int i=0;i<=GetHigh();i++)
			if(!(*v[i]==*arr.v[i]))return false;
		return true;
	}
	T* operator[](int id)const
	{
		return v[id];
	}
	void SetHigh(int new_high)
	{
		int last_high=v.GetHigh();
		if(new_high<last_high)
		{
			for(int i=new_high+1;i<=last_high;i++)
				delete v[i];
		}
		v.SetHigh(new_high);
		if(new_high>last_high)
		{
			for(int i=last_high+1;i<=new_high;i++)
				v[i]=new T;
		}
	}
	void SetCount(int new_count)
	{
		SetHigh(new_count-1);
	}
	int GetCount()const{
		return v.GetCount();
	}
	int GetHigh()const{
		return v.GetHigh();
	}
	T* GetTop(int id)
	{
		return v.GetTop(id);
	}
	T* Push(T* val)
	{
		v.Push(val);
		return val;
	}
	void Delete(int id)
	{
		delete[] v[id];
		v.Delete(id);
	}
	int Find(T* id)
	{
		return v.Find(id);
	}
};

template<class T>
class TIndexedArray
{
private:
	TVector<T> v;
	TVector<int> free_v;
	void GetNewSpace();
public:
	T& operator[](int id)
	{
		assert(id>=0&&id<=v.GetHigh());
		return v[id];
	}
	TIndexedArray(){}
	TIndexedArray(int use_reserve):v(use_reserve),free_v(use_reserve)
	{
		for(int i=0;i<use_reserve;i++)
		{
			v.add();
			free_v.Push(i);
		}
	}
	int Add(const T& val)
	{
		int temp;
		if(free_v.GetHigh()<0)
		{
			GetNewSpace();
		}
		v[temp=free_v.Pop()]=val;
		return temp;
	}
	int New()
	{
		if(free_v.GetHigh()<0)
		{
			GetNewSpace();
		}
		return free_v.GetPop();
	}
	void Free(int id)	
	{
		free_v.Push(id);
	}
	int GetCount()		
	{
		return v.GetCount();
	}
	int GetHigh()		
	{
		return v.GetHigh();
	}
};

template<class T>
void TIndexedArray<T>::GetNewSpace()
{
	v.Inc(1);
	free_v.Push(v.GetCount()-1);
}

template<class T>
class TIndexedPointerArray
{
private:
	TVectorList<T> v;
	TVector<int> free_v;
	void GetNewSpace()
	{
		v.Add();
		free_v.Push(v.GetHigh());
	}
public:
	TIndexedPointerArray(){}
	TIndexedPointerArray(int use_reserve):v(use_reserve),free_v(use_reserve)
	{
		v.Inc(use_reserve);
		for(int i=0;i<use_reserve;i++)
		{
			free_v.Push(i);
		}
	}
	int New()
	{
		if(free_v.GetHigh()<0)
			GetNewSpace();
		return free_v.GetPop();
	}
	void Delete(int id)
	{
		assert(id>=0);
		free_v.Push(id);
	}
	T* operator[](int id)
	{
		return v[id];
	}
};
