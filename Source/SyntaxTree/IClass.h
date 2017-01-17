#pragma once

#include <list>
#include <vector>
#include <memory>

#include "../lexer.h"

class ICanBeEnumeration
{
public:
	virtual bool IsEnumeration() const =0;
	virtual std::vector<Lexer::TNameId> GetEnums() const = 0;
	virtual int GetEnumId(Lexer::TNameId use_enum) const = 0;
};

class IClass
{
public:
	virtual ICanBeEnumeration* AsEnumeration() const =0;
	virtual Lexer::TTokenPos AsTokenPos() const =0;
	virtual bool IsTemplate()  const =0;
	virtual bool IsExternal()  const =0;
	virtual int GetTemplateParamsCount()  const =0;
	virtual Lexer::TNameId GetTemplateParam(int i) const =0;
	virtual IClass* GetNested(Lexer::TNameId name)  const =0;
	virtual Lexer::TNameId GetName()  const =0;
	virtual IClass* GetOwner()  const =0;
	virtual TType* GetParent()  const =0;
	virtual std::vector<Lexer::TNameId> GetFullClassName()  const =0;
	virtual int GetFieldsCount()  const =0;
	virtual TClassField* GetField(int i)  const =0;
	virtual int GetMethodsCount()  const =0;
	virtual TOverloadedMethod* GetMethod(int i)  const =0;
	virtual bool HasDefConstr() const =0;
	virtual TMethod* GetDefaultConstructor()  const =0;
	virtual bool HasDestructor() const =0;
	virtual TMethod* GetDestructor()  const =0;
	virtual TOverloadedMethod* GetCopyConstr()  const =0;
	virtual TOverloadedMethod* GetMoveConstr()  const =0;
	virtual TOverloadedMethod* GetOperator(int i)  const =0;
	virtual TOverloadedMethod* GetConversion()  const =0;
	virtual int GetNestedCount() const =0;
	virtual IClass* GetNested(int i) const =0;
};