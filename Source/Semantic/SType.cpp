#include "SType.h"

#include "SClass.h"
#include "SStatements.h"

#include "../Syntax/Statements.h"
#include "../Syntax/Method.h"

TSType::TSType(TSClass* use_owner, TType* use_syntax_node) :TSyntaxNode(use_syntax_node)
{
	owner = use_owner;
}

TSType::TSType(TSClass* use_owner, TSClass* use_class) : TSyntaxNode(NULL)
{
	owner = use_owner;
	classes.emplace_back(use_class);
	classes.back().class_of_type = use_class;
}

void TSType::LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables, TSClass* use_curr_class)
{
	for (TType_TClassName& v : GetSyntax()->GetClassNames())
	{
		classes.emplace_back(&v);
		use_curr_class = classes.back().LinkSignature(static_fields,static_variables, owner, use_curr_class);
	}
}

void TSType::LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{ 
	if (!IsSignatureLinked())
		SetSignatureLinked();
	else
		return;
	LinkSignature(static_fields, static_variables, NULL);
}

void TSType::LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	if (!IsBodyLinked())
		SetBodyLinked();
	else
		return;
	for (TSType_TClassName& v : classes)
	{
		v.LinkBody(static_fields, static_variables);
	}
}

bool TSType::IsEqualTo(const TSType& use_right)const
{
	assert(GetClass() != NULL&&use_right.GetClass() != NULL);
	return GetClass() == use_right.GetClass();
}

TSClass* TSType_TClassName::LinkSignature(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables,TSClass* use_owner, TSClass* use_curr_class)
{
	if (!IsSignatureLinked())
		SetSignatureLinked();
	else
		return NULL;

	if(use_curr_class==NULL)
	{
		use_curr_class = use_owner->GetClass(GetSyntax()->name);
		if(use_curr_class==NULL)
			use_owner->GetSyntax()->Error("Неизвестный тип!");
	}else
	{
		use_curr_class=use_curr_class->GetNested(GetSyntax()->name);
		if(use_curr_class==NULL)
			use_owner->GetSyntax()->Error("Вложенного класса с таким именем не существует!");
	}

	if (use_curr_class->GetType()==TNodeWithTemplates::Template)
	{
		auto template_params = &GetSyntax()->template_params;

		if (template_params->size() != 0)
		{

			if (!use_curr_class->GetSyntax()->IsTemplate())
				use_owner->GetSyntax()->Error("Класс не является шаблонным!");
			if (template_params->size() != use_curr_class->GetSyntax()->GetTemplateParamsCount())
				use_owner->GetSyntax()->Error("Шаблон имеет другое количество параметров!");
			for (TType_TTemplateParameter& t : *template_params)
			{
				template_params_classes.emplace_back();
				template_params_classes.back().is_value = t.is_value;
				//если параметр шаблона это константный идентификатор шаблона, то копируем его значение
				if (t.is_value)
				{
					template_params_classes.back().value = t.value;
				}
				else
				{
					//проверяем не является ли идентификатор шаблонным параметром
					if (use_owner->GetType() == TNodeWithTemplates::Realization && use_owner->HasTemplateParameter(t.type->GetClassNames().back().name))
					{
						TNodeWithTemplates::TTemplateParameter val;
						if (!use_owner->GetTemplateParameter(t.type->GetClassNames().back().name, val))
						{
							use_owner->GetSyntax()->Error("Не найден шаблонный параметр!");
						}
						if (val.is_value)
						{

							template_params_classes.back().is_value = true;
							template_params_classes.back().value = val.value;
						}
						else
						{
							template_params_classes.back().type.reset(new TSType(use_owner, t.type.get()));
							template_params_classes.back().type->LinkSignature(static_fields, static_variables);
						}
					}
					else
					{
						template_params_classes.back().type.reset(new TSType(use_owner, t.type.get()));
						template_params_classes.back().type->LinkSignature(static_fields, static_variables);
					}
				}
			}

			TSClass* realization = use_curr_class->FindTemplateRealization(template_params_classes);
			if (realization == NULL)
			{
				realization = new TSClass(use_curr_class->GetOwner(), use_curr_class->GetSyntax(), TNodeWithTemplates::Realization);
				use_curr_class->AddTemplateRealization(realization);
				realization->SetTemplateClass(use_curr_class);
				{
					std::vector<TNodeWithTemplates::TTemplateParameter> template_params;
					for (TSType_TTemplateParameter& t_par : template_params_classes)
					{
						template_params.emplace_back();
						template_params.back().is_value = t_par.is_value;
						if (!t_par.is_value)
							template_params.back().type = t_par.type->GetClass();
						template_params.back().value = t_par.value;
					}
					realization->SetTemplateParams(template_params);
				}
				realization->Build();
				realization->LinkSignature(static_fields, static_variables);
				
				//если класс или его методы являются внешними, то копируем привязки к внешним методам
				realization->CopyExternalMethodBindingsFrom(use_curr_class);

				if (use_curr_class->GetSyntax()->IsExternal())
				{
					
					realization->SetSize(use_curr_class->GetSize());
					realization->SetAutoMethodsInitialized();
					//TODO если класс внешний то размер должен получаться из обработчика, в зависимости от параметров
					//но размер мы можем получить только после CalculateSizes
				}
				else
				{
					realization->InitAutoMethods();
				}

				//std::vector<TSClass*> owners;
				//realization->CalculateSizes(owners);
				//realization->CalculateMethodsSizes();
			}
			use_curr_class = realization;
		}
		//иначе указано название шаблона, что соотвествует текущей реализации
		else
		{
			if (use_owner->GetType() != TNodeWithTemplates::Realization || use_owner->GetTemplateClass() != use_curr_class)
				use_owner->GetSyntax()->Error("Использование шаблона без параметров допускается только в самом шаблоне как типа текущей реализации!");
			use_curr_class = use_owner;
		}
	}

	class_of_type = use_curr_class;

	return use_curr_class;
}


void TSType_TClassName::LinkBody(std::vector<TSClassField*>* static_fields, std::vector<TSLocalVar*>* static_variables)
{
	if (!IsBodyLinked())
		SetBodyLinked();
	else
		return;

	for (TSType_TTemplateParameter& t_par : template_params_classes)
	{
		if (!t_par.is_value)
			t_par.type->LinkBody(static_fields, static_variables);
	}

	std::vector<TSClass*> owners;
	class_of_type->LinkBody(static_fields, static_variables);
	//class_of_type->CalculateSizes(owners);
	//class_of_type->CalculateMethodsSizes();
}

