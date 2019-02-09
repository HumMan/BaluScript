#pragma once

#include <vector>

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


class TGlobalRunContext
{
public:
	std::vector<TStaticValue>* static_fields;
	TRefsList* refs_list;
	TGlobalRunContext()
	{
		static_fields = nullptr;
		refs_list = nullptr;
	}
	TGlobalRunContext(std::vector<TStaticValue>* static_fields, TRefsList* refs_list)
	{
		this->static_fields = static_fields;
		this->refs_list = refs_list;
	}
};

class TMethodRunContext : public TGlobalRunContext
{
public:	
	std::vector<TStackValue>* formal_params;
	TStackValue* result;
	TStackValue* object;
	TMethodRunContext()
	{
		static_fields = nullptr;
		formal_params = nullptr;
		result = nullptr;
		object = nullptr;
	}
	TMethodRunContext(TGlobalRunContext global_context, std::vector<TStackValue>* formal_params,
		TStackValue* result, TStackValue* object)
	{
		this->static_fields = global_context.static_fields;
		this->refs_list = global_context.refs_list;
		this->formal_params = formal_params;
		this->result = result;
		this->object = object;
	}
	//запоминаем есть ли ссылки - чтобы в TDynArray, TPtr не произошло use after free после манипуляций
	void AddRefsFromParams();
	//очищаем список ссылок после выполнения метода
	void RemoveRefsFromParams();
};

class TStatementRunContext : public TMethodRunContext
{
public:
	bool* result_returned;
	std::vector<TStackValue>* local_variables;

	TStatementRunContext()
	{
		result_returned = nullptr;
		local_variables = nullptr;
	}
	TStatementRunContext(TMethodRunContext method_context, bool* result_returned, std::vector<TStackValue>* local_variables)
		:TMethodRunContext(method_context)
	{
		this->result_returned = result_returned;
		this->local_variables = local_variables;
	}
	TStatementRunContext(TMethodRunContext method_context, std::vector<TStackValue>* local_variables)
		:TMethodRunContext(method_context)
	{
		this->result_returned = nullptr;
		this->local_variables = local_variables;
	}
};

class TExpressionRunContext : public TStatementRunContext
{
public:
	TStackValue* expression_result;

	TExpressionRunContext()
	{
		expression_result = nullptr;
	}
	TExpressionRunContext(TStatementRunContext statement_context, TStackValue* expression_result)
		:TStatementRunContext(statement_context)
	{
		this->expression_result = expression_result;
	}
};