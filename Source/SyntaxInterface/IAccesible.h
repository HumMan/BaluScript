#pragma once

namespace SyntaxApi
{
	enum class TTypeOfAccess
	{
		Private,
		Protected,
		Public
	};

	class IAccessible
	{
	public:
		virtual TTypeOfAccess GetAccess()const=0;
	};
}