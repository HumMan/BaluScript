
#include "../Include/baluScript.h"

#include <fstream>
#include <streambuf>

#include "../Source/SemanticInterface/SemanticTreeApi.h"
#include "../Source/TreeRunner/TreeRunner.h"

#include "../Data/extern_binding.h"

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

void DeclBody(SemanticApi::ISMethod* method, std::vector<std::string>& result, int curr_level)
{
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
		return "operator" + std::to_string((int)type);
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
		auto name = "p"+std::to_string(i);
		auto is_ref = param->IsRef();
		auto type = ToS(param->GetClass()->IGetSyntax()->GetName());

		result += type + (is_ref ? "&" : "") + " " + name;
		if (i != count - 1)
			result += ", ";
	}
	return result;
}

void DeclMethod(SemanticApi::ISMethod* method, std::vector<std::string>& result, int curr_level)
{
	assert(method->IsExternal());

	//TODO не может получить класс
	//auto test = DeclParameters(method);

	Line("void ex_" + GenMethodName(method) + "(TMethodRunContext* run_context) \n", curr_level, result);
	Line("{\n", curr_level, result);
	DeclBody(method, result, curr_level + 1);
	Line("}\n", curr_level, result);
}

void DeclClass(SemanticApi::ISClass* scl, std::vector<std::string>& result, int curr_level)
{
	DeclMethod(scl->GetDefConstr(), result, curr_level);

	std::vector<SemanticApi::ISMethod*> m;
	m.clear();
	scl->GetCopyConstructors(m);
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

	if (curr_class->IsExternal())
	{
		DeclClass(curr_class, result, curr_level+1);
	}
	else
	{
		auto nested_count = curr_class->GetNestedCount();
		for (size_t i = 0; i < nested_count; i++)
		{
			TreeWalker(curr_class->GetNested(i), external_classes, result, curr_level + 1);
		}
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
