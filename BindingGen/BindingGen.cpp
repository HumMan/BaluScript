
#include "../Include/baluScript.h"

#include <fstream>
#include <streambuf>

#include "../Source/SemanticInterface/SemanticTreeApi.h"

Lexer::ILexer* lexer;

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

std::map<std::string, std::string> StoC_map = {
	{"TDynArray","TDynArr"},
	{ "TStaticArray","TStaticArr" },
	{ "string","TString" },
};

std::map<Lexer::TOperator, std::string> operators_map = {
	{ Lexer::TOperator::Plus, "Plus"},
	{ Lexer::TOperator::Minus, "Minus"},
	{ Lexer::TOperator::Mul, "Mul"},
	{ Lexer::TOperator::Div, "Div"},
	{ Lexer::TOperator::Percent, "Percent"},
	{ Lexer::TOperator::And, "And"},
	{ Lexer::TOperator::Or, "Or"},
	{ Lexer::TOperator::Not, "Not"},
	{ Lexer::TOperator::Assign, "Assign"},
	{ Lexer::TOperator::PlusA, "PlusA"},
	{ Lexer::TOperator::MinusA, "MinusA"},
	{ Lexer::TOperator::MulA, "MulA"},
	{ Lexer::TOperator::DivA, "DivA"},
	{ Lexer::TOperator::PercentA, "PercentA"},
	{ Lexer::TOperator::AndA, "AndA"},
	{ Lexer::TOperator::OrA, "OrA"},
	{ Lexer::TOperator::Less, "Less"},
	{ Lexer::TOperator::LessEqual, "LessEqual"},
	{ Lexer::TOperator::Equal, "Equal"},
	{ Lexer::TOperator::NotEqual, "NotEqual"},
	{ Lexer::TOperator::Greater, "Greater"},
	{ Lexer::TOperator::GreaterEqual, "GreaterEqual"},
	{ Lexer::TOperator::ParamsCall, "ParamsCall"},
	{ Lexer::TOperator::GetArrayElement, "GetArrayElement"},
	{ Lexer::TOperator::GetByReference,  "GetByReference"},
	{ Lexer::TOperator::UnaryMinus, "UnaryMinus"},
	{ Lexer::TOperator::Increment, "Increment"},
	{ Lexer::TOperator::Decrement,  "Decrement" }
};

std::string StoC(std::string v)
{
	if (StoC_map.find(v) != StoC_map.end())
		return StoC_map[v];
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
	case SemanticApi::SpecialClassMethodType::Default:
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
}

std::string DeclParameters(SemanticApi::ISMethod* method)
{
	std::string result;
	auto count = method->GetParamsCount();
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
			auto type = StoC(ToS(param->GetClass()->IGetSyntax()->GetName()));
			result += type + (is_ref ? "&" : "") + " " + name;
		}
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

std::string RunParams(bool is_template, size_t count)
{
	std::string result;

	if (is_template)
	{
		result += "run_context";
		if(count>0)
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

void DeclBody(SemanticApi::ISMethod* method, std::vector<std::string>& result, int curr_level)
{
	auto is_template = method->GetOwner()->GetType() == SemanticApi::TNodeWithTemplatesType::Template;

	auto is_static = method->IsStatic();

	std::string obj_type = StoC(ToS(method->GetOwner()->IGetSyntax()->GetName()));

	if (!is_static)
		Line(obj_type + "* obj = ((" + obj_type + "*)(run_context->object->get()));\n", curr_level, result);

	auto count = method->GetParamsCount();
	for (size_t i = 0; i < count; i++)
	{
		auto param = method->GetParam(i);
		auto is_ref = param->IsRef();
		std::string type;
		if (param->GetClass()->GetType() == SemanticApi::TNodeWithTemplatesType::SurrogateTemplateParam)
		{
			type = "void*";
		}
		else
		{
			type = StoC(ToS(param->GetClass()->IGetSyntax()->GetName()));
		}
		Line((is_ref?type+"*":type) + " param" + std::to_string(i) + 
			" = "+(is_ref?"":"*")+"((" + type + "*)(*run_context->formal_params)[" +
			std::to_string(i) + "].get());\n", curr_level, result);
	}

	auto ret_class = method->GetRetClass();

	auto has_ret = ret_class != nullptr;

	if (has_ret)
	{
		std::string type;
		if (ret_class->GetType() == SemanticApi::TNodeWithTemplatesType::SurrogateTemplateParam)
		{
			type = "void*";
			Line(type + " "+ " result = \n", curr_level, result);
		}
		else
		{
			type = StoC(ToS(ret_class->IGetSyntax()->GetName()));
			Line(type + " " + (method->IsReturnRef() ? "&" : "") + " result = \n", curr_level, result);
		}
	}

	Line((is_static?std::string(obj_type+"::"):std::string("obj->")) + GenMethodName(method) + 
		"(" + RunParams(is_template, count) + ");\n", curr_level, result);

	if (has_ret)
	{
		if (method->IsReturnRef())
			Line("run_context->result->SetAsReference(&result);\n", curr_level, result);
		else
			Line("*("+ StoC(ToS(ret_class->IGetSyntax()->GetName())) +"*)run_context->result->get() = result;\n", curr_level, result);
	}
}

void DeclMethod(SemanticApi::ISMethod* method, std::vector<std::string>& result, int curr_level)
{
	if (method == nullptr || !method->IsExternal())
		return;
	Line("void ex_" + GenMethodName(method) + "(TMethodRunContext* run_context) \n", curr_level, result);
	Line("{\n", curr_level, result);
	Line("//" + DeclParameters(method) + "\n", curr_level + 1, result);
	DeclBody(method, result, curr_level + 1);
	Line("}\n", curr_level, result);
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

int main(int argc, char* argv[])
{
	TSyntaxAnalyzer syntax;
	try
	{
		syntax.Compile("");

		lexer = syntax.GetLexer();

		std::vector<SemanticApi::ISClass*> external_classes;
		std::vector<std::string> result;

		TreeWalker(syntax.GetCompiledBaseClass(), external_classes, result, 0);

		std::ofstream file("../Data/extern_binding.h");
		for (auto v : result)
			file << v;
		file.close();
	}
	catch (std::string s)
	{
		printf(s.c_str());
	}


	printf("All done");
	return 0;
}
