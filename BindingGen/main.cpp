
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

		BindingGen::TBindingGenInfo binding_gen_info;

		binding_gen_info.bindings_offset = 0;
		binding_gen_info.script_class_to_c_map = StoC_map;

		BindingGen::Generate("", result, binding_gen_info);

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
