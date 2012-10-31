
#pragma once

template<class T,int size>
class TMatrix;

template<class T,int size>
class TVec
{
	enum
	{
		high=size-1
	};
protected:
	T v[size];
public:
	TVec(){}

#define TVEC_ASSIGN_BIN_OP(h,result,op_a,op,op_b)\
	if(h>=0)result[0]=op_a[0] op op_b[0];\
	if(h>=1)result[1]=op_a[1] op op_b[1];\
	if(h>=2)result[2]=op_a[2] op op_b[2];\
	if(h>=3)result[3]=op_a[3] op op_b[3];

#define TVEC_ASSIGN_BIN_OP_VAL(h,result,op_a,op,val)\
	if(h>=0)result[0]=op_a[0] op val;\
	if(h>=1)result[1]=op_a[1] op val;\
	if(h>=2)result[2]=op_a[2] op val;\
	if(h>=3)result[3]=op_a[3] op val;

#define TVEC_BIN_OP(h,op_a,op,op_b)\
	if(h>=0)op_a[0]op op_b[0];\
	if(h>=1)op_a[1]op op_b[1];\
	if(h>=2)op_a[2]op op_b[2];\
	if(h>=3)op_a[3]op op_b[3];

#define TVEC_BIN_OP_VAL(h,op_a,op,val)\
	if(h>=0)op_a[0]op val;\
	if(h>=1)op_a[1]op val;\
	if(h>=2)op_a[2]op val;\
	if(h>=3)op_a[3]op val;

#define TVEC_BIN_OP_SUM(h,op_a,op,op_b,sum_op)\
	(((h>=0)?(op_a[0]op op_b[0]):0)sum_op\
	 ((h>=1)?(op_a[1]op op_b[1]):0)sum_op\
	 ((h>=2)?(op_a[2]op op_b[2]):0)sum_op\
	 ((h>=3)?(op_a[3]op op_b[3]):0))\

#define TVEC_BIN_OP_SUM_VAL(h,op_a,op,val,sum_op)\
	(((h>=0)?(op_a[0]op val):0)sum_op\
	 ((h>=1)?(op_a[1]op val):0)sum_op\
	 ((h>=2)?(op_a[2]op val):0)sum_op\
	 ((h>=3)?(op_a[3]op val):0))\

#define TVEC_BIN_OP_SUM_FUNC(h,op_a,op,func,op_b,sum_op)\
	(((h>=0)?(op_a[0]op func(op_b[0])):0)sum_op\
	 ((h>=1)?(op_a[1]op func(op_b[1])):0)sum_op\
	 ((h>=2)?(op_a[2]op func(op_b[2])):0)sum_op\
	 ((h>=3)?(op_a[3]op func(op_b[3])):0))\

	explicit TVec(T v0)
	{
		TVEC_BIN_OP_VAL(high,v,=,v0);
	}
	explicit TVec(T v0, T v1)
	{
		//COMPILE_TIME_ERR(size==2);
		v[0]=v0;
		v[1]=v1;
	}
	explicit TVec(T v0, T v1, T v2)
	{
		//COMPILE_TIME_ERR(size==3);
		v[0]=v0;
		v[1]=v1;
		v[2]=v2;
	}
	explicit TVec(const TVec<T,2>& v0, T v1)
	{
		COMPILE_TIME_ERR(size==3);
		v[0]=v0[0];
		v[1]=v0[1];
		v[2]=v1;
	}
	explicit TVec(T v0, T v1, T v2, T v3)
	{
		COMPILE_TIME_ERR(size==4);
		v[0]=v0;
		v[1]=v1;
		v[2]=v2;
		v[3]=v3;
	}
	explicit TVec(const TVec<T,2>& v0, T v1, T v2)
	{
		COMPILE_TIME_ERR(size==4);
		v[0]=v0[0];
		v[1]=v0[1];
		v[2]=v1;
		v[3]=v2;
	}
	explicit TVec(const TVec<T,3>& v0, T v1)
	{
		COMPILE_TIME_ERR(size==4);
		v[0]=v0[0];
		v[1]=v0[1];
		v[2]=v0[2];
		v[3]=v1;
	}
	friend TVec operator+(const TVec& v1, const TVec& v2)
	{
		TVec t;
		TVEC_ASSIGN_BIN_OP(high,t.v,v1.v,+,v2.v);
		return t;
	}
	friend TVec operator-(const TVec& v1, const TVec& v2)
	{
		TVec t;
		TVEC_ASSIGN_BIN_OP(high,t.v,v1.v,-,v2.v);
		return t;
	}
	friend TVec operator-(const TVec& v1)
	{
		TVec t(v1);
		TVEC_BIN_OP(high,t.v,=-,v1.v);
		return t;
	}
	friend T operator*(const TVec& v1,const TVec& v2)
	{
		return TVEC_BIN_OP_SUM(high,v1.v,*,v2.v,+);
	}

	friend TVec operator/(const TVec& v1,const TVec& v2)
	{
		TVec t;
		TVEC_ASSIGN_BIN_OP(high,t.v,v1.v,/,v2.v);
		return t;
	}

	friend TVec operator*(const TVec& v1,const T val)
	{
		TVec t;
		TVEC_ASSIGN_BIN_OP_VAL(high,t.v,v1.v,*,val);
		return t;
	}
	friend TVec operator/(const TVec& v1,const T val)
	{
		TVec t;
		TVEC_ASSIGN_BIN_OP_VAL(high,t.v,v1,/,val);
		return t;
	}
	friend TVec operator<<(const TVec& v1,int i)
	{
		TVec t(v1);
		TVEC_BIN_OP_VAL(high,t.v,<<=,i);
		return t;
	}
	friend TVec operator>>(const TVec& v1,int i)
	{
		TVec t(v1);
		TVEC_BIN_OP_VAL(high,t.v,>>=,i);
		return t;
	}
	void operator+=(const TVec& v1)
	{
		TVEC_BIN_OP(high,v,+=,v1.v);
	}
	void operator-=(const TVec& v1)
	{
		TVEC_BIN_OP(high,v,-=,v1.v);
	}
	void operator*=(const TVec& v1)
	{
		TVEC_BIN_OP(high,v,*=,v1.v);
	}
	void operator/=(const TVec& v1)
	{
		TVEC_BIN_OP(high,v,/=,v1.v);
	}
	void operator*=(T val)
	{
		TVEC_BIN_OP_VAL(high,v,*=,val);
	}
	void operator<<=(int i)
	{
		TVEC_BIN_OP_VAL(high,v,<<=,i);
	}
	void operator>>=(int i)
	{
		TVEC_BIN_OP_VAL(high,v,>>=,i);
	}
	bool operator==(TVec right)
	{
		return TVEC_BIN_OP_SUM(high,v,==,right.v,&&);
	}
	bool operator!=(TVec right)
	{
		return TVEC_BIN_OP_SUM(high,v,!=,right.v,||);
	}

	T operator[](int id)const
	{
		assert(id>=0&&id<size);
		return v[id];
	}
	T& operator[](int id)
	{
		assert(id>=0&&id<size);
		return v[id];
	}
	TVec ComponentMul(const TVec& v1)const
	{
		TVec t;
		TVEC_ASSIGN_BIN_OP(high,t.v,v,*,v1.v);
		return t;
	}
	T AbsScalarMul(const TVec& v1)const
	{
		return TVEC_BIN_OP_SUM_FUNC(high,v,*,abs,v1,+);
	}
	T SqrLength()const
	{
		return TVEC_BIN_OP_SUM(high,v,*,v,+);
	}
	T Length()const
	{
		return sqrt(SqrLength());
	}
	void Normalize()
	{
		T inv_length=1/Length();
		TVEC_BIN_OP_VAL(high,v,*=,inv_length);
	}
	TVec GetNormalized()const
	{
		TVec t(*this);
		t.Normalize();
		return t;
	}
	T Distance(const TVec& v0)const
	{
		TVec t;
		TVEC_ASSIGN_BIN_OP(high,t.v,v,-,v0.v);
		return t.Length();
	}
	T SqrDistance(const TVec& v0)const
	{
		TVec t;
		TVEC_ASSIGN_BIN_OP(high,t.v,v,-,v0.v);
		return t.SqrLength();
	}
	TVec<T,size> Cross(const TVec<T,size>& v1)const			// i     j     k
	{													// v[0]  v[1]  v[2]
		assert(size==3);//COMPILE_TIME_ERR(size==3);						// v1[0] v1[1] v1[2]
		return TVec<T,size>( 
			v[1]*v1[2]-v[2]*v1[1],
			-v[0]*v1[2]+v[2]*v1[0],
			v[0]*v1[1]-v[1]*v1[0]);
	}
	TVec<T,2> Cross()const
	{
		COMPILE_TIME_ERR(size==2);
		return TVec<T,2>( 
			-v[1],v[0]);
	}
	//angle - ���� �������� � ��������
	TVec GetRotated(T angle)const
	{
		TVec dir(cos(angle),sin(angle));
		TMatrix<T,2> m(dir,dir.Cross());
		return m*(*this);
	}
	//angle - ���� �������� � ��������
	TVec GetRotated(const TVec& axis,T angle)const
	{
		if (angle)
		{
			//TVec ax=axis.GetNormalized();
			//assert(abs(axis.Length()-1)<0.0000001);
			TVec proj=axis*(axis*(*this));
			return proj+axis.Cross(*this)*sin(angle)+(*this-proj)*cos(angle);
		}
		else return *this;
	}
	TVec<T,3> GetHomogen()const 
	{
		COMPILE_TIME_ERR(size==4);
		return TVec<T,3>(
			v[0]/v[3],
			v[1]/v[3],
			v[2]/v[3]);
	}
	void MakeRandom()
	{
		TVEC_BIN_OP_VAL(high,v,=,Randfs());
	}
	void MakeRandom(const TVec& dir,T focus_min,T focus_max)
	{
		//TODO ������� ������� ������ ��������� ��������� ������ � ����� �����(����������� 180 ��������� ��� ����������� �������)
		// ��� �� �������
		//TVec temp((dir[1]==0&&dir[2]==0)?TVec3(0,1,0):TVec3(1,0,0));
		//float r=Randf();
		//TVec3 cr(dir.GetRotated(dir.Cross(temp),(focus_min*r+focus_max*(1-r))*float(M_PI)/180.0f));
		//return (cr.GetRotated(dir,rand()/float(RAND_MAX*2*M_PI))).GetNormalized();
	}
	bool IsZero()const
	{
		return TVEC_BIN_OP_SUM_VAL(high,v,==,0,&&);
	}
	static T AngleBetween(const TVec& v0,const TVec& v1)//returns minimal angle between vectors (unsigned)
	{
		if(v0.IsZero()||v1.IsZero())assert(false);//return 0;
		float temp=v0*v1/(sqrt(v0.SqrLength()*v1.SqrLength()));
		if(temp>1)temp=1;
		else if(temp<-1)temp=-1;
		return acos(temp);
	}
	static TVec ClosestPointOnLine(const TVec& p,const TVec& p1,const TVec& p2)
	{
		TVec v(p2-p1);
		T length=v.Length();
		v*=1/length;
		T proj=v*(p-p1);
		return proj<0?p1:(proj>length?p2:p1+v*proj);
	}
	static TVec GetNormal(const TVec& v0,const TVec& v1,const TVec& v2)
	{
		return ((v1-v0).Cross(v2-v0)).GetNormalized();
	}
};

typedef TVec<float,2>			TVec2;
typedef TVec<double,2>			TVec2d;
typedef TVec<int,2>				TVec2i;
typedef TVec<unsigned int,2>	TVec2ui;
typedef TVec<float,3>			TVec3;
typedef TVec<double,3>			TVec3d;
typedef TVec<float,4>			TVec4;
typedef TVec<double,4>			TVec4d;
