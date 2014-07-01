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
	///<summary>≈сли класс €вл€етс€ реализацией одного из шаблонов, то содержит указатели на классы, заданные в параметрах шаблона</summary>
	std::vector<TSClass*> template_params;
	///<summary>≈сли класс €вл€етс€ реализацией одного из шаблонов, то содержит указатель на шаблонный класс по которому была сгенерирована данна€ реализаци€</summary>
	TSClass* template_class;
	TTemplateRealizations* templates;

	std::list<TSClassField> fields;
	std::list<TSOverloadedMethod> methods;
	TSOverloadedMethod constructors;
	///<summary>ѕользовательский деструктор (автоматический деструктор, если существует, будет добавлен как PostEvent)</summary>
	std::shared_ptr<TSMethod> destructor;
	TSOverloadedMethod operators[TOperator::End];
	TSOverloadedMethod conversions;

	std::vector<std::shared_ptr<TSClass>> nested_classes;

	std::shared_ptr<TSMethod> auto_def_constr;
	///<summary>»меетс€ пользовательский конструктор по умолчанию</summary>
	bool constr_override;
	///<summary>јвтоматически созданный деструктор</summary>
	std::shared_ptr<TSMethod> auto_destr;
	///<summary>“ип от которого унаследован данный класс</summary>
	TSClass* parent;
	///<summary>ќт данного класса запрещено наследование</summary>
	bool is_sealed;
	///<summary> ласс в пределах которого объ€влен данный класс</summary>
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