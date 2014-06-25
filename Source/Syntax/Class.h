#pragma once

#include "../lexer.h"

#include "Type.h"
#include "OverloadedMethod.h"
#include "../notOptimizedProgram.h"
#include "Accessible.h"

#include <list>
#include <vector>
#include <memory>

class TTemplateRealizations;
class TClassField;
class TOverloadedMethod;

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

	std::vector<TTemplateParam> template_params;

	TTemplateRealizations* templates;

	std::vector<std::shared_ptr <TClassField>> fields;
	std::vector<std::shared_ptr<TOverloadedMethod>> methods;
	TOverloadedMethod constructors;
	std::shared_ptr<TMethod> destructor;
	TOverloadedMethod operators[TOperator::End];
	TOverloadedMethod conversions;
	std::vector<TNameId> enums;

	std::vector<std::shared_ptr<TClass>> nested_classes;

	std::shared_ptr<TMethod> auto_def_constr;
	bool constr_override;//имеется пользовательский конструктор по умолчанию
	std::shared_ptr<TMethod> auto_destr;

	TNameId name;
	TType parent;
	bool is_sealed;

	TClass* owner;

	void ValidateSizes(std::vector<TClass*> &owners);
public:
	TClass(TClass* use_owner,TTemplateRealizations* use_templates);
	void InitOwner(TClass* use_owner);
	void AnalyzeSyntax(TLexer& source);

	void CreateInternalClasses();

	void BuildClass();
	void BuildClass(std::vector<TClass*> buff);
	void DeclareMethods();
	void CheckForErrors();
	void InitAutoMethods(TNotOptimizedProgram &program);
	void BuildMethods(TNotOptimizedProgram &program);

	void AccessDecl(TLexer& source,bool& readonly, TTypeOfAccess::Enum access);

	bool IsTemplate();
	bool IsEnum();
	int GetEnumId(TNameId use_enum);
	void SetTemplateParamClass(int id, TClass* use_class);
	void SetIsTemplate(bool use_is_template);
	TClass* GetTemplateParamClass(int id);
	int GetTemplateParamsCount();
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
	TNameId GetName();
	int GetSize();
	TClass* GetOwner();
	TClass* GetParent();
	TTemplateRealizations* GetTemplates();
	TClassField* GetField(TNameId name,bool only_in_this);
	TClassField* GetField(TNameId name,bool is_static,bool only_in_this);
	bool GetConstructors(std::vector<TMethod*> &result);
	TMethod* GetDefConstr();
	TMethod* GetCopyConstr();
	TMethod* GetDestructor();
	TMethod* GetAutoDestructor();
	bool GetOperators(std::vector<TMethod*> &result, TOperator::Enum op);
	TMethod* GetBinOp(TOperator::Enum op,TClass* left,bool left_ref,TClass* right, bool right_ref);
	bool GetMethods(std::vector<TMethod*> &result, TNameId use_method_name);
	bool GetMethods(std::vector<TMethod*> &result, TNameId use_method_name, bool is_static);
	TMethod* GetConversion(bool source_ref,TClass* target_type);

	bool HasDefConstr();
	bool HasCopyConstr();
	bool HasDestr();
	bool HasConversion(TClass* target_type);
};