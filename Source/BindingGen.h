#pragma once

#include <map> 

#include "lexer.h"

namespace BindingGen
{
	BALUSCRIPT_DLL_INTERFACE void Generate(std::string source, 
		std::map<std::string, std::string> script_class_to_c_map,
		//TODO ��������� ������ ������� �������������� ��������� (����������� ��� ���������������)
		std::vector<std::string> interface_script_class,
		std::vector<std::string>& result, int offset);
}
