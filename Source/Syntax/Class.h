#pragma once

#include <list>
#include <vector>
#include <memory>

#include "../lexer.h"

#include "../Syntax/Method.h"

#include "Type.h"
#include "OverloadedMethod.h"
#include "Accessible.h"

class TClassField;
class TMethod;

class TCanBeEnumeration
{
	bool is_enum;
	bool is_enum_initialized;
public:
	TCanBeEnumeration()
	{
		is_enum = false;
		is_enum_initialized = false;
	}
	void SetAsEnumeration()
	{
		assert(!is_enum_initialized);
		is_enum_initialized = true;
		is_enum = true;
	}
	bool IsEnumeration()
	{
		return is_enum;
	}
	std::vector<TNameId> enums;
	int GetEnumId(TNameId use_enum)
	{
		assert(is_enum);
		for (int i = 0; i < enums.size(); i++)
			if (enums[i] == use_enum)
				return i;
		return -1;
	}
};

class TClass:public TTokenPos, public TCanBeEnumeration
{
	friend class TSClass;
	friend class TNodeWithTemplates;
protected:
	bool is_template;

	std::vector<TNameId> template_params;

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
	bool is_external;
public:
	TClass(TClass* use_owner);

	void AnalyzeSyntax(TLexer& source);


	void AccessDecl(TLexer& source,bool& readonly, TTypeOfAccess::Enum access);

	bool IsTemplate();
	bool IsExternal()
	{
		return is_external;
	}
	
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
};