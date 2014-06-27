#pragma once

namespace TTypeOfAccess
{
	enum Enum
	{
		Private,
		Protected,
		Public
	};
}

class TAccessible
{
protected:
	TTypeOfAccess::Enum access;
public:
	TAccessible();
	TTypeOfAccess::Enum GetAccess()const;
	void SetAccess(TTypeOfAccess::Enum use_access);
};