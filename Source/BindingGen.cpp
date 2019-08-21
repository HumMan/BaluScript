#include "BindingGen.h"

#include "../Include/baluScript.h"

#include <fstream>
#include <streambuf>

#include "../Source/SemanticInterface/SemanticTreeApi.h"

#include <algorithm>

using namespace BindingGen;

Lexer::ILexer* lexer;

TBindingGenInfo binding_gen_info;

class TMethodBindInfo
{
public:
	std::string path;
};

std::vector<TMethodBindInfo> bindings;

std::map<Lexer::TOperator, std::string> operators_map = {
	{ Lexer::TOperator::Plus, "Plus" },
	{ Lexer::TOperator::Minus, "Minus" },
	{ Lexer::TOperator::Mul, "Mul" },
	{ Lexer::TOperator::Div, "Div" },
	{ Lexer::TOperator::Percent, "Percent" },
	{ Lexer::TOperator::And, "And" },
	{ Lexer::TOperator::Or, "Or" },
	{ Lexer::TOperator::Not, "Not" },
	{ Lexer::TOperator::Assign, "Assign" },
	{ Lexer::TOperator::PlusA, "PlusA" },
	{ Lexer::TOperator::MinusA, "MinusA" },
	{ Lexer::TOperator::MulA, "MulA" },
	{ Lexer::TOperator::DivA, "DivA" },
	{ Lexer::TOperator::PercentA, "PercentA" },
	{ Lexer::TOperator::AndA, "AndA" },
	{ Lexer::TOperator::OrA, "OrA" },
	{ Lexer::TOperator::Less, "Less" },
	{ Lexer::TOperator::LessEqual, "LessEqual" },
	{ Lexer::TOperator::Equal, "Equal" },
	{ Lexer::TOperator::NotEqual, "NotEqual" },
	{ Lexer::TOperator::Greater, "Greater" },
	{ Lexer::TOperator::GreaterEqual, "GreaterEqual" },
	{ Lexer::TOperator::ParamsCall, "ParamsCall" },
	{ Lexer::TOperator::GetArrayElement, "GetArrayElement" },
	{ Lexer::TOperator::GetByReference,  "GetByReference" },
	{ Lexer::TOperator::UnaryMinus, "UnaryMinus" },
	{ Lexer::TOperator::Increment, "Increment" },
	{ Lexer::TOperator::Decrement,  "Decrement" }
};

void Line(std::string value, int curr_level, std::vector<std::string>& result)
{
	std::string r;
	for (int i = 0; i < curr_level; i++)
		r += "\t";
	result.push_back(r + value);
}

std::string ToS(Lexer::TNameId id)
{
	return lexer->GetNameFromId(id);
}

bool IsInterfaceClass(std::string v)
{
	return std::find(
        binding_gen_info.interface_script_class.begin(),
        binding_gen_info.interface_script_class.end(),
        v) != binding_gen_info.interface_script_class.end();
}

bool IsSharedPtrClass(std::string v)
{
	return std::find(
		binding_gen_info.shared_ptr_script_class.begin(),
		binding_gen_info.shared_ptr_script_class.end(),
		v) != binding_gen_info.shared_ptr_script_class.end();
}

std::string GetNamespace(SemanticApi::ISMethod* method)
{
	auto curr_class = method->GetOwner();
	std::string result;
	while (curr_class != nullptr)
	{
		result = "ns_" + ToS(curr_class->IGetSyntax()->GetName()) + "::" + result;
		curr_class = curr_class->GetOwner();
	}
	return result;
}


std::string StoC(std::string v)
{
	if (binding_gen_info.script_class_to_c_map.find(v) != binding_gen_info.script_class_to_c_map.end())
		return binding_gen_info.script_class_to_c_map[v];
	else
		return v;
}

std::string GenMethodName(SemanticApi::ISMethod* method)
{
	switch (method->GetType())
	{
	case SemanticApi::SpecialClassMethodType::NotSpecial:
		return ToS(method->IGetSyntax()->GetName());
	case SemanticApi::SpecialClassMethodType::AutoDefConstr:
		assert(false);
		break;
	case SemanticApi::SpecialClassMethodType::AutoCopyConstr:
		assert(false);
		break;
	case SemanticApi::SpecialClassMethodType::AutoDestructor:
		assert(false);
		break;
	case SemanticApi::SpecialClassMethodType::AutoAssignOperator:
		assert(false);
		break;
	case SemanticApi::SpecialClassMethodType::DefaultConstr:
		return "def_constr";
	case SemanticApi::SpecialClassMethodType::CopyConstr:
		return "copy_constr";
	case SemanticApi::SpecialClassMethodType::Destructor:
		return "destructor";
	case SemanticApi::SpecialClassMethodType::Operator:
	{
		auto type = method->GetOperatorType();
		return "operator_" + operators_map[type];
	}
	case SemanticApi::SpecialClassMethodType::Conversion:
		return "conversion";
	default:
		assert(false);
		break;
	}
	return "invalid_method!";
}

std::string DeclParameters(SemanticApi::ISMethod* method)
{
	std::string result;
	auto count = method->GetParamsCount();
	for (size_t i = 0; i < count; i++)
	{
		auto param = method->GetParam(i);
		auto is_ref = param->IsRef();
		//
		if (param->GetClass()->GetType() == SemanticApi::TNodeWithTemplatesType::SurrogateTemplateParam)
		{
			std::string type = "void*";
			result += type + (is_ref ? "&" : "") + " ";
		}
		else
		{
			auto type = StoC(ToS(param->GetClass()->IGetSyntax()->GetName()));
			result += type + (is_ref ? "&" : "") + " ";
		}
		result += " " + ToS(param->IGetSyntax()->GetName());
		if (i != count - 1)
			result += ", ";

	}
	result += " return --- ";

	auto ret_class = method->GetRetClass();
	if (ret_class != nullptr)
	{
		if (ret_class->GetType() == SemanticApi::TNodeWithTemplatesType::SurrogateTemplateParam)
		{
			std::string type = "void*";
			result += type + (method->IsReturnRef() ? "&" : "");
		}
		else
		{
			auto type = StoC(ToS(ret_class->IGetSyntax()->GetName()));
			result += type + (method->IsReturnRef() ? "&" : "");
		}
	}
	else
	{
		result += "nothing";
	}

	return result;
}


std::string GetParamType(SemanticApi::ISParameter* param, bool& is_interface_type, bool& is_shared_ptr_type)
{
	is_interface_type = false;
	is_shared_ptr_type = false;
	std::string type;
	if (param->GetClass()->GetType() == SemanticApi::TNodeWithTemplatesType::SurrogateTemplateParam)
	{
		type = "void*";
	}
	else
	{
		auto script_type = ToS(param->GetClass()->IGetSyntax()->GetName());
		type = StoC(script_type);
		if (IsInterfaceClass(script_type))
		{
			is_interface_type = true;
			type = type + "*";
		}
		else if (IsSharedPtrClass(script_type))
		{
			is_shared_ptr_type = true;
		}
	}
	return type;
}

std::string callScriptFromC_DeclParameters(SemanticApi::ISMethod* method)
{
	std::string result;
	auto count = method->GetParamsCount();

	result += "TGlobalRunContext global_context, SemanticApi::ISMethod* compiled_method, ISyntaxAnalyzer* syntax";
	if (count>0)
		result += ", ";

	for (size_t i = 0; i < count; i++)
	{
		auto param = method->GetParam(i);
		auto name = "p" + std::to_string(i);
		auto is_ref = param->IsRef();

		if (param->GetClass()->GetType() == SemanticApi::TNodeWithTemplatesType::SurrogateTemplateParam)
		{
			std::string type = "void*";
			result += type + (is_ref ? "&" : "") + " " + name;
		}
		else
		{
			bool is_interface_type;
			bool is_shared_ptr_type;
			auto type = GetParamType(param, is_interface_type, is_shared_ptr_type);
			if(is_interface_type)
				result += type + " const " + name;
			else if(is_shared_ptr_type)
				result += "std::shared_ptr<"+ type + "> const &" + name;
			else
				result += (is_ref ? "" : "const ") + type + (is_ref ? "&" : "") + " " + name;

			result += " /*" + ToS(param->IGetSyntax()->GetName()) + "*/";
		}
		if (i != count - 1)
			result += ", ";

	}
	auto ret_class = method->GetRetClass();
	if (ret_class != nullptr)
		throw std::runtime_error("Генерация вызова из Си поддерживается только для методов без возврата значения");

	return result;
}


std::string RunParams(bool is_template, size_t count)
{
	std::string result;

	if (is_template)
	{
		result += "run_context";
		if (count>0)
			result += ", ";
	}

	for (size_t i = 0; i < count; i++)
	{
		result += "param" + std::to_string(i);
		if (i != count - 1)
			result += ", ";
	}
	return result;
}

std::string GetRetType(SemanticApi::ISMethod* method, bool& is_share_ptr_type)
{
	is_share_ptr_type = false;
	auto ret_class = method->GetRetClass();
	auto script_type = ToS(ret_class->IGetSyntax()->GetName());
	auto type = StoC(script_type);
	if (IsInterfaceClass(script_type))
		type = type + "*";
	if (IsSharedPtrClass(script_type))
		is_share_ptr_type = true;
	return type;
}

bool NeedTypeConverter(std::string type, TTypeConverterInfo& info)
{
	auto it = binding_gen_info.type_converters.find(type);
	if (it != binding_gen_info.type_converters.end())
	{
		info = (*it).second;
		return true;
	}
	else
		return false;
}

void DeclBody(SemanticApi::ISMethod* method, std::vector<std::string>& result, int curr_level)
{
	auto is_template = method->GetOwner()->GetType() == SemanticApi::TNodeWithTemplatesType::Template;

	auto is_static = method->IsStatic();

	auto obj_type_script = ToS(method->GetOwner()->IGetSyntax()->GetName());

	bool is_obj_type_interface = IsInterfaceClass(obj_type_script);
	bool is_shared_ptr_type = IsSharedPtrClass(obj_type_script);

	std::string obj_type = StoC(obj_type_script);

	if ((!is_static) && is_shared_ptr_type)
	{
		Line("TScriptSharedPointer<"+obj_type+">* obj = ((TScriptSharedPointer<"+ obj_type+">*)(run_context->object->get()));\n", curr_level, result);
	}
	else
	{
		if ((!is_static) && is_obj_type_interface)
		{
			obj_type = obj_type + "*";
		}
		if (!is_static)
		{
			Line(obj_type + "* obj = ((" + obj_type + "*)(run_context->object->get()));\n", curr_level, result);
		}
	}

	auto count = method->GetParamsCount();
	for (size_t i = 0; i < count; i++)
	{
		auto param = method->GetParam(i);
		auto is_ref = param->IsRef();
		bool is_interface_type;
		bool is_shared_ptr_type;
		auto type = GetParamType(param, is_interface_type, is_shared_ptr_type);

		assert(!(is_ref && is_interface_type));

		TTypeConverterInfo convert_info;
		bool need_convert = NeedTypeConverter(type, convert_info);

		assert(!(need_convert&&is_ref));

		if (is_interface_type)
		{
			assert(!is_ref);
			Line(type +
				(" param") + std::to_string(i) +
				" = " +
				("*") +
				"((" + type + "*)(*run_context->formal_params)[" + std::to_string(i) + "].get());\n", curr_level, result);
		}
		else if (is_shared_ptr_type)
		{
			Line("std::shared_ptr<"+type +">"+
				(" param") + std::to_string(i) +
				" = " +
				("*") +
				"((TScriptSharedPointer<" + type + ">*)(*run_context->formal_params)[" + std::to_string(i) + "].get())->v;\n", curr_level, result);
		}
		else if (param->GetClass()->GetType() == SemanticApi::TNodeWithTemplatesType::Template)
		{
			Line(type + "* " +
				(need_convert ? "temp_param" : " param") + std::to_string(i) +
				" = " +
				"((" + type + "*)(*run_context->formal_params)[" + std::to_string(i) + "].get());\n", curr_level, result);
		}
		else if (param->GetClass()->GetType() == SemanticApi::TNodeWithTemplatesType::SurrogateTemplateParam)
		{
			//TODO
			assert(false);
		}
		else
		{
			Line((is_ref ? type + "&" : "const " + type + "& ") +
				(need_convert ? "temp_param" : " param") + std::to_string(i) +
				" = " +
				("*") +
				"((" + type + "*)(*run_context->formal_params)[" + std::to_string(i) + "].get());\n", curr_level, result);
		}

		if (need_convert)
		{
			Line(convert_info.result_type+" param"+std::to_string(i)+" = "+
				convert_info.in_converter+"(temp_param"+ std::to_string(i) +");\n", curr_level, result);
		}
	}

	auto ret_class = method->GetRetClass();

	auto has_ret = ret_class != nullptr;
		
	if (has_ret)
	{	
		if (ret_class->GetType() == SemanticApi::TNodeWithTemplatesType::SurrogateTemplateParam)
		{
			std::string type = "void*";
			Line(type + " " + " result = \n", curr_level, result);
		}
		else
		{
			bool is_shared_ptr_type;
			std::string type = GetRetType(method, is_shared_ptr_type);
			if (is_shared_ptr_type)
			{
				assert(!method->IsReturnRef());

				Line("std::shared_ptr<" + type + "> result =\n", curr_level, result);
			}
			else
			{
				TTypeConverterInfo convert_info;
				bool need_convert = NeedTypeConverter(type, convert_info);

				assert(!(need_convert&&method->IsReturnRef()));

				if (need_convert)
				{
					Line(convert_info.result_type + " " + "temp_result = \n", curr_level, result);
				}
				else
				{
					Line(type + " " + (method->IsReturnRef() ? "&" : "") + (need_convert ? "temp_result" : " result") + " = \n", curr_level, result);
				}
			}
		}
	}

	//вызов метода с параметрами
	Line((is_static ? std::string(obj_type + "::") : ((is_obj_type_interface || is_shared_ptr_type) ? std::string("(*obj)->") : std::string("obj->")))
		+ GenMethodName(method) +
		"(" + RunParams(is_template, count) + ");\n", curr_level, result);

	//запись результата
	if (has_ret)
	{		
		if (ret_class->GetType() == SemanticApi::TNodeWithTemplatesType::SurrogateTemplateParam)
		{
			if (method->IsReturnRef())
				Line("run_context->result->SetAsReference(result);\n", curr_level, result);
			else
				throw std::runtime_error("Не поддерживается возврат шаблонного параметра по значению из extern");
		}
		else
		{
			bool is_shared_ptr_type;
			std::string type = GetRetType(method, is_shared_ptr_type);
			if (is_shared_ptr_type)
			{
				assert(!method->IsReturnRef());
				Line("*(TScriptSharedPointer<" + type + ">*)run_context->result->get() = TScriptSharedPointer<" + type + ">(result);\n", curr_level, result);
			}
			else
			{
				if (method->IsReturnRef())
					Line("run_context->result->SetAsReference(&result);\n", curr_level, result);
				else
				{
					TTypeConverterInfo convert_info;
					bool need_convert = NeedTypeConverter(type, convert_info);

					if (need_convert)
					{
						Line(type + " result = " +
							convert_info.out_converter + "(temp_result);\n", curr_level, result);
					}

					Line("*(" + type + "*)run_context->result->get() = result;\n", curr_level, result);
				}
			}
		}		
	}
}


void callScriptFromC_DeclBody(SemanticApi::ISMethod* method, std::vector<std::string>& result, int curr_level)
{
	auto is_template = method->GetOwner()->GetType() == SemanticApi::TNodeWithTemplatesType::Template;

	auto is_static = method->IsStatic();
	if (!is_static)
		throw std::runtime_error("Генерация вызова из Си поддерживается только для static методов");

	Line("std::vector<TStackValue> params;\n", curr_level, result);

	auto count = method->GetParamsCount();
	for (size_t i = 0; i < count; i++)
	{
		auto param = method->GetParam(i);
		auto is_ref = param->IsRef();

		std::string type = ToS(param->GetClass()->IGetSyntax()->GetName());
		bool is_interface_type;
		bool is_shared_ptr_type;
		auto typeC = GetParamType(param, is_interface_type, is_shared_ptr_type);

		//TODO заменить на сквозную индексацию класса, т.к. порядок меняться не будет
		Line(std::string("params.push_back(TStackValue(") + (is_ref ? "true" : "false") +
			", syntax->GetCompiledBaseClass()->GetClass(syntax->GetLexer()->GetIdFromName(\"" + type + "\"))));\n", curr_level, result);
		if (is_shared_ptr_type)
		{
			Line(std::string("*(TScriptSharedPointer<") + typeC + ">*" + ")params[" + std::to_string(i) + "].get() = " +
				"TScriptSharedPointer<"+ typeC +">(p" + std::to_string(i) + ");\n", curr_level, result);
		}
		else
		{
			Line(std::string("*(") + typeC + "*" + (is_ref ? "*" : "") + ")params[" + std::to_string(i) + "].get() = " +
				(is_ref ? "&" : "") + "p" + std::to_string(i) + ";\n", curr_level, result);
		}
	}

	Line("TStackValue result, object;\n", curr_level, result);
	Line("TreeRunner::Run(compiled_method, TMethodRunContext(global_context, &params, &result, &object));\n", curr_level, result);

	for (size_t i = 0; i < count; i++)
	{
		auto param = method->GetParam(i);
		auto is_ref = param->IsRef();

		std::string type = ToS(param->GetClass()->IGetSyntax()->GetName());
		bool is_interface_type;
		bool is_shared_ptr_type;
		auto typeC = GetParamType(param, is_interface_type, is_shared_ptr_type);
		auto destr = param->GetClass()->GetDestructor();
		if (!is_ref&&destr != nullptr)
		{
			Line("{\n", curr_level, result);
			Line("TStackValue destructor_result;\n", curr_level+1, result);
			Line("std::vector<TStackValue> without_params;\n", curr_level + 1, result);
			Line("TreeRunner::Run(params["+ std::to_string(i) +"].GetClass()->GetDestructor(), TMethodRunContext(global_context, &without_params, &destructor_result, &params[" + std::to_string(i) + "]));\n", curr_level + 1, result);
			Line("}\n", curr_level, result);
		}
	}
}

void DeclMethod(SemanticApi::ISMethod* method, std::vector<std::string>& result, int curr_level)
{
	if (method == nullptr)
		return;

	if (method->IsExternal())
	{
		//генерируем код для вызова метода с++ из скрипта
		std::string method_name = "bind_" + GenMethodName(method);

		TMethodBindInfo info;
		info.path = GetNamespace(method) + method_name;

		bindings.push_back(info);

		if (bindings.size() > binding_gen_info.bindings_offset)
		{
			auto obj_type_script = ToS(method->GetOwner()->IGetSyntax()->GetName());
			bool is_obj_type_interface = IsInterfaceClass(obj_type_script);
			bool is_obj_type_shared_ptr = IsSharedPtrClass(obj_type_script);

			auto c_type = StoC(obj_type_script);

			Line("void " + method_name + "(TMethodRunContext* run_context) \n", curr_level, result);
			Line("{\n", curr_level, result);
			Line(std::string("//") + (method->IsStatic() ? "static - " : "") + DeclParameters(method) + "\n", curr_level + 1, result);

			//для интерфейсных классов генерируем спец методы
			if (is_obj_type_shared_ptr && method_name == "bind_def_constr")
			{
				Line("TScriptSharedPointer<" + c_type + ">* obj = ((TScriptSharedPointer<" + c_type + ">*)(run_context->object->get()));\n", curr_level + 1, result);
				Line("obj->def_constr();\n", curr_level + 1, result);
			}
			else if (is_obj_type_shared_ptr && method_name == "bind_copy_constr")
			{
				Line("TScriptSharedPointer<" + c_type + ">* obj = ((TScriptSharedPointer<" + c_type + ">*)(run_context->object->get()));\n", curr_level+1, result);
				Line("TScriptSharedPointer<" + c_type + ">* param0 = ((TScriptSharedPointer<" + c_type + ">*)(*run_context->formal_params)[0].get());\n", curr_level + 1, result);
				Line("obj->copy_constr(param0);\n", curr_level + 1, result);
			}
			else if (is_obj_type_shared_ptr && method_name == "bind_destructor")
			{
				Line("TScriptSharedPointer<" + c_type + ">* obj = ((TScriptSharedPointer<" + c_type + ">*)(run_context->object->get()));\n", curr_level + 1, result);
				Line("obj->destructor();\n", curr_level + 1, result);
			}
			else if (is_obj_type_shared_ptr && method_name == "bind_operator_Assign")
			{
				Line("TScriptSharedPointer<" + c_type + ">* param0 = ((TScriptSharedPointer<" + c_type + ">*)(*run_context->formal_params)[0].get());\n", curr_level + 1, result);
				Line("TScriptSharedPointer<" + c_type + ">* param1 = ((TScriptSharedPointer<" + c_type + ">*)(*run_context->formal_params)[1].get());\n", curr_level + 1, result);
				Line("param0->operator_Assign(param1);\n", curr_level + 1, result);
			}
			else
			{
				DeclBody(method, result, curr_level + 1);
			}
			Line("}\n", curr_level, result);
		}
	}
	else
	{
		//генерируем код для вызова метода скрипта из с++
		//поддерживается только static без возврата результата
		if (method->GetRetClass() == nullptr && method->IsStatic() && method->GetOwner()->GetType() == SemanticApi::TNodeWithTemplatesType::Class)
		{
			std::string method_name = "callScriptFromC_" + GenMethodName(method);

			TMethodBindInfo info;
			info.path = GetNamespace(method) + method_name;

			Line("void " + method_name + "(" + callScriptFromC_DeclParameters(method) + ") \n", curr_level, result);
			Line("{\n", curr_level, result);
			callScriptFromC_DeclBody(method, result, curr_level + 1);
			Line("}\n", curr_level, result);
		}
	}
}

void DeclClass(SemanticApi::ISClass* scl, std::vector<std::string>& result, int curr_level)
{
	DeclMethod(scl->GetDefConstr(), result, curr_level);

	std::vector<SemanticApi::ISMethod*> m;
	m.clear();
	scl->GetCopyConstructors(m);
	if (m.size() > 0)
		DeclMethod(m[0], result, curr_level);

	DeclMethod(scl->GetDestructor(), result, curr_level);

	for (int i = 0; i < (int)Lexer::TOperator::End; i++)
	{
		m.clear();
		scl->GetOperators(m, (Lexer::TOperator)i);
		for (auto v : m)
			DeclMethod(v, result, curr_level);
	}

	m.clear();
	scl->GetMethods(m);
	for (auto v : m)
		DeclMethod(v, result, curr_level);
}

void TreeWalker(SemanticApi::ISClass* curr_class, std::vector<SemanticApi::ISClass*> external_classes, std::vector<std::string>& result, int curr_level)
{
	Line(std::string("namespace ns_") + ToS(curr_class->IGetSyntax()->GetName()) + " \n", curr_level, result);
	Line("{\n", curr_level, result);


	DeclClass(curr_class, result, curr_level + 1);
	auto nested_count = curr_class->GetNestedCount();
	for (size_t i = 0; i < nested_count; i++)
	{
		TreeWalker(curr_class->GetNested(i), external_classes, result, curr_level + 1);
	}

	Line("}\n", curr_level, result);
}

void GenBindingRegistration(std::vector<std::string>& result, int curr_level, int bindings_offset)
{
	Line("namespace ns_Script\n", curr_level, result);
	Line("{\n", curr_level, result);
	curr_level += 1;
	Line("std::vector<SemanticApi::TExternalSMethod> Register()\n", curr_level, result);
	Line("{\n", curr_level, result);
	curr_level += 1;
	Line("std::vector<SemanticApi::TExternalSMethod> result;\n", curr_level, result);
	for (size_t i = 0; i < bindings.size(); i++)
	{
		if (i >= bindings_offset)
		{
			Line("result.push_back(" + bindings[i].path + ");\n", curr_level, result);
		}
	}
	Line("return result;\n", curr_level, result);
	curr_level -= 1;
	Line("}\n", curr_level, result);

	Line("const int BindingOffset = " + std::to_string(bindings_offset) + ";\n", curr_level, result);
	Line("const int BindingCount = " + std::to_string(bindings.size() - bindings_offset) + ";\n", curr_level, result);

	curr_level -= 1;
	Line("}\n", curr_level, result);
}

void BindingGen::Generate(std::string source, std::vector<std::string>& result, TBindingGenInfo info)
{
	TSyntaxAnalyzer syntax;

	bindings.clear();

	binding_gen_info = info;

	syntax.Compile(source.c_str(), binding_gen_info.external_classes, binding_gen_info.external_bindings);

	lexer = syntax.GetLexer();

	std::vector<SemanticApi::ISClass*> external_classes;

	Line("//auto-generated code\n", 0, result);

	TreeWalker(syntax.GetCompiledBaseClass(), external_classes, result, 0);

	GenBindingRegistration(result, 0, binding_gen_info.bindings_offset);
}
