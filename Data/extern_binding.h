//auto-generated code
namespace ns_Script 
{
	void callScriptFromC_Print(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, const float p0 /*v*/) 
	{
		TMethodRunContext method_context(&global_context);
		auto& params=method_context.GetFormalParams();
		params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("float"))));
		*(float*)params[0].get() = p0;
		TreeRunner::Run(compiled_method, method_context);
	}
	namespace ns_int 
	{
		void callScriptFromC_operator_PlusA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, int& p0 /*l*/, const int p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_MinusA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, int& p0 /*l*/, const int p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_MulA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, int& p0 /*l*/, const int p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_DivA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, int& p0 /*l*/, const int p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_PercentA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, int& p0 /*l*/, const int p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_Increment(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, int& p0 /*l*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int**)params[0].get() = &p0;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_Decrement(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, int& p0 /*l*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("int"))));
			*(int**)params[0].get() = &p0;
			TreeRunner::Run(compiled_method, method_context);
		}
	}
	namespace ns_float 
	{
		void callScriptFromC_operator_PlusA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, float& p0 /*l*/, const float p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("float"))));
			*(float**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("float"))));
			*(float*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_MinusA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, float& p0 /*l*/, const float p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("float"))));
			*(float**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("float"))));
			*(float*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_MulA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, float& p0 /*l*/, const float p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("float"))));
			*(float**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("float"))));
			*(float*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_DivA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, float& p0 /*l*/, const float p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("float"))));
			*(float**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("float"))));
			*(float*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
	}
	namespace ns_bool 
	{
	}
	namespace ns_vec2 
	{
		void callScriptFromC_operator_PlusA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, BaluLib::TVec2& p0 /*l*/, const BaluLib::TVec2 p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2"))));
			*(BaluLib::TVec2**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2"))));
			*(BaluLib::TVec2*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_MinusA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, BaluLib::TVec2& p0 /*l*/, const BaluLib::TVec2 p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2"))));
			*(BaluLib::TVec2**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2"))));
			*(BaluLib::TVec2*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_MulA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, BaluLib::TVec2& p0 /*l*/, const BaluLib::TVec2 p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2"))));
			*(BaluLib::TVec2**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2"))));
			*(BaluLib::TVec2*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_DivA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, BaluLib::TVec2& p0 /*l*/, const BaluLib::TVec2 p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2"))));
			*(BaluLib::TVec2**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2"))));
			*(BaluLib::TVec2*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
	}
	namespace ns_vec2i 
	{
		void callScriptFromC_operator_PlusA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, BaluLib::TVec2i& p0 /*l*/, const BaluLib::TVec2i p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2i"))));
			*(BaluLib::TVec2i**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2i"))));
			*(BaluLib::TVec2i*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_MinusA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, BaluLib::TVec2i& p0 /*l*/, const BaluLib::TVec2i p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2i"))));
			*(BaluLib::TVec2i**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2i"))));
			*(BaluLib::TVec2i*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_MulA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, BaluLib::TVec2i& p0 /*l*/, const BaluLib::TVec2i p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2i"))));
			*(BaluLib::TVec2i**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2i"))));
			*(BaluLib::TVec2i*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
		void callScriptFromC_operator_DivA(TGlobalRunContext& global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax, BaluLib::TVec2i& p0 /*l*/, const BaluLib::TVec2i p1 /*r*/) 
		{
			TMethodRunContext method_context(&global_context);
			auto& params=method_context.GetFormalParams();
			params.push_back(TStackValue(true, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2i"))));
			*(BaluLib::TVec2i**)params[0].get() = &p0;
			params.push_back(TStackValue(false, syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName("vec2i"))));
			*(BaluLib::TVec2i*)params[1].get() = p1;
			TreeRunner::Run(compiled_method, method_context);
		}
	}
	namespace ns_char 
	{
	}
	namespace ns_dword 
	{
	}
	namespace ns_TDynArray 
	{
		void bind_def_constr(TMethodRunContext& run_context) 
		{
			// return --- nothing
			TDynArr* obj = ((TDynArr*)(run_context.GetObject().get()));
			obj->def_constr(run_context);
		}
		void bind_copy_constr(TMethodRunContext& run_context) 
		{
			//TDynArr&  copy_from return --- nothing
			TDynArr* obj = ((TDynArr*)(run_context.GetObject().get()));
			TDynArr*  param0 = ((TDynArr*)run_context.GetFormalParams()[0].get());
			obj->copy_constr(run_context, param0);
		}
		void bind_destructor(TMethodRunContext& run_context) 
		{
			// return --- nothing
			TDynArr* obj = ((TDynArr*)(run_context.GetObject().get()));
			obj->destructor(run_context);
		}
		void bind_operator_Assign(TMethodRunContext& run_context) 
		{
			//static - TDynArr&  v, TDynArr&  l return --- nothing
			TDynArr*  param0 = ((TDynArr*)run_context.GetFormalParams()[0].get());
			TDynArr*  param1 = ((TDynArr*)run_context.GetFormalParams()[1].get());
			TDynArr::operator_Assign(run_context, param0, param1);
		}
		void bind_operator_GetArrayElement(TMethodRunContext& run_context) 
		{
			//static - TDynArr&  v, int  id return --- void*&
			TDynArr*  param0 = ((TDynArr*)run_context.GetFormalParams()[0].get());
			const int&  param1 = *((int*)run_context.GetFormalParams()[1].get());
			void*  result = 
			TDynArr::operator_GetArrayElement(run_context, param0, param1);
			run_context.GetResult().SetAsReference(result);
		}
		void bind_resize(TMethodRunContext& run_context) 
		{
			//int  new_size return --- nothing
			TDynArr* obj = ((TDynArr*)(run_context.GetObject().get()));
			const int&  param0 = *((int*)run_context.GetFormalParams()[0].get());
			obj->resize(run_context, param0);
		}
		void bind_size(TMethodRunContext& run_context) 
		{
			// return --- int
			TDynArr* obj = ((TDynArr*)(run_context.GetObject().get()));
			int  result = 
			obj->size(run_context);
			*(int*)run_context.GetResult().get() = result;
		}
	}
	namespace ns_TStaticArray 
	{
		void bind_operator_GetArrayElement(TMethodRunContext& run_context) 
		{
			//static - TStaticArr&  v, int  id return --- void*&
			TStaticArr*  param0 = ((TStaticArr*)run_context.GetFormalParams()[0].get());
			const int&  param1 = *((int*)run_context.GetFormalParams()[1].get());
			void*  result = 
			TStaticArr::operator_GetArrayElement(run_context, param0, param1);
			run_context.GetResult().SetAsReference(result);
		}
		void bind_size(TMethodRunContext& run_context) 
		{
			// return --- int
			TStaticArr* obj = ((TStaticArr*)(run_context.GetObject().get()));
			int  result = 
			obj->size(run_context);
			*(int*)run_context.GetResult().get() = result;
		}
	}
	namespace ns_string 
	{
		void bind_def_constr(TMethodRunContext& run_context) 
		{
			// return --- nothing
			TString* obj = ((TString*)(run_context.GetObject().get()));
			obj->def_constr();
		}
		void bind_copy_constr(TMethodRunContext& run_context) 
		{
			//TString&  copy_from return --- nothing
			TString* obj = ((TString*)(run_context.GetObject().get()));
			TString& param0 = *((TString*)run_context.GetFormalParams()[0].get());
			obj->copy_constr(param0);
		}
		void bind_destructor(TMethodRunContext& run_context) 
		{
			// return --- nothing
			TString* obj = ((TString*)(run_context.GetObject().get()));
			obj->destructor();
		}
		void bind_operator_Plus(TMethodRunContext& run_context) 
		{
			//static - TString&  v, TString&  l return --- TString
			TString& param0 = *((TString*)run_context.GetFormalParams()[0].get());
			TString& param1 = *((TString*)run_context.GetFormalParams()[1].get());
			TString  result = 
			TString::operator_Plus(param0, param1);
			*(TString*)run_context.GetResult().get() = result;
		}
		void bind_operator_Assign(TMethodRunContext& run_context) 
		{
			//static - TString&  v, TString&  l return --- nothing
			TString& param0 = *((TString*)run_context.GetFormalParams()[0].get());
			TString& param1 = *((TString*)run_context.GetFormalParams()[1].get());
			TString::operator_Assign(param0, param1);
		}
		void bind_operator_PlusA(TMethodRunContext& run_context) 
		{
			//static - TString&  v, TString&  l return --- nothing
			TString& param0 = *((TString*)run_context.GetFormalParams()[0].get());
			TString& param1 = *((TString*)run_context.GetFormalParams()[1].get());
			TString::operator_PlusA(param0, param1);
		}
		void bind_operator_GetArrayElement(TMethodRunContext& run_context) 
		{
			//static - TString&  v, int  id return --- char&
			TString& param0 = *((TString*)run_context.GetFormalParams()[0].get());
			const int&  param1 = *((int*)run_context.GetFormalParams()[1].get());
			char & result = 
			TString::operator_GetArrayElement(param0, param1);
			run_context.GetResult().SetAsReference(&result);
		}
		void bind_length(TMethodRunContext& run_context) 
		{
			// return --- int
			TString* obj = ((TString*)(run_context.GetObject().get()));
			int  result = 
			obj->length();
			*(int*)run_context.GetResult().get() = result;
		}
	}
	namespace ns_TPtr 
	{
		void bind_def_constr(TMethodRunContext& run_context) 
		{
			// return --- nothing
			TPtr* obj = ((TPtr*)(run_context.GetObject().get()));
			obj->def_constr(run_context);
		}
		void bind_copy_constr(TMethodRunContext& run_context) 
		{
			//TPtr&  copy_from return --- nothing
			TPtr* obj = ((TPtr*)(run_context.GetObject().get()));
			TPtr*  param0 = ((TPtr*)run_context.GetFormalParams()[0].get());
			obj->copy_constr(run_context, param0);
		}
		void bind_destructor(TMethodRunContext& run_context) 
		{
			// return --- nothing
			TPtr* obj = ((TPtr*)(run_context.GetObject().get()));
			obj->destructor(run_context);
		}
		void bind_operator_Assign(TMethodRunContext& run_context) 
		{
			//static - TPtr&  v, TPtr&  l return --- nothing
			TPtr*  param0 = ((TPtr*)run_context.GetFormalParams()[0].get());
			TPtr*  param1 = ((TPtr*)run_context.GetFormalParams()[1].get());
			TPtr::operator_Assign(run_context, param0, param1);
		}
		void bind_data(TMethodRunContext& run_context) 
		{
			// return --- void*&
			TPtr* obj = ((TPtr*)(run_context.GetObject().get()));
			void*  result = 
			obj->data(run_context);
			run_context.GetResult().SetAsReference(result);
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
		result.push_back(ns_Script::ns_TPtr::bind_def_constr);
		result.push_back(ns_Script::ns_TPtr::bind_copy_constr);
		result.push_back(ns_Script::ns_TPtr::bind_destructor);
		result.push_back(ns_Script::ns_TPtr::bind_operator_Assign);
		result.push_back(ns_Script::ns_TPtr::bind_data);
		return result;
	}
	const int BindingOffset = 0;
	const int BindingCount = 22;
}
