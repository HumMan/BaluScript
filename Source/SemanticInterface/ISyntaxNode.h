
namespace SemanticApi
{
	template<class T>
	class ISSyntaxNode
	{
	public:
		virtual T* GetSyntax()const = 0;
	};

	class ISNodeWithSize
	{
	public:
		virtual size_t GetSize()const = 0;
		virtual bool IsSizeInitialized()const = 0;
	};

	class ISNodeSignatureLinked
	{
	public:
		virtual bool IsSignatureLinked()const = 0;
	};


	class ISNodeBodyLinked
	{
	public:
		virtual bool IsBodyLinked()const = 0;
	};

	class ISNodeWithAutoMethods
	{
	public:
		virtual bool IsAutoMethodsInitialized()const = 0;
	};
}