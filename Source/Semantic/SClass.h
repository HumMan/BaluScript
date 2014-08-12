#pragma once

#include "../Syntax/Class.h"
#include "TemplateRealizations.h"
#include "SMethod.h"
#include "SOverloadedMethod.h"
#include "SSyntaxNode.h"

class TTemplateRealizations;
class TSClassField;
class TSLocalVar;
class TStaticValue;
class TStackValue;

class TSClass:public TSyntaxNode<TClass>, public TNodeWithSize,public TNodeSignatureLinked,public TNodeBodyLinked,public TNodeWithTemplates, public TNodeWithAutoMethods
{	
	friend class TStaticArr;
	std::list<TSClassField> fields;
	std::list<TSOverloadedMethod> methods;

	std::unique_ptr<TSMethod> default_constructor;
	std::unique_ptr<TSOverloadedMethod> copy_constructors,move_constructors;
	///<summary>���������������� ���������� (�������������� ����������, ���� ����������, ����� �������� ��� PostEvent)</summary>
	std::unique_ptr<TSMethod> destructor;
	std::unique_ptr<TSOverloadedMethod> operators[TOperator::End];
	std::unique_ptr<TSOverloadedMethod> conversions;

	std::vector<std::unique_ptr<TSClass>> nested_classes;

	std::unique_ptr<TSMethod> auto_def_constr;
	std::unique_ptr<TSMethod> auto_copy_constr;
	std::unique_ptr<TSMethod> auto_move_constr;
	///<summary>������������� ��������� �������� ������������</summary>
	std::unique_ptr<TSMethod> auto_assign_operator;
	///<summary>������������� ��������� ����������</summary>
	std::unique_ptr<TSMethod> auto_destr;
	///<summary>��� �� �������� ����������� ������ �����</summary>
	TSType parent;
	///<summary>�� ������� ������ ��������� ������������</summary>
	bool is_sealed;
	///<summary>����� � �������� �������� �������� ������ �����</summary>
	TSClass* owner;

public:
	void AddClass(TSClass* use_class);
	void CopyExternalMethodBindingsFrom(TSClass* source);
	TSClass(TSClass* use_owner, TClass* use_syntax_node, TNodeWithTemplates::Type type = TNodeWithTemplates::Unknown);
	TSClass* GetClass(TNameId use_name);
	void CheckForErrors();
	TSClass* GetOwner();
	TSClass* GetParent()
	{
		return parent.GetClass();
	}
	TSClass* GetNestedByFullName(std::vector<TNameId> full_name, int curr_id);
	TSClassField* GetField(TNameId name, bool only_in_this);
	TSClassField* GetField(TNameId name, bool is_static, bool only_in_this);
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
	bool GetOperators(std::vector<TSMethod*> &result, TOperator::Enum op);
	bool GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name);
	bool GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name, bool is_static);
	TSMethod* GetConversion(bool source_ref, TSClass* target_type);

	TSClass* GetNested(TNameId name);

	TNodeWithTemplates::TTemplateParameter GetTemplateParameter(TNameId name);
	bool HasTemplateParameter(TNameId name);
	///<summary>��������� �������������� ������ �� ���������������(��� ���� ����� ��� �������) ��� ������ ���� ������� �����</summary>
	void Build();
	///<param name = 'static_fields'>������ ����������� ����� ������ - ��������� ��� �� ���������� �������������</param>
	///<param name = 'static_fields'>������ ����������� ��������� ����������</param>
	void LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables);
	void CalculateSizes(std::vector<TSClass*> &owners);
	void CalculateMethodsSizes();
	void InitAutoMethods();

	void RunAutoDefConstr(std::vector<TStaticValue> &static_fields, TStackValue& object);
	void RunAutoDestr(std::vector<TStaticValue> &static_fields, TStackValue& object);
	void RunAutoCopyConstr(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& object);
	void RunAutoAssign(std::vector<TStaticValue> &static_fields, std::vector<TStackValue> &formal_params, TStackValue& object);
};