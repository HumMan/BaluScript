#pragma once

#include "Class.h"

class TTemplateRealizations
{
public:
	struct TTemplateRealization
	{
		///<summary>��������� �� �������� ��������� �����</summary>
		TClass* template_pointer;
		///<summary>������ ���������� ������� ������� � ���������� ���������� �����������</summary>
		std::vector<std::unique_ptr<TClass>> realizations;
	};
	///<summary>������ ���� �������� � �� ����������</summary>
	std::vector<std::unique_ptr<TTemplateRealization>> templates;
	std::vector<std::unique_ptr<TClass>>* FindTemplate(TClass* use_template)
	{
		for (int i = 0; i < templates.size(); i++)
			if (templates[i]->template_pointer == use_template)
				return &templates[i]->realizations;
		return NULL;
	}
};
