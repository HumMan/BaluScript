#pragma once

#include "../../SyntaxInterface/SyntaxTreeApi.h"

#include "TemplateRealizations.h"
#include "SSyntaxNode.h"

class TSClassField;
class TSMethod;

class TSClass:public TSyntaxNode<SyntaxApi::IClass>, public TNodeWithSize,public TNodeSignatureLinked,
	public TNodeBodyLinked,public TNodeWithTemplates, public TNodeWithAutoMethods,
	public SemanticApi::ISClass
{	
	class TPrivate;
	std::unique_ptr<TPrivate> _this;

public:
	TSClass(TSClass* use_owner, SyntaxApi::IClass* use_syntax_node, SemanticApi::TNodeWithTemplatesType type = SemanticApi::TNodeWithTemplatesType::Unknown);
	TSClass(TSClass* use_owner);
	~TSClass();

	std::vector<SemanticApi::ISClassField*> GetFields()const;

	void SetExternal(const std::vector<SemanticApi::TExternalSMethod>& bindings, int& curr_bind);

	void AddClass(TSClass* use_class);
	void CopyExternalMethodBindingsFrom(TSClass* source);
	TSClass* GetClass(Lexer::TNameId use_name);
	void CheckForErrors();
	TSClass* GetOwner()const;
	TSClass* GetNestedByFullName(std::vector<Lexer::TNameId> full_name, size_t curr_id);
	TSClassField* GetField(Lexer::TNameId name, bool only_in_this);
	SemanticApi::ISClassField* GetField(size_t i)const;
	size_t GetFieldsCount()const;
	TSClassField* GetField(Lexer::TNameId name, bool is_static, bool only_in_this);
	bool HasConversion(SemanticApi::ISClass* target_type);

	bool IsExternal()const;
	SyntaxApi::IClass* IGetSyntax()const;

	SemanticApi::ISMethod* GetAutoDefConstr()const;
	SemanticApi::ISMethod* GetAutoCopyConstr()const;
	SemanticApi::ISMethod* GetAutoAssignOperator()const;
	SemanticApi::ISMethod* GetAutoDestr()const;
	
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

	void GetMethods(std::vector<SemanticApi::ISMethod*> &result)const;
	bool GetMethods(std::vector<SemanticApi::ISMethod*> &result, Lexer::TNameId use_method_name,
		SemanticApi::Filter is_static=SemanticApi::Filter::NotSet, 
		bool scan_owner = true)const;

	SemanticApi::ISMethod* GetConversion(bool source_ref, SemanticApi::ISClass* target_type)const;

	TSClass* GetNested(Lexer::TNameId name)const;
	size_t GetNestedCount()const;
	ISClass* GetNested(size_t index)const;

	bool GetTemplateParameter(Lexer::TNameId name, SemanticApi::TTemplateParameter& result)const;
	bool HasTemplateParameter(Lexer::TNameId name)const;
	///<summary>ѕостоение семанитческого дерева по синтаксическому(дл€ всех кроме тел методов) без какого либо анализа типов</summary>
	void Build();
	///<param name = 'static_fields'>—писок статических полей класса - необходим дл€ их дальнейшей инициализации</param>
	///<param name = 'static_fields'>—писок статических локальных переменных</param>
	void LinkSignature(SemanticApi::TGlobalBuildContext build_context);
	void LinkBody(SemanticApi::TGlobalBuildContext build_context);
	void CalculateSizes(std::vector<TSClass*> &owners);
	void CalculateMethodsSizes();
	void InitAutoMethods();

	//TODO добавить в дальнейшем проверку - можно ли заменить на memcpy операторы copy и assign
	/*void RunAutoDefConstr(std::vector<TStaticValue> &static_fields, TStackValue& object);
	void RunAutoDestr(std::vector<TStaticValue> &static_fields, TStackValue& object);
	void RunAutoCopyConstr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& object);
	void RunAutoAssign(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params);*/
};