#pragma once

#include <vector>
#include <assert.h>

#include "../../Include/common.h"

class TStaticValue;
class TStackValue;

class TRefsList
{
	std::vector<void*> list;
public:
	void AddRef(void* p)
	{
		if (list.size() == 0)
			list.push_back(p);
		else
		{
			for (auto it = list.begin(); it < list.end(); it++)
			{
				if (*it > p)
				{
					list.insert(it, p);
					return;
				}
			}
			list.push_back(p);
		}
	}
	void RemoveRef(void* p)
	{
		for (auto it = list.begin(); it < list.end(); it++)
		{
			if (*it == p)
			{
				list.erase(it);
				break;
			}
		}
	}
	bool RefsInRange(void* left, void* right)
	{
		for (auto it = list.begin(); it < list.end(); it++)
		{
			if (left<=*it && *it<=right)
			{
				return true;
			}
		}
		return false;
	}
};

class BALUSCRIPT_DLL_INTERFACE NonAssignable {
private:
	NonAssignable(NonAssignable const&)=delete;
	NonAssignable& operator=(NonAssignable const&)=delete;
public:
	NonAssignable() {}
};

class TGlobalRunContextPrivate;
class BALUSCRIPT_DLL_INTERFACE TGlobalRunContext: public NonAssignable
{
	TGlobalRunContextPrivate* p;
public:	
	TGlobalRunContext();
	~TGlobalRunContext();
	std::vector<TStaticValue>& GetStaticFields();
	TRefsList& GetRefsList();
};

class TMethodRunContextPrivate;
class BALUSCRIPT_DLL_INTERFACE TMethodRunContext : public NonAssignable
{
	TMethodRunContextPrivate* p;
public:	
	TMethodRunContext(TGlobalRunContext* global_context);
	~TMethodRunContext();
	//запоминаем есть ли ссылки - чтобы в TDynArray, TPtr не произошло use after free после манипуляций
	void AddRefsFromParams();
	//очищаем список ссылок после выполнения метода
	void RemoveRefsFromParams();
	TGlobalRunContext* GetGlobalContext();
	TStackValue& GetObject();
	std::vector<TStackValue>& GetFormalParams();
	bool IsResultReturned();
	void SetResultReturned();
	TStackValue& GetResult();
};

class TStatementRunContextPrivate;
class TStatementRunContext : public NonAssignable
{
	TStatementRunContextPrivate* p;
public:
	TMethodRunContext* GetMethodContext();
	TGlobalRunContext* GetGlobalContext();
	std::vector<TStackValue>& GetLocalVariables();
	std::vector<TStackValue>& GetTempObjects();
	TStatementRunContext(TMethodRunContext* method_context);
	~TStatementRunContext();
};

class TExpressionRunContextPrivate;
class TExpressionRunContext : public NonAssignable
{
	TExpressionRunContextPrivate* p;
public:
	void SetExpressionResult(TStackValue& value);
	TStackValue& GetExpressionResult();
	TStatementRunContext* GetStatementContext();
	TMethodRunContext* GetMethodContext();
	TGlobalRunContext* GetGlobalContext();
	TExpressionRunContext(TStatementRunContext* statement_context);
	~TExpressionRunContext();
};