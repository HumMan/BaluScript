#pragma once

#include <map> 

#include "lexer.h"

#include "SemanticInterface/SemanticTreeApi.h"

namespace BindingGen
{
	class TTypeConverterInfo
	{
	public:
		std::string in_converter;
		std::string out_converter;
		std::string result_type;
	};

	class TBindingGenInfo
	{
	public:
		std::map<std::string, std::string> script_class_to_c_map;
		std::vector<std::string> interface_script_class;
		std::vector<std::string> shared_ptr_script_class;
		std::vector<SemanticApi::TExternalClassDecl> external_classes;
		std::vector<SemanticApi::TExternalSMethod> external_bindings;
		std::map<std::string, TTypeConverterInfo> type_converters;
		size_t bindings_offset;
	};

	BALUSCRIPT_DLL_INTERFACE void Generate(std::string source, std::vector<std::string>& result, TBindingGenInfo info);
}
