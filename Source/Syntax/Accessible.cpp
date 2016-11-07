#include "Accessible.h"

using namespace Lexer;

TAccessible::TAccessible()
	:access(TTypeOfAccess::Public)
{
}

TTypeOfAccess::Enum TAccessible::GetAccess()const
{
	return access;
}

void TAccessible::SetAccess(TTypeOfAccess::Enum use_access){
	access = use_access;
}