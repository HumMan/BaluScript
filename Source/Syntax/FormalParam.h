
class TFormalParam
{
	TClass* class_pointer;
	TClass* type;//указатель на тип (для доступа к статическим членам и функциям)
	bool is_ref;
	TOpArray ops;
	TVector<TMethod*> methods;//указатель на перегруженые методы
	bool need_push_this;//если вызывются методы класса из самого класса 
	void Init(){
		class_pointer=NULL;
		is_ref=false;
		need_push_this=false;
		type=NULL;
	}
public:
	TFormalParam(){
		Init();
	}
	TFormalParam(TVector<TMethod*> use_methods, bool use_need_push_this){
		Init();
		methods=use_methods;
		need_push_this=use_need_push_this;
	}
	TFormalParam(TClass* use_class,bool use_is_ref,TOpArray use_ops){
		Init();
		class_pointer=use_class;
		is_ref=use_is_ref;
		ops=use_ops;
	}
	TFormalParam(TClass* use_type){
		Init();
		type=use_type;
	}
	bool IsRef()const{
		return is_ref;
	}
	void SetIsRef(bool use_is_ref){
		is_ref=use_is_ref;
	}
	bool NeedPushThis()const{
		return need_push_this;
	}
	bool IsMethods()const{
		return methods.GetCount()!=0;
	}
	bool IsType()const{
		return type!=NULL;
	}
	TClass* GetType()const{
		return type;
	}
	TVector<TMethod*>& GetMethods(){
		return methods;
	}
	TClass* GetClass()const{
		return class_pointer;
	}
	bool IsVoid()const{
		return class_pointer==NULL&&(!IsMethods())&&type==NULL;//TODO в дальнейшем methods_pointer не должен считаться void
	}
	TOpArray& GetOps(){
		return ops;
	}
	TOpArray GetOps()const{
		return ops;
	}
};

inline static TFormalParam operator+(const TOpArray& use_left,const TFormalParam& use_right)
{
	TFormalParam result(use_right);
	result.GetOps()=use_left+use_right.GetOps();
	return result;
}
