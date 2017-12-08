#include "Accessible.h"

using namespace Lexer;
using namespace SyntaxInternal;

TAccessible::TAccessible()
	:access(SyntaxApi::TTypeOfAccess::Public)
{
}

SyntaxApi::TTypeOfAccess TAccessible::GetAccess()const
{
	return access;
}

void TAccessible::SetAccess(SyntaxApi::TTypeOfAccess use_access){
	access = use_access;
}

void TMultifield::SetFactorId(Lexer::TNameId id)
{
	assert(factor_id.IsNull());
	factor_id = id;
}
Lexer::TNameId TMultifield::GetFactorId()
{
	assert(!factor_id.IsNull());
	return factor_id;
}
bool TMultifield::HasSizeMultiplierId()
{
	return !factor_id.IsNull();
}
TMultifield::TMultifield()
{
	size_multiplier = -1;
	size_multiplier_set = false;
}
void TMultifield::SetSizeMultiplier(int mul)
{
	assert(!size_multiplier_set);
	size_multiplier = mul;
	size_multiplier_set = true;
}
bool TMultifield::HasSizeMultiplier()
{
	return size_multiplier_set;
}
int TMultifield::GetSizeMultiplier()
{
	return size_multiplier;
}
