#pragma once

#include "../TreeRunner/RunContext.h"

#include <string>

#include "../SemanticInterface/SemanticTreeApi.h"

class TStaticValue;
class TStackValue;
class TSyntaxAnalyzer;
class TSClass;

class TString
{
private:
	/*TStringWrapper()
	{
		v = new T();
	}
	TStringWrapper(const TStringWrapper& r)
	{
		Init(*r.v);
	}

	void operator=(const TStringWrapper& right)
	{
		*v = *right.v;
	}
*/
	std::string* v;
public:
	void Init()
	{
		v = new std::string();
	}
	void Init(const std::string& value)
	{
		v = new std::string(value);
	}
	
	char GetChar(int i)
	{
		return(*v)[i];
	}
	int GetLength()
	{
		return v->size();
	}
	//T& GetCppValue()
	//{
	//	return *v;
	//}

	void def_constr();
	void copy_constr(TString* param0);
	void destructor();

	static TString operator_Plus(TString* left, TString* right);
	static TString operator_Assign(TString* left, TString* right);
	static TString operator_PlusA(TString* left, TString* right);
	static char& operator_GetArrayElement(TString* obj, int index);

	int length();

	//
	//static void constructor(TMethodRunContext* run_context);
	//static void destructor(TMethodRunContext* run_context);
	//static void copy_constr(TMethodRunContext* run_context);
	//static void assign_op(TMethodRunContext* run_context);
	//static void assign_plus_op(TMethodRunContext* run_context);
	//static void plus_op(TMethodRunContext* run_context);
	//static void get_char_op(TMethodRunContext* run_context);
	//static void get_length(TMethodRunContext* run_context);

	static SemanticApi::TExternalClassDecl DeclareExternalClass();
};
