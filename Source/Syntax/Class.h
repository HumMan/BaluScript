
class TClass:public TTokenPos
{
	bool is_enum;
	bool is_template;
	bool methods_declared;//для всех методов настроены типы параметров
	bool methods_build;//построены тела методов
	bool auto_methods_build;//построены тела автоматическких методов
	int size;//если не "-1" то проверены типы членов класса на зацикливание и настроен размер класса

	struct TTemplateParam
	{
		TNameId name;
		TClass* class_pointer;
	};

	TVector<TTemplateParam> template_params;

	TTemplateRealizations* templates;

	TVectorList<TClassField> field;
	TVectorList<TOverloadedMethod> method;
	TOverloadedMethod constructor;
	TSmartPointer<TMethod> destructor;
	TOverloadedMethod operators[TOperator::End];
	TOverloadedMethod conversion;
	TVector<TNameId> enums;

	TVectorList<TClass> nested_class;

	TSmartPointer<TMethod> auto_def_constr;
	bool constr_override;//имеется пользовательский конструктор по умолчанию
	TSmartPointer<TMethod> auto_destr;

	TNameId name;
	TType parent;
	bool is_sealed;

	TClass* owner;

	void ValidateSizes(TVector<TClass*> &owners);
public:
	TClass(TClass* use_owner,TTemplateRealizations* use_templates);
	void InitOwner(TClass* use_owner)
	{
		owner=use_owner;
		//при копировании надо перенастраивать указатель owner у всех
		for(int i=0;i<=field.GetHigh();i++)field[i]->InitOwner(this);
		for(int i=0;i<=method.GetHigh();i++)method[i]->InitOwner(this);
		constructor.InitOwner(this);
		if(!destructor.IsNull())
			destructor->InitOwner(this);
		for(int i=0;i<TOperator::End;i++)operators[i].InitOwner(this);
		conversion.InitOwner(this);
		for(int i=0;i<=nested_class.GetHigh();i++)nested_class[i]->InitOwner(this);
	}
	
	void AnalyzeSyntax(TLexer& source);

	void CreateInternalClasses();

	void BuildClass();
	void BuildClass(TVector<TClass*> buff);
	void DeclareMethods();
	void CheckForErrors();
	void InitAutoMethods(TNotOptimizedProgram &program);
	void BuildMethods(TNotOptimizedProgram &program);

	void AccessDecl(TLexer& source,bool& readonly, TTypeOfAccess::Enum& access);

	bool IsTemplate(){
		return is_template;
	}
	bool IsEnum(){
		return is_enum;
	}
	int GetEnumId(TNameId use_enum)
	{
		assert(is_enum);
		for(int i=0;i<=enums.GetHigh();i++)
			if(enums[i]==use_enum)return i;
		return -1;
	}
	void SetTemplateParamClass(int id,TClass* use_class){
		template_params[id].class_pointer=use_class;
	}
	void SetIsTemplate(bool use_is_template){
		is_template=use_is_template;
	}
	TClass* GetTemplateParamClass(int id){
		return template_params[id].class_pointer;
	}
	int GetTemplateParamsHigh(){
		return template_params.GetHigh();
	}
	bool IsChildOf(TClass* use_parent);
	void AddMethod(TMethod* use_method,TNameId name);
	void AddOperator(TOperator::Enum op,TMethod* use_method);
	void AddConversion(TMethod* method);
	void AddConstr(TMethod* use_method);
	void AddDestr(TMethod* use_method);
	void AddNested(TClass* use_class);
	void AddField(TClassField* use_field);
	TClass* GetNested(TNameId name);
	TClass* GetClass(TNameId use_name);
	TNameId GetName(){
		return name;
	}
	int GetSize(){
		return size;
	}
	TClass* GetOwner(){
		return owner;
	}
	TClass* GetParent(){
		return parent.GetClass();
	}
	TTemplateRealizations* GetTemplates(){
		return templates;
	}
	TClassField* GetField(TNameId name,bool only_in_this);
	TClassField* GetField(TNameId name,bool is_static,bool only_in_this);
	bool GetConstructors(TVector<TMethod*> &result);
	TMethod* GetDefConstr();
	TMethod* GetCopyConstr();
	TMethod* GetDestructor();
	TMethod* GetAutoDestructor(){
		return auto_destr.GetPointer();
	}
	bool GetOperators(TVector<TMethod*> &result,TOperator::Enum op);
	TMethod* GetBinOp(TOperator::Enum op,TClass* left,bool left_ref,TClass* right, bool right_ref);
	bool GetMethods(TVector<TMethod*> &result,TNameId use_method_name);
	bool GetMethods(TVector<TMethod*> &result,TNameId use_method_name,bool is_static);
	TMethod* GetConversion(bool source_ref,TClass* target_type);

	bool HasDefConstr();
	bool HasCopyConstr();
	bool HasDestr();
	bool HasConversion(TClass* target_type);
};