#pragma once

namespace SyntaxApi
{
	class IBytecode: public virtual IStatement
	{
	public:
		virtual const std::vector<TOpcode::Enum>& GetBytecode()const=0;
	};
}