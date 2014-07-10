#include "SType.h"

#include "SClass.h"

TSType::TSType(TSClass* use_owner, TType* use_syntax_node) :TSyntaxNode(use_syntax_node)
{
	owner = use_owner;
}

void TSType::Link(TSClass* use_curr_class)
{
	for (TType::TClassName& v : GetSyntax()->GetClassNames())
	{
		classes.emplace_back(&v);
		use_curr_class = classes.back().Link(owner, use_curr_class);
	}
}

void TSType::Link()
{
	Link(NULL);
}

bool TSType::IsEqualTo(const TSType& use_right)const
{
	assert(GetClass() != NULL&&use_right.GetClass() != NULL);
	return GetClass() == use_right.GetClass();
}

TSClass* TSType_TClassName::Link(TSClass* use_owner, TSClass* use_curr_class)
{
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
			template_params_classes.back().Link();
		}
		
		TTemplateRealizations* templates = use_owner->GetTemplates();
		std::vector<std::shared_ptr<TSClass>>* template_realizations = templates->FindTemplateRealizations(use_curr_class);

		TSClass* realization = NULL;
		if (template_realizations == NULL)
		{
			
			TTemplateRealizations::TTemplateRealization* t = new TTemplateRealizations::TTemplateRealization();
			templates->templates.push_back(std::shared_ptr<TTemplateRealizations::TTemplateRealization>(t));
			t->template_pointer = use_curr_class;
			template_realizations = &t->realizations;
		}
		realization = templates->FindTemplateRealization(use_curr_class, template_params_classes);
		if(realization==NULL)
		{
			//TODO !!!!!!!!!!  шаблонный класс не имеет доступа к своему шаблону
			//при указании имени класса без шаблонныйх параметров - то же самое что с параметрами текущего класса
			realization = new TSClass(use_curr_class->GetOwner(), templates, use_curr_class->GetSyntax());
			template_realizations->push_back(std::shared_ptr<TSClass>(realization));
			realization->template_class = use_curr_class;
			for (TSType& t_par : template_params_classes)
			{
				realization->template_params.push_back(t_par.GetClass());
			}
			realization->Build();		
			realization->Link();
			std::vector<TSClass*> owners;
			realization->CalculateSizes(owners);
			realization->CalculateMethodsSizes();
		}
		use_curr_class=realization;
	}

	class_of_type = use_curr_class;

	return use_curr_class;
}
