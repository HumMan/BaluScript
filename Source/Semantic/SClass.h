#pragma once

#include "../Syntax/Class.h"
#include "TemplateRealizations.h"
#include "SMethod.h"
#include "SOverloadedMethod.h"
#include "SSyntaxNode.h"

class TTemplateRealizations;
class TSClassField;

class TSClass:public TSyntaxNode<TClass>
{	
	///<summary>���� ����� �������� ����������� ������ �� ��������, �� �������� ��������� �� ������, �������� � ���������� �������</summary>
	std::vector<TSClass*> template_params;
	///<summary>���� ����� �������� ����������� ������ �� ��������, �� �������� ��������� �� ��������� ����� �� �������� ���� ������������� ������ ����������</summary>
	TSClass* template_class;
	TTemplateRealizations* templates;

	std::list<TSClassField> fields;
	std::list<TSOverloadedMethod> methods;
	TSOverloadedMethod constructors;
	///<summary>���������������� ���������� (�������������� ����������, ���� ����������, ����� �������� ��� PostEvent)</summary>
	std::shared_ptr<TSMethod> destructor;
	TSOverloadedMethod operators[TOperator::End];
	TSOverloadedMethod conversions;

	std::vector<std::shared_ptr<TSClass>> nested_classes;

	std::shared_ptr<TSMethod> auto_def_constr;
	///<summary>������� ���������������� ����������� �� ���������</summary>
	bool constr_override;
	///<summary>������������� ��������� ����������</summary>
	std::shared_ptr<TSMethod> auto_destr;
	///<summary>��� �� �������� ����������� ������ �����</summary>
	TSClass* parent;
	///<summary>�� ������� ������ ��������� ������������</summary>
	bool is_sealed;
	///<summary>����� � �������� �������� �������� ������ �����</summary>
	TSClass* owner;

public:
	TSClass(TSClass* use_owner, TTemplateRealizations* use_templates,TClass* use_syntax_node);
	TSClass* GetClass(TNameId use_name);
	void CheckForErrors();
	TSClass* GetOwner();
	TSClass* GetParent();
	TSClassField* GetField(TNameId name, bool only_in_this);
	TSClassField* GetField(TNameId name, bool is_static, bool only_in_this);
	const std::vector<TSClass*>& GetTemplateParams()
	{
		return template_params;
	}
	bool GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name);
	bool GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name, bool is_static);
	TSClass* GetNested(TNameId name);
	TTemplateRealizations* GetTemplates();
	void Link();
};