#pragma once

#include <baluLib.h>

class TClass;

class TTemplateRealizations
{
public:
	struct TTemplateRealization
	{
		TClass* template_pointer;
		TVectorList<TClass> realizations;
	};
	TVectorList<TTemplateRealization> templates;
	TVectorList<TClass>* FindTemplate(TClass* use_template)
	{
		for (int i = 0; i <= templates.GetHigh(); i++)
			if (templates[i]->template_pointer == use_template)
				return &templates[i]->realizations;
		return NULL;
	}
};
