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

#include "../SyntaxTree/IClass.h"

class TCanBeEnumeration : public ICanBeEnumeration
{
	bool is_enum;
	bool is_enum_initialized;
	std::vector<Lexer::TNameId> enums;
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
	bool IsEnumeration()const
	{
		return is_enum;
	}	
	std::vector<Lexer::TNameId> GetEnums()const
	{
		return enums;
	}
	int GetEnumId(Lexer::TNameId use_enum)const
	{
		assert(is_enum);
		for (size_t i = 0; i < enums.size(); i++)
			if (enums[i] == use_enum)
				return i;
		return -1;
	}
};

class TClass:public Lexer::TTokenPos, public TCanBeEnumeration, public IClass
{
protected:
	bool is_template;

	std::vector<Lexer::TNameId> template_params;

	std::vector<std::unique_ptr<TClassField>> fields;
	std::vector<std::unique_ptr<TOverloadedMethod>> methods;
	std::unique_ptr<TMethod> constr_default;
	std::unique_ptr<TOverloadedMethod> constr_copy, constr_move;
	///<summary>Пользовательский деструктор</summary>
	std::unique_ptr<TMethod> destructor;
	std::unique_ptr<TOverloadedMethod> operators[(short)Lexer::TOperator::End];
	std::unique_ptr<TOverloadedMethod> conversions;

	std::vector<std::unique_ptr<TClass>> nested_classes;

	///<summary>Название класса</summary>
	Lexer::TNameId name;
	///<summary>Тип от которого унаследован данный класс</summary>
	std::unique_ptr<TType> parent;
	///<summary>От данного класса запрещено наследование</summary>
	bool is_sealed;
	///<summary>Класс в пределах которого объявлен данный класс</summary>
	TClass* owner;
	bool is_external;
	
	void AccessDecl(Lexer::ILexer* source, bool& readonly, TTypeOfAccess access);
public:
	TClass(TClass* use_owner);
	void AnalyzeSyntax(Lexer::ILexer* source);	
	void AddNested(TClass* use_class);
	void SetName(Lexer::TNameId name);

	bool IsTemplate()const;
	bool IsExternal()const;
	
	int GetTemplateParamsCount()const;
	Lexer::TNameId GetTemplateParam(int i)const;

	TClass* GetNested(int i)const;
	TClass* GetNested(Lexer::TNameId name)const;
	int FindNested(Lexer::TNameId name)const;
	int GetNestedCount()const;
	
	Lexer::TNameId GetName()const;	
	TClass* GetOwner()const;
	TType* GetParent()const;
	std::vector<Lexer::TNameId> GetFullClassName()const;
	int GetFieldsCount()const;
	TClassField* GetField(int i) const;
	int GetMethodsCount()const;
	TOverloadedMethod* GetMethod(int i) const;

	bool HasDefConstr()const;
	TMethod* GetDefaultConstructor() const;
	bool HasDestructor()const;
	TMethod* GetDestructor() const;
	TOverloadedMethod* GetCopyConstr() const;
	TOverloadedMethod* GetMoveConstr() const;
	TOverloadedMethod* GetOperator(int i) const;
	TOverloadedMethod* GetConversion() const;

	void AddMethod(TMethod* use_method, Lexer::TNameId name);
	void AddOperator(Lexer::TOperator op, TMethod* use_method);
	void AddConversion(TMethod* method);
	void AddDefaultConstr(TMethod* use_method);
	void AddCopyConstr(TMethod* use_method);
	void AddMoveConstr(TMethod* use_method);
	void AddDestr(TMethod* use_method);
	TClassField* EmplaceField(TClass* use_field_owner);

	ICanBeEnumeration* AsEnumeration() const;
	Lexer::TTokenPos AsTokenPos()const;
};