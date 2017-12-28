#pragma once

#include "../../SyntaxInterface/SyntaxTreeApi.h"

#include "BuildContext.h"

#include "TemplateRealizations.h"
//#include "SMethod.h"
//#include "SOverloadedMethod.h"
#include "SSyntaxNode.h"

class TSClassField;
class TSMethod;
class TStaticValue;
class TStackValue;

class TSClass:public TSyntaxNode<SyntaxApi::IClass>, public TNodeWithSize,public TNodeSignatureLinked,public TNodeBodyLinked,public TNodeWithTemplates, public TNodeWithAutoMethods,
	public SemanticApi::ISClass
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
	SemanticApi::ISClassField* GetField(size_t i)const;
	size_t GetFieldsCount()const;
	TSClassField* GetField(Lexer::TNameId name, bool is_static, bool only_in_this);
	bool HasConversion(SemanticApi::ISClass* target_type);
	bool IsNestedIn(SemanticApi::ISClass* use_parent);
	
	SemanticApi::ISMethod* GetDefConstr()const;
	SemanticApi::ISMethod* GetDestructor()const;
	//
	SemanticApi::ISMethod* GetCopyConstr()const;
	SemanticApi::ISMethod* GetMoveConstr()const;
	SemanticApi::ISMethod* GetAssignOperator()const;
	//
	bool GetCopyConstructors(std::vector<SemanticApi::ISMethod*> &result)const;
	bool GetMoveConstructors(std::vector<SemanticApi::ISMethod*> &result)const;
	bool GetOperators(std::vector<SemanticApi::ISMethod*> &result, Lexer::TOperator op)const;
	bool GetMethods(std::vector<SemanticApi::ISMethod*> &result, Lexer::TNameId use_method_name)const;
	bool GetMethods(std::vector<SemanticApi::ISMethod*> &result, Lexer::TNameId use_method_name, bool is_static)const;
	SemanticApi::ISMethod* GetConversion(bool source_ref, SemanticApi::ISClass* target_type)const;

	TSClass* GetNested(Lexer::TNameId name)const;

	bool GetTemplateParameter(Lexer::TNameId name, SemanticApi::TTemplateParameter& result)const;
	bool HasTemplateParameter(Lexer::TNameId name)const;
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
	/*void RunAutoDefConstr(std::vector<TStaticValue> &static_fields, TStackValue& object);
	void RunAutoDestr(std::vector<TStaticValue> &static_fields, TStackValue& object);
	void RunAutoCopyConstr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& object);
	void RunAutoAssign(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params);*/
};