#pragma once

#include "../Syntax/Class.h"
#include "TemplateRealizations.h"
#include "SMethod.h"
#include "SOverloadedMethod.h"
#include "SSyntaxNode.h"

class TTemplateRealizations;
class TSClassField;

class TSClass:public TSyntaxNode<TClass>, public TNodeWithSize,public TNodeSignatureLinked,public TNodeBodyLinked,public TNodeWithTemplates
{	
	std::list<TSClassField> fields;
	std::list<TSOverloadedMethod> methods;
	std::unique_ptr<TSOverloadedMethod> constructors;
	///<summary>Пользовательский деструктор (автоматический деструктор, если существует, будет добавлен как PostEvent)</summary>
	std::unique_ptr<TSMethod> destructor;
	std::unique_ptr<TSOverloadedMethod> operators[TOperator::End];
	std::unique_ptr<TSOverloadedMethod> conversions;

	std::vector<std::unique_ptr<TSClass>> nested_classes;

	std::unique_ptr<TSMethod> auto_def_constr;
	///<summary>Имеется пользовательский конструктор по умолчанию</summary>
	bool constr_override;
	///<summary>Автоматически созданный деструктор</summary>
	std::unique_ptr<TSMethod> auto_destr;
	///<summary>Тип от которого унаследован данный класс</summary>
	TSClass* parent;
	///<summary>От данного класса запрещено наследование</summary>
	bool is_sealed;
	///<summary>Класс в пределах которого объявлен данный класс</summary>
	TSClass* owner;
public:

	void CreateInternalClasses();//TODO только для главного класса
	TSClass(TSClass* use_owner, TClass* use_syntax_node, TNodeWithTemplates::Type type = TNodeWithTemplates::Unknown);
	TSClass* GetClass(TNameId use_name);
	void CheckForErrors();
	TSClass* GetOwner();
	TSClass* GetParent()
	{
		return parent;
	}
	TSClassField* GetField(TNameId name, bool only_in_this);
	TSClassField* GetField(TNameId name, bool is_static, bool only_in_this);
	bool HasConversion(TSClass* target_type);
	bool IsNestedIn(TSClass* use_parent);
	bool GetOperators(std::vector<TSMethod*> &result, TOperator::Enum op);
	TSMethod* GetBinOp(TOperator::Enum op, TSClass* left, bool left_ref, TSClass* right, bool right_ref);
	bool GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name);
	bool GetMethods(std::vector<TSMethod*> &result, TNameId use_method_name, bool is_static);
	bool GetConstructors(std::vector<TSMethod*> &result);
	TSMethod* GetDefConstr();
	TSMethod* GetCopyConstr();
	TSMethod* GetDestructor();
	TSMethod* GetConversion(bool source_ref, TSClass* target_type);
	TSClass* GetNested(TNameId name);
	///<summary>Постоение семанитческого дерева по синтаксическому(для всех кроме тел методов) без какого либо анализа типов</summary>
	void Build();
	void LinkSignature();
	void LinkBody();
	void CalculateSizes(std::vector<TSClass*> &owners);
	void CalculateMethodsSizes();
};