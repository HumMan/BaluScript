namespace ns_Script 
{
	namespace ns_int 
	{
	}
	namespace ns_float 
	{
	}
	namespace ns_bool 
	{
	}
	namespace ns_vec2 
	{
	}
	namespace ns_vec2i 
	{
	}
	namespace ns_char 
	{
	}
	namespace ns_dword 
	{
	}
	namespace ns_TDynArray 
	{
		void bind_def_constr(TMethodRunContext* run_context) 
		{
			// return --- nothing
			TDynArr* obj = ((TDynArr*)(run_context->object->get()));
			obj->def_constr(run_context);
		}
		void bind_copy_constr(TMethodRunContext* run_context) 
		{
			//TDynArr& p0 return --- nothing
			TDynArr* obj = ((TDynArr*)(run_context->object->get()));
			TDynArr* param0 = ((TDynArr*)(*run_context->formal_params)[0].get());
			obj->copy_constr(run_context, param0);
		}
		void bind_destructor(TMethodRunContext* run_context) 
		{
			// return --- nothing
			TDynArr* obj = ((TDynArr*)(run_context->object->get()));
			obj->destructor(run_context);
		}
		void bind_operator_Assign(TMethodRunContext* run_context) 
		{
			//TDynArr& p0, TDynArr& p1 return --- nothing
			TDynArr* param0 = ((TDynArr*)(*run_context->formal_params)[0].get());
			TDynArr* param1 = ((TDynArr*)(*run_context->formal_params)[1].get());
			TDynArr::operator_Assign(run_context, param0, param1);
		}
		void bind_operator_GetArrayElement(TMethodRunContext* run_context) 
		{
			//TDynArr& p0, int p1 return --- void*&
			TDynArr* param0 = ((TDynArr*)(*run_context->formal_params)[0].get());
			int param1 = *((int*)(*run_context->formal_params)[1].get());
			void*  result = 
			TDynArr::operator_GetArrayElement(run_context, param0, param1);
			run_context->result->SetAsReference(result);
		}
		void bind_resize(TMethodRunContext* run_context) 
		{
			//int p0 return --- nothing
			TDynArr* obj = ((TDynArr*)(run_context->object->get()));
			int param0 = *((int*)(*run_context->formal_params)[0].get());
			obj->resize(run_context, param0);
		}
		void bind_size(TMethodRunContext* run_context) 
		{
			// return --- int
			TDynArr* obj = ((TDynArr*)(run_context->object->get()));
			int  result = 
			obj->size(run_context);
			*(int*)run_context->result->get() = result;
		}
	}
	namespace ns_TStaticArray 
	{
		void bind_operator_GetArrayElement(TMethodRunContext* run_context) 
		{
			//TStaticArr& p0, int p1 return --- void*&
			TStaticArr* param0 = ((TStaticArr*)(*run_context->formal_params)[0].get());
			int param1 = *((int*)(*run_context->formal_params)[1].get());
			void*  result = 
			TStaticArr::operator_GetArrayElement(run_context, param0, param1);
			run_context->result->SetAsReference(result);
		}
		void bind_size(TMethodRunContext* run_context) 
		{
			// return --- int
			TStaticArr* obj = ((TStaticArr*)(run_context->object->get()));
			int  result = 
			obj->size(run_context);
			*(int*)run_context->result->get() = result;
		}
	}
	namespace ns_string 
	{
		void bind_def_constr(TMethodRunContext* run_context) 
		{
			// return --- nothing
			TString* obj = ((TString*)(run_context->object->get()));
			obj->def_constr();
		}
		void bind_copy_constr(TMethodRunContext* run_context) 
		{
			//TString& p0 return --- nothing
			TString* obj = ((TString*)(run_context->object->get()));
			TString* param0 = ((TString*)(*run_context->formal_params)[0].get());
			obj->copy_constr(param0);
		}
		void bind_destructor(TMethodRunContext* run_context) 
		{
			// return --- nothing
			TString* obj = ((TString*)(run_context->object->get()));
			obj->destructor();
		}
		void bind_operator_Plus(TMethodRunContext* run_context) 
		{
			//TString& p0, TString& p1 return --- TString
			TString* param0 = ((TString*)(*run_context->formal_params)[0].get());
			TString* param1 = ((TString*)(*run_context->formal_params)[1].get());
			TString  result = 
			TString::operator_Plus(param0, param1);
			*(TString*)run_context->result->get() = result;
		}
		void bind_operator_Assign(TMethodRunContext* run_context) 
		{
			//TString& p0, TString& p1 return --- nothing
			TString* param0 = ((TString*)(*run_context->formal_params)[0].get());
			TString* param1 = ((TString*)(*run_context->formal_params)[1].get());
			TString::operator_Assign(param0, param1);
		}
		void bind_operator_PlusA(TMethodRunContext* run_context) 
		{
			//TString& p0, TString& p1 return --- nothing
			TString* param0 = ((TString*)(*run_context->formal_params)[0].get());
			TString* param1 = ((TString*)(*run_context->formal_params)[1].get());
			TString::operator_PlusA(param0, param1);
		}
		void bind_operator_GetArrayElement(TMethodRunContext* run_context) 
		{
			//TString& p0, int p1 return --- char&
			TString* param0 = ((TString*)(*run_context->formal_params)[0].get());
			int param1 = *((int*)(*run_context->formal_params)[1].get());
			char & result = 
			TString::operator_GetArrayElement(param0, param1);
			run_context->result->SetAsReference(&result);
		}
		void bind_length(TMethodRunContext* run_context) 
		{
			// return --- int
			TString* obj = ((TString*)(run_context->object->get()));
			int  result = 
			obj->length();
			*(int*)run_context->result->get() = result;
		}
	}
}
namespace ns_Script
{
	std::vector<SemanticApi::TExternalSMethod> Register()
	{
		std::vector<SemanticApi::TExternalSMethod> result;
		result.push_back(ns_Script::ns_TDynArray::bind_def_constr);
		result.push_back(ns_Script::ns_TDynArray::bind_copy_constr);
		result.push_back(ns_Script::ns_TDynArray::bind_destructor);
		result.push_back(ns_Script::ns_TDynArray::bind_operator_Assign);
		result.push_back(ns_Script::ns_TDynArray::bind_operator_GetArrayElement);
		result.push_back(ns_Script::ns_TDynArray::bind_resize);
		result.push_back(ns_Script::ns_TDynArray::bind_size);
		result.push_back(ns_Script::ns_TStaticArray::bind_operator_GetArrayElement);
		result.push_back(ns_Script::ns_TStaticArray::bind_size);
		result.push_back(ns_Script::ns_string::bind_def_constr);
		result.push_back(ns_Script::ns_string::bind_copy_constr);
		result.push_back(ns_Script::ns_string::bind_destructor);
		result.push_back(ns_Script::ns_string::bind_operator_Plus);
		result.push_back(ns_Script::ns_string::bind_operator_Assign);
		result.push_back(ns_Script::ns_string::bind_operator_PlusA);
		result.push_back(ns_Script::ns_string::bind_operator_GetArrayElement);
		result.push_back(ns_Script::ns_string::bind_length);
		return result;
	}
}
