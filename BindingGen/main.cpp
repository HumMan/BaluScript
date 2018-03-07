
#include "../Include/baluScript.h"

#include <fstream>
#include <streambuf>

#include "../Source/BindingGen.h"

std::map<std::string, std::string> StoC_map = {
	{ "TDynArray","TDynArr" },
	{ "TStaticArray","TStaticArr" },
	{ "string","TString" },
	{ "vec2i", "BaluLib::TVec2i" },
	{ "vec2", "BaluLib::TVec2" },
};

int main(int argc, char* argv[])
{
	try
	{
		std::vector<std::string> result;
		std::vector<SemanticApi::TExternalClassDecl> _external_classes;
		std::vector<SemanticApi::TExternalSMethod> _external_bindings;
		BindingGen::Generate("", StoC_map, std::vector<std::string>(), result, 0, _external_classes, _external_bindings);

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
