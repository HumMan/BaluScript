#pragma once

namespace SyntaxApi
{
	class IType
	{
	public:
		virtual std::vector<std::shared_ptr<IType_TClassName>> GetClassNames() const = 0;
	};

	class IType_TTemplateParameter
	{
	public:
		virtual bool IsValue()const=0;
		virtual int GetValue()const = 0;
		virtual IType* GetType()const = 0;
	};

	class IType_TClassName
	{
	public:
		virtual Lexer::TNameId GetName()const=0;
		virtual std::vector<int> GetDimensions()const=0;
		virtual std::vector<std::shared_ptr<SyntaxApi::IType_TTemplateParameter>> GetTemplateParams() const=0;
	};
}