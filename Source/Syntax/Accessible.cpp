#include "Accessible.h"

using namespace Lexer;

TAccessible::TAccessible()
	:access(TTypeOfAccess::Public)
{
}

TTypeOfAccess TAccessible::GetAccess()const
{
	return access;
}

void TAccessible::SetAccess(TTypeOfAccess use_access){
	access = use_access;
}