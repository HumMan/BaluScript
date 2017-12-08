#pragma once

#include "../SyntaxInterface/SyntaxTreeApi.h"

#include "TemplateRealizations.h"
#include "SMethod.h"
#include "SOverloadedMethod.h"
#include "SSyntaxNode.h"

class TSClassField;
class TStaticValue;
class TStackValue;

class BALUSCRIPT_DLL_INTERFACE TSClass:public TSyntaxNode<SyntaxApi::IClass>, public TNodeWithSize,public TNodeSignatureLinked,public TNodeBodyLinked,public TNodeWithTemplates, public TNodeWithAutoMethods
{	
	class TPrivate;
	std::unique_ptr<TPrivate> _this;

public:
	TSClass(TSClass* use_owner, SyntaxApi::IClass* use_syntax_node, TNodeWithTemplates::Type type = TNodeWithTemplates::Unknown);
	~TSClass();

	TSMethod* GetAutoDefConstr()const;
	TSMethod* GetAutoDestr()const;
	void AddClass(TSClass* use_class);
	void CopyExternalMethodBindingsFrom(TSClass* source);
	TSClass* GetClass(Lexer::TNameId use_name);
	void CheckForErrors();
	TSClass* GetOwner();
	TSClass* GetParent();
	TSClass* GetNestedByFullName(std::vector<Lexer::TNameId> full_name, size_t curr_id);
	TSClassField* GetField(Lexer::TNameId name, bool only_in_this);
	TSClassField* GetField(size_t i)const;
	size_t GetFieldsCount()const;
	TSClassField* GetField(Lexer::TNameId name, bool is_static, bool only_in_this);
	bool HasConversion(TSClass* target_type);
	bool IsNestedIn(TSClass* use_parent);
	
	TSMethod* GetDefConstr();
	TSMethod* GetDestructor();
	//
	TSMethod* GetCopyConstr();
	TSMethod* GetMoveConstr();
	TSMethod* GetAssignOperator();
	//
	bool GetCopyConstructors(std::vector<TSMethod*> &result);
	bool GetMoveConstructors(std::vector<TSMethod*> &result);
	bool GetOperators(std::vector<TSMethod*> &result, Lexer::TOperator op);
	bool GetMethods(std::vector<TSMethod*> &result, Lexer::TNameId use_method_name);
	bool GetMethods(std::vector<TSMethod*> &result, Lexer::TNameId use_method_name, bool is_static);
	TSMethod* GetConversion(bool source_ref, TSClass* target_type);

	TSClass* GetNested(Lexer::TNameId name);

	bool GetTemplateParameter(Lexer::TNameId name, TNodeWithTemplates::TTemplateParameter& result);
	bool HasTemplateParameter(Lexer::TNameId name);
	///<summary>ѕостоение семанитческого дерева по синтаксическому(дл€ всех кроме тел методов) без какого либо анализа типов</summary>
	void Build();
	///<param name = 'static_fields'>—писок статических полей класса - необходим дл€ их дальнейшей инициализации</param>
	///<param name = 'static_fields'>—писок статических локальных переменных</param>
	void LinkSignature(TGlobalBuildContext build_context);
	void LinkBody(TGlobalBuildContext build_context);
	void CalculateSizes(std::vector<TSClass*> &owners);
	void CalculateMethodsSizes();
	void InitAutoMethods();

	//TODO добавить в дальнейшем проверку - можно ли заменить на memcpy операторы copy и assign
	void RunAutoDefConstr(std::vector<TStaticValue> &static_fields, TStackValue& object);
	void RunAutoDestr(std::vector<TStaticValue> &static_fields, TStackValue& object);
	void RunAutoCopyConstr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& object);
	void RunAutoAssign(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params);
};