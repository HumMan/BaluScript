#pragma once

#include <map> 

#include "lexer.h"

#include "SemanticInterface/SemanticTreeApi.h"

namespace BindingGen
{
	BALUSCRIPT_DLL_INTERFACE void Generate(std::string source, 
		std::map<std::string, std::string> script_class_to_c_map,
		//TODO временный список классов представляющих указатель (применяется доп разименовывание)
		std::vector<std::string> interface_script_class,
		std::vector<std::string>& result, int offset,
		std::vector<SemanticApi::TExternalClassDecl> _external_classes,
		std::vector<SemanticApi::TExternalSMethod> _external_bindings);
}
