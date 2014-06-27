#include "Class.h"

#include "../NativeTypes/DynArray.h"
#include "Method.h"
#include "Statements.h"

void TClass::InitOwner(TClass* use_owner)
{
	owner = use_owner;
	//при копировании надо перенастраивать указатель owner у всех
	for (TClassField& var : fields)
		var.InitOwner(this);
	constructors.InitOwner(this);
	if (destructor)
		destructor->InitOwner(this);
	for (int i = 0; i<TOperator::End; i++)
		operators[i].InitOwner(this);
	conversions.InitOwner(this);
	for (const std::shared_ptr<TClass>& nested_class : nested_classes)
		nested_class->InitOwner(this);
}

bool TClass::IsTemplate(){
	return is_template;
}
bool TClass::IsEnum(){
	return is_enum;
}
int TClass::GetEnumId(TNameId use_enum)
{
	assert(is_enum);
	for (int i = 0; i < enums.size();i++)
		if (enums[i] == use_enum)
			return i;
	return -1;
}
void TClass::SetTemplateParamClass(int id, TClass* use_class){
	//template_params[id].class_pointer = use_class;
	int i = 0;
	std::list<TTemplateParam>::iterator it = template_params.begin();
	while (i != id)
	{
		it++;
		i++;
	}
	it->class_pointer = use_class;
}
void TClass::SetIsTemplate(bool use_is_template){
	is_template = use_is_template;
}
TClass* TClass::GetTemplateParamClass(int id){
	//return template_params[id].class_pointer;
	int i = 0;
	std::list<TTemplateParam>::iterator it = template_params.begin();
	while (i!=id)
	{
		it++;
		i++;
	}
	return it->class_pointer;
}
int TClass::GetTemplateParamsCount(){
	return template_params.size();
}

TNameId TClass::GetName(){
	return name;
}
int TClass::GetSize(){
	return size;
}
TClass* TClass::GetOwner(){
	return owner;
}
TClass* TClass::GetParent(){
	return parent.GetClass();
}
TTemplateRealizations* TClass::GetTemplates(){
	return templates;
}

TMethod* TClass::GetAutoDestructor(){
	return auto_destr.get();
}

TClass::TClass(TClass* use_owner, TTemplateRealizations* use_templates) :parent(this) 
{
	is_enum = false;
	is_template = false;
	methods_declared = false;
	methods_build = false;
	auto_methods_build = false;

	templates = use_templates;
	is_sealed = false;
	size = -1;

	auto_def_constr = NULL;
	auto_destr = NULL;

	owner = use_owner;
}

void TClass::CreateInternalClasses() {
	TClass* t = new TClass(this, templates);
	t->size = 1;
	t->name = source->GetIdFromName("dword");
	t->methods_build = true;
	t->auto_methods_build = true;
	t->methods_declared = true;
	nested_classes.push_back(std::unique_ptr<TClass>(t));

	t = new TClass(this, templates);
	t->size = _INTSIZEOF(TDynArr) / 4;
	t->name = source->GetIdFromName("TDynArrayInternalFields");
	t->methods_build = true;
	t->auto_methods_build = true;
	t->methods_declared = true;
	nested_classes.push_back(std::unique_ptr<TClass>(t));
}

void TClass::BuildClass(std::vector<TClass*> buff) {
	assert(!is_template);//шаблон используетс¤ только дл¤ создани¤ реализаций
	if (size > 0)
		return;
	ValidateSizes(buff);
	for (const std::shared_ptr<TClass>& nested_class : nested_classes)
		if (!nested_class->IsTemplate())
			nested_class->BuildClass();
}

void TClass::BuildClass() {
	assert(!is_template);
	if (size > 0)
		return;
	std::vector<TClass*> buff;
	ValidateSizes(buff);
	for (const std::shared_ptr<TClass>& nested_class : nested_classes)
		if (!nested_class->IsTemplate())
			nested_class->BuildClass();
}

void TClass::ValidateSizes(std::vector<TClass*> &owners) {
	int class_size = 0;
	if (std::find(owners.begin(), owners.end(), this) != owners.end()) 
	{
		//Error(" ласс не может содержать поле с собственным типом(кроме дин. массивов)!");//TODO см. initautomethods дл¤ дин массивов
		Error(" ласс не может содержать поле собственного типа!");
	} else {
		owners.push_back(this);
		if (parent.GetClass() != NULL) {
			TClass* parent_class = parent.GetClass();
			parent_class->BuildClass(owners);
			class_size = parent_class->GetSize();
			//добавл¤ем приведение в родительские классы
			do {
				TMethod* temp = new TMethod(this,TClassMember::Conversion);
				temp->SetAs(TOpArray(), parent_class, true, true,1);

				TParameter* t = new TParameter(this, temp);
				t->SetAs(true, this);
				temp->AddParam(t);
				temp->CalcParamSize();
				AddConversion(temp);
				parent_class = parent_class->GetParent();
			} while (parent_class != NULL);
		}
		for (TClassField& field : fields)
		{
			field.GetClass()->BuildClass(owners);
			if (!field.IsStatic()) {
				field.SetOffset(class_size);
				class_size += field.GetClass()->GetSize();
			}
		}
	}
	owners.pop_back();
	size = class_size;
}

void TClass::DeclareMethods() {
	assert(!is_template);
	if (methods_declared)
		return;
	for (TOverloadedMethod& method : methods)
	{
		method.Declare();
	}
	constructors.Declare();
	if (destructor)
		destructor->Declare();

	for (int i = 0; i < TOperator::End; i++)
		operators[i].Declare();
	conversions.Declare();
	methods_declared = true;
	for (const std::shared_ptr<TClass>& nested_class : nested_classes)
		if (!nested_class->IsTemplate())
			nested_class->DeclareMethods();
}

void TClass::BuildMethods(TNotOptimizedProgram &program) {
	assert(!is_template);
	if (methods_build)
		return;
	for (TOverloadedMethod& method : methods)
	{
		method.Build(program);
	}
	constructors.Build(program);
	if (destructor)
		destructor->Build(program);

	for (int i = 0; i < TOperator::End; i++)
		operators[i].Build(program);
	conversions.Build(program);
	methods_build = true;
	for (const std::shared_ptr<TClass>& nested_class : nested_classes)
		if (!nested_class->IsTemplate())
			nested_class->BuildMethods(program);
}

bool TClass::IsChildOf(TClass* use_parent) {
	if (parent.GetClass() == NULL)
		return false;
	if (parent.GetClass() == use_parent)
		return true;
	return parent.GetClass()->IsChildOf(use_parent);
}

void TClass::AddMethod(TMethod* use_method, TNameId name) {
	//»щем перегруженные методы с таким же именем, иначе добавл¤ем новый
	TOverloadedMethod* temp = NULL;
	for (TOverloadedMethod& method : methods)
		if (method.GetName() == name)
			temp = &method;
	if (temp == NULL)
	{
		methods.push_back(name);
		temp = &methods.back();
	}
	temp->methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddOperator(TOperator::Enum op, TMethod* use_method) {
	operators[op].methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddConversion(TMethod* use_method) {
	conversions.methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddConstr(TMethod* use_method) {
	if(use_method->GetParamsCount()==0)
		constr_override=true;
	constructors.methods.push_back(std::unique_ptr<TMethod>(use_method));
}

void TClass::AddDestr(TMethod* use_method) {
	if (destructor)
		Error("ƒеструктор уже существует!");
	destructor = std::shared_ptr<TMethod>(use_method);
}

void TClass::AddNested(TClass* use_class) {
	nested_classes.push_back(std::shared_ptr<TClass>(use_class));
}

//void TClass::AddField(TClassField* use_field) {
	//fields.emplace_back(std::shared_ptr<TClassField>(use_field));
//}
TClassField* TClass::AddField(TClass* use_field_owner)
{
	fields.emplace_back(use_field_owner);
	return &fields.back();
}

TClass* TClass::GetNested(TNameId name) {
	for (const std::shared_ptr<TClass>& nested_class : nested_classes)
		if (nested_class->name == name)
			return nested_class.get();
	return NULL;
}

TClass* TClass::GetClass(TNameId use_name) {
	if (name == use_name)
		return this;
	for (const std::shared_ptr<TClass>& nested_class : nested_classes)
	{
		if (nested_class->name == use_name)
			return nested_class.get();
	}
	
	//assert(!is_template);
	if (!is_template)
	{
		for (const TTemplateParam& template_param : template_params)
			if (template_param.name == use_name)
				return template_param.class_pointer;
	}
		if (owner != NULL)
			return owner->GetClass(use_name);
	return NULL;
}

TClassField* TClass::GetField(TNameId name, bool only_in_this) {
	TClassField* result_ns = GetField(name, false, only_in_this);
	TClassField* result_s = GetField(name, true, only_in_this);
	if (result_ns != NULL)
		return result_ns;
	else
		return result_s;
}

TClassField* TClass::GetField(TNameId name, bool is_static, bool only_in_this) {
	TClassField* result_parent = NULL;
	if (parent.GetClass() != NULL)
		result_parent = parent.GetClass()->GetField(name, true);
	if (result_parent != NULL)
		return result_parent;
	for (TClassField& field : fields)
	{
		if (field.IsStatic() == is_static && field.GetName() == name) {
			return &field;
		}
	}
	if (!only_in_this && is_static && owner != NULL)
		return owner->GetField(name, true, false);
	return NULL;
}

bool TClass::GetConstructors(std::vector<TMethod*> &result) {
	assert(methods_declared&&auto_methods_build);
	for (const std::shared_ptr<TMethod>& constructor : constructors.methods)
	{
		result.push_back(constructor.get());
	}

	if (!constr_override && auto_def_constr)
		result.push_back(auto_def_constr.get());
	return result.size() > 0;
}

TMethod* TClass::GetDefConstr() {
	assert(methods_declared&&auto_methods_build);
	if (constr_override)
	{
		for (const std::shared_ptr<TMethod>& constructor : constructors.methods)
			if (constructor->GetParamsCount() == 0) 
				return constructor.get();
	}
	else
		return auto_def_constr.get();
	return NULL;
}

TMethod* TClass::GetCopyConstr() {
	assert(methods_declared);
	for (const std::shared_ptr<TMethod>& constructor : constructors.methods)
	{
		if (constructor->GetParamsCount() == 1
				&& constructor->GetParam(0)->GetClass() == this
				&& constructor->GetParam(0)->IsRef() == true) {
			return constructor.get();
		}
	}
	return NULL;
}

TMethod* TClass::GetDestructor() {
	assert(methods_declared&&auto_methods_build);
	return (destructor) ? destructor.get() : auto_destr.get();
}

bool TClass::GetOperators(std::vector<TMethod*> &result, TOperator::Enum op) {
	assert(methods_declared&&auto_methods_build);
	operators[op].GetMethods(result);
	return result.size() > 0;
}

TMethod* TClass::GetBinOp(TOperator::Enum op, TClass* left, bool left_ref,
		TClass* right, bool right_ref) {
	assert(methods_declared&&auto_methods_build);
	for (std::shared_ptr<TMethod>& oper : operators[op].methods)
	{
		assert(oper->GetParamsCount() == 2);
		if (oper->GetParam(0)->GetClass() == this
			&& oper->GetParam(0)->IsRef() == true
			&& oper->GetParam(1)->GetClass() == this
			&& oper->GetParam(1)->IsRef()
			== right_ref) {
			return oper.get();
		}
	}
	return NULL;
}

bool TClass::GetMethods(std::vector<TMethod*> &result, TNameId use_method_name) {
	assert(methods_declared);
	for (TOverloadedMethod& ov_method : methods)
	{
		if (ov_method.GetName() == use_method_name)
		{
			for (const std::shared_ptr<TMethod>& method : ov_method.methods)
				result.push_back(method.get());
		}
	}
	if (owner != NULL)
		owner->GetMethods(result, use_method_name, true);
	if (parent.GetClass() != NULL)
		parent.GetClass()->GetMethods(result, use_method_name);
	return result.size() > 0;
}

bool TClass::GetMethods(std::vector<TMethod*> &result, TNameId use_method_name,
		bool is_static) {
	assert(methods_declared);
	for (TOverloadedMethod& ov_method : methods)
	{
		if (ov_method.GetName() == use_method_name)
		{
			for (const std::shared_ptr<TMethod>& method : ov_method.methods)
				if (method->IsStatic() == is_static)
					result.push_back(method.get());
		}
	}
	if (is_static && owner != NULL)
		owner->GetMethods(result, use_method_name, true);
	if (parent.GetClass() != NULL)
		parent.GetClass()->GetMethods(result, use_method_name, is_static);
	return result.size() > 0;
}

TMethod* TClass::GetConversion(bool source_ref, TClass* target_type) {
	assert(methods_declared);
	for (const std::shared_ptr<TMethod>& conversion : conversions.methods)
	{
		if (conversion->GetRetClass() == target_type
			&& conversion->GetParam(0)->IsRef() == source_ref) {
			return conversion.get();
		}
	}
	return NULL;
}

bool TClass::HasDefConstr() {
	assert(auto_methods_build);
	return GetDefConstr() != NULL;
}

bool TClass::HasCopyConstr() {
	assert(auto_methods_build);
	return GetCopyConstr() != NULL;
}

bool TClass::HasDestr() {
	assert(auto_methods_build);
	return GetDestructor() != NULL;
}

bool TClass::HasConversion(TClass* target_type) {
	return GetConversion(true, target_type)
			|| GetConversion(false, target_type);
}

void TClass::AccessDecl(TLexer& source, bool& readonly,
		TTypeOfAccess::Enum access) {
	if (source.Type() == TTokenType::ResWord) {
		switch (source.Token()) {
		case TResWord::Readonly:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = true;
			access = TTypeOfAccess::Public;
			break;
		case TResWord::Public:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Public;
			break;
		case TResWord::Protected:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Protected;
			break;
		case TResWord::Private:
			source.GetToken();
			source.GetToken(TTokenType::Colon);
			readonly = false;
			access = TTypeOfAccess::Private;
			break;
		}
	}
}

void TClass::AnalyzeSyntax(TLexer& source) {
	InitPos(source);

	if(source.TestAndGet(TTokenType::ResWord,TResWord::Enum))
	{
		size=1;
		is_enum=true;
		source.TestToken(TTokenType::Identifier);
		name=source.NameId();
		source.GetToken();
		source.GetToken(TTokenType::LBrace);
		do
		{
			source.Test(TTokenType::Identifier);
			for(int i=0;i<enums.size();i++)
				if(enums[i]==source.NameId())
					source.Error("Њеречисление с таким имененм уже существует!");
			enums.push_back(source.NameId());
			source.GetToken();
		}while(source.TestAndGet(TTokenType::Comma));
		source.GetToken(TTokenType::RBrace);
		return;
	}

	source.GetToken(TTokenType::ResWord, TResWord::Class);
	is_sealed = source.TestAndGet(TTokenType::ResWord, TResWord::Sealed);
	source.TestToken(TTokenType::Identifier);
	name = source.NameId();
	source.GetToken();
	if (source.TestAndGet(TTokenType::Operator, TOperator::Less)) {
		is_template = true;
		do {
			source.TestToken(TTokenType::Identifier);
			TTemplateParam p;
			p.class_pointer = NULL;
			p.name = source.NameId();
			template_params.push_back(p);
			source.GetToken();
			if (!source.TestAndGet(TTokenType::Comma))
				break;
		} while (true);
		source.GetToken(TTokenType::Operator, TOperator::Greater);
	}
	if (source.TestAndGet(TTokenType::Colon)) {
		source.TestToken(TTokenType::Identifier);
		parent.AnalyzeSyntax(source);
	}
	source.GetToken(TTokenType::LBrace);

	bool readonly = false;
	TTypeOfAccess::Enum access = TTypeOfAccess::Public;

	while (true) {
		bool end_while = false;
		AccessDecl(source, readonly, access);
		if (source.Type() == TTokenType::ResWord)
			switch (source.Token()) {
			case TResWord::Class: {
				TClass* nested = new TClass(this, templates);
				AddNested(nested);
				nested->AnalyzeSyntax(source);
			}
				break;
			case TResWord::Enum: {
				TClass* enumeraiton= new TClass(this,templates);
				AddNested(enumeraiton);
				enumeraiton->AnalyzeSyntax(source);
			}
				break;
			case TResWord::Func:
			case TResWord::Constr:
			case TResWord::Destr:
			case TResWord::Operator:
			case TResWord::Conversion: {
				TMethod* func = new TMethod(this);
				func->SetAccess(access);
				func->AnalyzeSyntax(source);
			}
				break;
			default:
				end_while = true;
			}
		else if (source.Type() == TTokenType::Identifier) {
			fields.push_back(this);
			fields.back().SetAccess(access);
			fields.back().SetReadOnly(readonly);
			fields.back().AnalyzeSyntax(source);
		} else
			break;
		if (end_while)
			break;
	}
	source.GetToken(TTokenType::RBrace);
	if (owner == NULL)
		source.GetToken(TTokenType::Done);//в файле должен содержатьс§ только один основной класс
}