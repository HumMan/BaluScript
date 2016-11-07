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
	std::vector<Lexer::TNameId> enums;
	int GetEnumId(Lexer::TNameId use_enum)
	{
		assert(is_enum);
		for (size_t i = 0; i < enums.size(); i++)
			if (enums[i] == use_enum)
				return i;
		return -1;
	}
};

class TClass:public Lexer::TTokenPos, public TCanBeEnumeration
{
	friend class TSClass;
	friend class TNodeWithTemplates;
	friend class TSyntaxAnalyzer;
protected:
	bool is_template;

	std::vector<Lexer::TNameId> template_params;

	std::list<TClassField> fields;
	std::list<TOverloadedMethod> methods;
	std::unique_ptr<TMethod> constr_default;
	TOverloadedMethod constr_copy, constr_move;
	///<summary>Пользовательский деструктор</summary>
	std::unique_ptr<TMethod> destructor;
	TOverloadedMethod operators[(short)Lexer::TOperator::End];
	TOverloadedMethod conversions;

	std::vector<std::unique_ptr<TClass>> nested_classes;

	///<summary>Название класса</summary>
	Lexer::TNameId name;
	///<summary>Тип от которого унаследован данный класс</summary>
	TType parent;
	///<summary>От данного класса запрещено наследование</summary>
	bool is_sealed;
	///<summary>Класс в пределах которого объявлен данный класс</summary>
	TClass* owner;
	bool is_external;
public:
	TClass(TClass* use_owner);
	void AnalyzeSyntax(Lexer::ILexer* source);
	void AccessDecl(Lexer::ILexer* source,bool& readonly, TTypeOfAccess access);

	void SetIsTemplate(bool use_is_template);
	
	bool IsTemplate();
	bool IsExternal();
	int GetTemplateParamsCount();
	TClass* GetNested(Lexer::TNameId name);
	Lexer::TNameId GetName();
	TClass* GetOwner();
	std::vector<Lexer::TNameId> GetFullClassName();

	void AddMethod(TMethod* use_method, Lexer::TNameId name);
	void AddOperator(Lexer::TOperator op, TMethod* use_method);
	void AddConversion(TMethod* method);
	//void AddConstr(TMethod* use_method);
	void AddDefaultConstr(TMethod* use_method);
	void AddCopyConstr(TMethod* use_method);
	void AddMoveConstr(TMethod* use_method);
	void AddDestr(TMethod* use_method);
	void AddNested(TClass* use_class);
	TClassField* EmplaceField(TClass* use_field_owner);
};