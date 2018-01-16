#include "SType.h"

#include "SClass.h"
#include "SStatements.h"

TSType::TSType(TSClass* use_owner, SyntaxApi::IType* use_syntax_node) :TSyntaxNode(use_syntax_node)
{
	owner = use_owner;
}

TSType::TSType(TSClass* use_owner, TSClass* use_class) : TSyntaxNode(nullptr)
{
	owner = use_owner;
	classes.emplace_back(use_class);
	classes.back().class_of_type = use_class;
}

void TSType::LinkSignature(SemanticApi::TGlobalBuildContext build_context, TSClass* use_curr_class)
{
	for (auto v : GetSyntax()->GetClassNames())
	{
		classes.emplace_back(v.get());
		use_curr_class = classes.back().LinkSignature(build_context, owner, use_curr_class);
	}
}

void TSType::LinkSignature(SemanticApi::TGlobalBuildContext build_context)
{ 
	if (!IsSignatureLinked())
		SetSignatureLinked();
	else
		return;
	LinkSignature(build_context, nullptr);
}

void TSType::LinkBody(SemanticApi::TGlobalBuildContext build_context)
{
	if (!IsBodyLinked())
		SetBodyLinked();
	else
		return;
	for (TSType_TClassName& v : classes)
	{
		v.LinkBody(build_context);
	}
}

bool TSType::IsEqualTo(const TSType& use_right)const
{
	assert(GetClass() != nullptr&&use_right.GetClass() != nullptr);
	return GetClass() == use_right.GetClass();
}

TSClass* TSType_TClassName::LinkSignature(SemanticApi::TGlobalBuildContext build_context,TSClass* use_owner, TSClass* use_curr_class)
{
	if (!IsSignatureLinked())
		SetSignatureLinked();
	else
		return nullptr;

	if(use_curr_class==nullptr)
	{
		use_curr_class = use_owner->GetClass(GetSyntax()->GetName());
		if(use_curr_class==nullptr)
			use_owner->GetSyntax()->Error("Неизвестный тип!");
	}else
	{
		use_curr_class = use_curr_class->GetNested(GetSyntax()->GetName());
		if(use_curr_class==nullptr)
			use_owner->GetSyntax()->Error("Вложенного класса с таким именем не существует!");
	}

	if (use_curr_class->GetType()==TNodeWithTemplates::Template)
	{
		auto template_params = GetSyntax()->GetTemplateParams();

		if (template_params.size() != 0)
		{

			if (!use_curr_class->GetSyntax()->IsTemplate())
				use_owner->GetSyntax()->Error("Класс не является шаблонным!");
			if (template_params.size() != use_curr_class->GetSyntax()->GetTemplateParamsCount())
				use_owner->GetSyntax()->Error("Шаблон имеет другое количество параметров!");
			for (const auto t : template_params)
			{
				template_params_classes.emplace_back();
				template_params_classes.back().is_value = t->IsValue();
				//если параметр шаблона это константный идентификатор шаблона, то копируем его значение
				if (t->IsValue())
				{
					template_params_classes.back().value = t->GetValue();
				}
				else
				{
					//проверяем не является ли идентификатор шаблонным параметром
					if (use_owner->GetType() == TNodeWithTemplates::Realization && use_owner->HasTemplateParameter(t->GetType()->GetClassNames().back()->GetName()))
					{
						SemanticApi::TTemplateParameter val;
						if (!use_owner->GetTemplateParameter(t->GetType()->GetClassNames().back()->GetName(), val))
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
							template_params_classes.back().type.reset(new TSType(use_owner, t->GetType()));
							template_params_classes.back().type->LinkSignature(build_context);
						}
					}
					else
					{
						template_params_classes.back().type.reset(new TSType(use_owner, t->GetType()));
						template_params_classes.back().type->LinkSignature(build_context);
					}
				}
			}

			TSClass* realization = use_curr_class->FindTemplateRealization(template_params_classes);
			if (realization == nullptr)
			{
				realization = new TSClass(use_curr_class->GetOwner(), use_curr_class->GetSyntax(), TNodeWithTemplates::Realization);
				use_curr_class->AddTemplateRealization(realization);
				realization->SetTemplateClass(use_curr_class);
				{
					std::vector<SemanticApi::TTemplateParameter> template_params;
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
				realization->LinkSignature(build_context);
				
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


void TSType_TClassName::LinkBody(SemanticApi::TGlobalBuildContext build_context)
{
	if (!IsBodyLinked())
		SetBodyLinked();
	else
		return;

	for (TSType_TTemplateParameter& t_par : template_params_classes)
	{
		if (!t_par.is_value)
			t_par.type->LinkBody(build_context);
	}

	std::vector<TSClass*> owners;
	class_of_type->LinkBody(build_context);
	//class_of_type->CalculateSizes(owners);
	//class_of_type->CalculateMethodsSizes();
}

SemanticApi::ISClass* TSType::GetClass()const
{
	if (classes.size() == 0)
		return nullptr;
	return classes.back().class_of_type;
}
