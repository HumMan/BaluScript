﻿#pragma once

namespace SyntaxApi
{
	class IStatements: public virtual IStatement
	{
	public:
		virtual IStatement* GetStatement(int i)=0;
		virtual int GetStatementsCount()const=0;
	};
}