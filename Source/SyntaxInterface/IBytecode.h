#pragma once

namespace SyntaxApi
{
	enum class TOpParamType
	{
		VALUE,
		GET_ARR_ELEMENT_CLASS_ID
	};

	struct TBytecodeOp
	{		
		TOpParamType f[2];
		Lexer::TNameId id[2];
		TOp op;
		TBytecodeOp()
		{
			f[0] = TOpParamType::VALUE;
			f[1] = TOpParamType::VALUE;
		}
	};

	class IBytecode: public virtual IStatement
	{
	public:
		virtual const std::vector<SyntaxApi::TBytecodeOp>& GetBytecode()const=0;
	};
}