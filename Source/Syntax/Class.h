#pragma once

#include <list>
#include <vector>
#include <memory>

#include "../lexer.h"

#include "Type.h"
#include "Method.h"
#include "OverloadedMethod.h"
#include "Accessible.h"

class TClassField;
class TMethod;

class TClass:public TTokenPos
{
protected:
	bool is_template;

	std::list<TNameId> template_params;

	std::list<TClassField> fields;
	std::list<TOverloadedMethod> methods;
	TOverloadedMethod constructors;
	///<summary>Пользовательский деструктор (автоматический деструктор, если существует, будет добавлен как PostEvent)</summary>
	std::unique_ptr<TMethod> destructor;
	TOverloadedMethod operators[TOperator::End];
	TOverloadedMethod conversions;

	std::vector<std::unique_ptr<TClass>> nested_classes;

	///<summary>Имеется пользовательский конструктор по умолчанию</summary>
	bool constr_override;
	///<summary>Название класса</summary>
	TNameId name;
	///<summary>Тип от которого унаследован данный класс</summary>
	TType parent;
	///<summary>От данного класса запрещено наследование</summary>
	bool is_sealed;
	///<summary>Класс в пределах которого объявлен данный класс</summary>
	TClass* owner;
public:
	TClass(TClass* use_owner);

	void AnalyzeSyntax(TLexer& source);


	void AccessDecl(TLexer& source,bool& readonly, TTypeOfAccess::Enum access);

	bool IsTemplate();
	
	void SetIsTemplate(bool use_is_template);
	
	int GetTemplateParamsCount();
	void AddMethod(TMethod* use_method, TNameId name);
	void AddOperator(TOperator::Enum op, TMethod* use_method);
	void AddConversion(TMethod* method);
	void AddConstr(TMethod* use_method);
	void AddDestr(TMethod* use_method);
	void AddNested(TClass* use_class);
	//void AddField(TClassField* use_field);
	TClassField* AddField(TClass* use_field_owner);
	TClass* GetNested(TNameId name);
	TNameId GetName();
	
	TClass* GetOwner();
	TClass* GetClass(TNameId class_name);
	
	bool GetOperators(std::vector<TMethod*> &result, TOperator::Enum op);
	bool GetMethods(std::vector<TMethod*> &result, TNameId use_method_name);
	bool GetMethods(std::vector<TMethod*> &result, TNameId use_method_name, bool is_static);
};