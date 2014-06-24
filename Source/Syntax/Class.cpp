#include <baluScript.h>

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
	nested_class.Push(t);

	t = new TClass(this, templates);
	t->size = _INTSIZEOF(TVirtualMachine::TDynArr) / 4;
	t->name = source->GetIdFromName("TDynArrayInternalFields");
	t->methods_build = true;
	t->auto_methods_build = true;
	t->methods_declared = true;
	nested_class.Push(t);
}

void TClass::BuildClass(TVector<TClass*> buff) {
	assert(!is_template);//шаблон используетс€ только дл€ создани€ реализаций
	if (size > 0)
		return;
	ValidateSizes(buff);
	for (int i = 0; i <= nested_class.GetHigh(); i++)
		if (!nested_class[i]->IsTemplate())
			nested_class[i]->BuildClass();
}

void TClass::BuildClass() {
	assert(!is_template);
	if (size > 0)
		return;
	TVector<TClass*> buff;
	ValidateSizes(buff);
	for (int i = 0; i <= nested_class.GetHigh(); i++)
		if (!nested_class[i]->IsTemplate())
			nested_class[i]->BuildClass();
}

void TClass::ValidateSizes(TVector<TClass*> &owners) {
	int class_size = 0;
	if (owners.Find(this) != -1) {
		//Error(" ласс не может содержать поле с собственным типом(кроме дин. массивов)!");//TODO см. initautomethods дл€ дин массивов
		Error(" ласс не может содержать поле собственного типа!");
	} else {
		owners.Push(this);
		if (parent.GetClass() != NULL) {
			TClass* parent_class = parent.GetClass();
			parent_class->BuildClass(owners);
			class_size = parent_class->GetSize();
			//добавл€ем приведение в родительские классы
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
		for (int i = 0; i <= field.GetHigh(); i++) {
			field[i]->GetClass()->BuildClass(owners);
			if (!field[i]->IsStatic()) {
				field[i]->SetOffset(class_size);
				class_size += field[i]->GetClass()->GetSize();
			}
		}
	}
	owners.Pop();
	size = class_size;
}

void TClass::DeclareMethods() {
	assert(!is_template);
	if (methods_declared)
		return;
	for (int i = 0; i <= method.GetHigh(); i++)
		method[i]->Declare();
	constructor.Declare();
	if (!destructor.IsNull())
		destructor->Declare();

	for (int i = 0; i < TOperator::End; i++)
		operators[i].Declare();
	conversion.Declare();
	methods_declared = true;
	for (int i = 0; i <= nested_class.GetHigh(); i++)
		if (!nested_class[i]->IsTemplate())
			nested_class[i]->DeclareMethods();
}

void TClass::BuildMethods(TNotOptimizedProgram &program) {
	assert(!is_template);
	if (methods_build)
		return;
	for (int i = 0; i <= method.GetHigh(); i++)
		method[i]->Build(program);
	constructor.Build(program);
	if (!destructor.IsNull())
		destructor->Build(program);

	for (int i = 0; i < TOperator::End; i++)
		operators[i].Build(program);
	conversion.Build(program);
	methods_build = true;
	for (int i = 0; i <= nested_class.GetHigh(); i++)
		if (!nested_class[i]->IsTemplate())
			nested_class[i]->BuildMethods(program);
}

bool TClass::IsChildOf(TClass* use_parent) {
	if (parent.GetClass() == NULL)
		return false;
	if (parent.GetClass() == use_parent)
		return true;
	return parent.GetClass()->IsChildOf(use_parent);
}

void TClass::AddMethod(TMethod* use_method, TNameId name) {
	//»щем перегруженные методы с таким же именем, иначе добавл€ем новый
	TOverloadedMethod* temp = NULL;
	for (int i = 0; i <= method.GetHigh(); i++)
		if (method[i]->GetName() == name)
			temp = method[i];
	if (temp == NULL)
		temp = method.Push(new TOverloadedMethod(name));
	temp->method.Push(use_method);
}

void TClass::AddOperator(TOperator::Enum op, TMethod* use_method) {
	operators[op].method.Push(use_method);
}

void TClass::AddConversion(TMethod* method) {
	conversion.method.Push(method);
}

void TClass::AddConstr(TMethod* use_method) {
	if(use_method->GetParamsHigh()==-1)
		constr_override=true;
	constructor.method.Push(use_method);
}

void TClass::AddDestr(TMethod* use_method) {
	if (!destructor.IsNull())
		Error("ƒеструктор уже существует!");
	destructor = use_method;
}

void TClass::AddNested(TClass* use_class) {
	nested_class.Push(use_class);
}

void TClass::AddField(TClassField* use_field) {
	field.Push(use_field);
}

TClass* TClass::GetNested(TNameId name) {
	for (int i = 0; i <= nested_class.GetHigh(); i++)
		if (nested_class[i]->name == name)
			return nested_class[i];
	return NULL;
}

TClass* TClass::GetClass(TNameId use_name) {
	if (name == use_name)
		return this;
	for (int i = 0; i <= nested_class.GetHigh(); i++)
		if (nested_class[i]->name == use_name)
			return nested_class[i];
	assert(!is_template);
	for (int i = 0; i <= template_params.GetHigh(); i++)
		if (template_params[i].name == use_name)
			return template_params[i].class_pointer;
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
	for (int i = 0; i <= field.GetHigh(); i++) {
		if (field[i]->IsStatic() == is_static && field[i]->GetName() == name) {
			return field[i];
		}
	}
	if (!only_in_this && is_static && owner != NULL)
		return owner->GetField(name, true, false);
	return NULL;
}

bool TClass::GetConstructors(TVector<TMethod*> &result) {
	assert(methods_declared&&auto_methods_build);
	for (int k = 0; k <= constructor.method.GetHigh(); k++) {
		result.Push(constructor.method[k]);
	}
	if (!constr_override && !auto_def_constr.IsNull())
		result.Push(auto_def_constr.GetPointer());
	return result.GetHigh() >= 0;
}

TMethod* TClass::GetDefConstr() {
	assert(methods_declared&&auto_methods_build);
	if (constr_override)
	{
		for (int i = 0; i <= constructor.method.GetHigh(); i++) 
			if (constructor.method[i]->GetParamsHigh() == -1) 
				return constructor.method[i];
	}
	else
		return auto_def_constr.GetPointer();
	return NULL;
}

TMethod* TClass::GetCopyConstr() {
	assert(methods_declared);
	for (int i = 0; i <= constructor.method.GetHigh(); i++) {
		if (constructor.method[i]->GetParamsHigh() == 0
				&& constructor.method[i]->GetParam(0)->GetClass() == this
				&& constructor.method[i]->GetParam(0)->IsRef() == true) {
			return constructor.method[i];
		}
	}
	return NULL;
}

TMethod* TClass::GetDestructor() {
	assert(methods_declared&&auto_methods_build);
	return (!destructor.IsNull()) ? destructor.GetPointer() : auto_destr.GetPointer();
}

bool TClass::GetOperators(TVector<TMethod*> &result, TOperator::Enum op) {
	assert(methods_declared&&auto_methods_build);
	operators[op].GetMethods(result);
	return result.GetHigh() >= 0;
}

TMethod* TClass::GetBinOp(TOperator::Enum op, TClass* left, bool left_ref,
		TClass* right, bool right_ref) {
	assert(methods_declared&&auto_methods_build);
	for (int i = 0; i <= operators[op].method.GetHigh(); i++) {
		assert(operators[op].method[i]->GetParamsHigh()==1);
		if (operators[op].method[i]->GetParam(0)->GetClass() == this
			&& operators[op].method[i]->GetParam(0)->IsRef() == true
			&& operators[op].method[i]->GetParam(1)->GetClass() == this
			&& operators[op].method[i]->GetParam(1)->IsRef()
			== right_ref) {
				return operators[op].method[i];
		}
	}
	return NULL;
}

bool TClass::GetMethods(TVector<TMethod*> &result, TNameId use_method_name) {
	assert(methods_declared);
	for (int i = 0; i <= method.GetHigh(); i++)
		if (method[i]->GetName() == use_method_name) {
			for (int k = 0; k <= method[i]->method.GetHigh(); k++)
				result.Push(method[i]->method[k]);
		}
	if (owner != NULL)
		owner->GetMethods(result, use_method_name, true);
	if (parent.GetClass() != NULL)
		parent.GetClass()->GetMethods(result, use_method_name);
	return result.GetHigh() >= 0;
}

bool TClass::GetMethods(TVector<TMethod*> &result, TNameId use_method_name,
		bool is_static) {
	assert(methods_declared);
	for (int i = 0; i <= method.GetHigh(); i++)
		if (method[i]->GetName() == use_method_name) {
			for (int k = 0; k <= method[i]->method.GetHigh(); k++)
				if (method[i]->method[k]->IsStatic() == is_static)
					result.Push(method[i]->method[k]);
		}
	if (is_static && owner != NULL)
		owner->GetMethods(result, use_method_name, true);
	if (parent.GetClass() != NULL)
		parent.GetClass()->GetMethods(result, use_method_name, is_static);
	return result.GetHigh() >= 0;
}

TMethod* TClass::GetConversion(bool source_ref, TClass* target_type) {
	assert(methods_declared);
	for (int i = 0; i <= conversion.method.GetHigh(); i++) {
		if (conversion.method[i]->GetRetClass() == target_type
				&& conversion.method[i]->GetParam(0)->IsRef() == source_ref) {
			return conversion.method[i];
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
		TTypeOfAccess::Enum& access) {
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
			for(int i=0;i<=enums.GetHigh();i++)
				if(enums[i]==source.NameId())
					source.Error("Њеречисление с таким имененм уже существует!");
			enums.Push(source.NameId());
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
			template_params.Push(p);
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
			TClassField* temp_field = new TClassField(this);
			field.Push(temp_field);
			temp_field->SetAccess(access);
			temp_field->SetReadOnly(readonly);
			temp_field->AnalyzeSyntax(source);
		} else
			break;
		if (end_while)
			break;
	}
	source.GetToken(TTokenType::RBrace);
	if (owner == NULL)
		source.GetToken(TTokenType::Done);//в файле должен содержатьс§ только один основной класс
}