#include "SType.h"

#include "SClass.h"
#include "../Syntax/Statements.h"

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
	for (TType::TClassName& v : GetSyntax()->GetClassNames())
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

	auto template_params = &GetSyntax()->template_params;

	if (template_params->size() != 0)
	{
		
		if(!use_curr_class->GetSyntax()->IsTemplate())
			use_owner->GetSyntax()->Error("Класс не является шаблонным!");
		if(template_params->size()!=use_curr_class->GetSyntax()->GetTemplateParamsCount())
			use_owner->GetSyntax()->Error("Шаблон имеет другое количество параметров!");
		for (TType& t: *template_params)
		{
			template_params_classes.emplace_back(use_owner,&t);
			template_params_classes.back().LinkSignature(static_fields, static_variables);
		}
		
		TSClass* realization = use_curr_class->FindTemplateRealization(template_params_classes);
		if(realization==NULL)
		{
			//TODO !!!!!!!!!!  шаблонный класс не имеет доступа к своему шаблону
			//при указании имени класса без шаблонныйх параметров - то же самое что с параметрами текущего класса
			realization = new TSClass(use_curr_class->GetOwner(), use_curr_class->GetSyntax(), TNodeWithTemplates::Realization);
			use_curr_class->AddTemplateRealization(realization);
			realization->SetTemplateClass(use_curr_class);
			{
				std::vector<TSClass*> template_params;
				for (TSType& t_par : template_params_classes)
				{
					template_params.push_back(t_par.GetClass());
				}
				realization->SetTemplateParams(template_params);
			}
			realization->Build();
			realization->LinkSignature(static_fields, static_variables);
			//std::vector<TSClass*> owners;
			//realization->CalculateSizes(owners);
			//realization->CalculateMethodsSizes();
		}
		use_curr_class=realization;
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

	for (TSType& t_par : template_params_classes)
	{
		t_par.LinkBody(static_fields, static_variables);
	}

	std::vector<TSClass*> owners;
	class_of_type->LinkBody(static_fields, static_variables);
	//class_of_type->CalculateSizes(owners);
	//class_of_type->CalculateMethodsSizes();
}

