#pragma once

#include "../SyntaxTreeApi.h"

namespace SyntaxInternal
{
	
	class TAccessible : public virtual SyntaxApi::IAccessible
	{
	private:
		SyntaxApi::TTypeOfAccess access;
	public:
		TAccessible();
		SyntaxApi::TTypeOfAccess GetAccess()const;
		void SetAccess(SyntaxApi::TTypeOfAccess use_access);
	};

	class TMultifield: public virtual SyntaxApi::IMultiField
	{
		int size_multiplier;
		bool size_multiplier_set;
		Lexer::TNameId factor_id;
	public:
		void SetFactorId(Lexer::TNameId id);
		Lexer::TNameId GetFactorId();
		bool HasSizeMultiplierId();
		TMultifield();
		void SetSizeMultiplier(int mul);
		bool HasSizeMultiplier();
		int GetSizeMultiplier();
	};
}