#pragma once

#include "../TreeRunner/RunContext.h"

#include <string>

#include "../SemanticInterface/SemanticTreeApi.h"

class TStaticValue;
class TStackValue;
class TSyntaxAnalyzer;
class TSClass;

template<class T>
class TStringWrapper
{
private:
	T* v;
public:
	void Init()
	{
		v = new T();
	}
	void Init(const T& value)
	{
		v = new T(value);
	}
	TStringWrapper()
	{
		v = new T();
	}
	TStringWrapper(const TStringWrapper& r)
	{
		Init(*r.v);
	}
	char GetChar(int i)
	{
		return(*v)[i];
	}
	int GetLength()
	{
		return v->size();
	}
	T& GetCppValue()
	{
		return *v;
	}
	void operator=(const TStringWrapper& right)
	{
		*v = *right.v;
	}
	~TStringWrapper()
	{
		delete v;
		v = nullptr;
	}
	static void constructor(TMethodRunContext* run_context);
	static void destructor(TMethodRunContext* run_context);
	static void copy_constr(TMethodRunContext* run_context);
	static void assign_op(TMethodRunContext* run_context);
	static void assign_plus_op(TMethodRunContext* run_context);
	static void plus_op(TMethodRunContext* run_context);
	static void get_char_op(TMethodRunContext* run_context);
	static void get_length(TMethodRunContext* run_context);

	static SemanticApi::TExternalClassDecl DeclareExternalClass();
};

typedef TStringWrapper<std::string> TString;
