#include "OverloadedMethod.h"

#include "Method.h"
#include "Statements.h"

using namespace Lexer;

TNameId TOverloadedMethod::GetName()const{
	return name;
}