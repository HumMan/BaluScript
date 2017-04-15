#pragma once

namespace SyntaxApi
{
	struct TBytecodeOp
	{
		enum TOpParamType
		{
			VALUE,
			GET_ARR_ELEMENT_CLASS_ID
		};
		TOpParamType f[2];
		Lexer::TNameId id[2];
		TOp op;
		TBytecodeOp()
		{
			f[0] = VALUE;
			f[1] = VALUE;
		}
	};

	class IBytecode: public virtual IStatement
	{
	public:
		virtual const std::vector<SyntaxApi::TBytecodeOp>& GetBytecode()const=0;
	};
}