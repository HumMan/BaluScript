#pragma once

#include <string>
#include <assert.h>
#include <vector>

namespace Lexer
{

	//TODO русские буквы непереваривает
	enum class TTokenType
	{
		ResWord,
		Operator,
		Value,
		Identifier,
		Bytecode,
		Done,

		LBracket,
		RBracket,
		LParenth,
		RParenth,
		LBrace,
		RBrace,
		Semicolon,
		Colon,
		Vertical,
		Question,
		Comma,
		Ampersand,
		Dot
	};

	enum class TResWord
	{
		If,
		Else,
		For,
		While,
		Class,
		Enum,
		Sealed,
		Return,
		True,
		False,
		Static,
		Extern,
		Private,
		Protected,
		Public,
		Readonly,
		This,
		Bytecode,
		Multifield,
		New,

		Func,
		/*Constr,*/
		Default,
		Copy,
		Move,
		Destr,
		Operator,
		Conversion,

		End //используется для получения количества зарезервированных слов
	};

	enum class TValue
	{
		Int,
		Float,
		Bool,
		String,
		Char
	};

	enum class TOperator
	{
		Plus,
		Minus,
		Mul,
		Div,
		Percent,

		And,
		Or,
		Not,   //унарное логическое не

		Assign,
		PlusA,
		MinusA,
		MulA,
		DivA,
		PercentA,
		AndA,
		OrA,

		Less,
		LessEqual,
		Equal,
		NotEqual,
		Greater,
		GreaterEqual,

		ParamsCall,
		GetArrayElement,
		GetByReference, //оператор -> доступа к значению по указателю (shared_ptr)

		UnaryMinus, //унарный минус
		Increment, //префиксный инкремент
		Decrement, //префиксный декремент

		End //используется для получения количества операторов
	};

	class TNameId
	{
		friend class TLexerPrivate;
	private:
		int id;
#ifdef _DEBUG
		std::string* s;
		TNameId(int use_id, std::string* use_s)
		{
			id = use_id;
			s = use_s;
		}
#else
		TNameId(int use_id)
		{
			id=use_id;
		}
#endif
	public:
		TNameId()
		{
			id = -1;
		}
		TNameId(const TNameId& use_id)
		{
			id = use_id.id;
#ifdef _DEBUG
			s = use_id.s;
#endif
		}
		int GetId()const
		{
			return id;
		}
		bool operator==(TNameId right)const
		{
			assert(!(id == -1 || right.id == -1));
			return id == right.id;
		}
		bool operator!=(TNameId right)const
		{
			return id != right.id;
		}
		bool IsNull()const
		{
			assert(id >= -1);
			return id == -1;
		}
	};

	struct TToken
	{
		TTokenType type;
		short token;
		union
		{
			int int_attrib;
			float float_attrib;
			int identifier;
		};
		int line, col;
		TToken(){}

		TToken(TTokenType use_type) :type(use_type){}
		TToken(TTokenType use_type, short use_token) :type(use_type), token(use_token){}
		TToken(TTokenType use_type, short use_token, int use_att) :type(use_type), token(use_token), int_attrib(use_att){}
		TToken(TTokenType use_type, short use_token, float use_att) :type(use_type), token(use_token), float_attrib(use_att){}
		TToken(TTokenType use_type, short use_token, TNameId use_identifier)
			:type(use_type), token(use_token), identifier(use_identifier.GetId()){}

		void SetLineCol(int use_line, int use_col)
		{
			line = use_line;
			col = use_col;
		}
		bool operator==(const TToken& v)const
		{
			return type == v.type&&token == v.token;
		}
	};

	class ILexer
	{
	public:
		virtual int GetCurrentToken() = 0;
		virtual void ParseSource(const char* use_source) = 0;
		virtual void Error(char* s, int token_id = -1, va_list args = NULL) = 0;
		virtual void SetCurrentToken(int use_curr_token) = 0;
		virtual TNameId NameId() = 0;
		//TODO не должно использоваться, только через lexer по name_id (т.к. много одинаковых)
		virtual std::string StringValue() = 0;
		virtual TNameId String() = 0;
		virtual float Float() = 0;
		virtual int Int() = 0;
		virtual bool Bool() = 0;
		virtual char Char() = 0;
		virtual int GetAttrib() = 0;//используется в байткоде чтобы не делать лишних приведений типа
		virtual std::string GetNameFromId(TNameId use_id) = 0;
		virtual TNameId GetIdFromName(const char* use_name) = 0;

		virtual TTokenType Type() = 0;
		virtual int Token() = 0;

		virtual bool Test(TTokenType type) = 0;
		virtual bool Test(TTokenType type, int token) = 0;
		bool Test(TResWord token)
		{
			return Test(TTokenType::ResWord, (short)token);
		}
		bool Test(TOperator token)
		{
			return Test(TTokenType::Operator, (short)token);
		}
		bool Test(TValue token)
		{
			return Test(TTokenType::Value, (short)token);
		}
		
		virtual bool TestAndGet(TTokenType type) = 0;
		virtual bool TestAndGet(TTokenType type, int token) = 0;
		bool TestAndGet(TResWord token)
		{
			return TestAndGet(TTokenType::ResWord, (short)token);
		}
		bool TestAndGet(TOperator token)
		{
			return TestAndGet(TTokenType::Operator, (short)token);
		}
		bool TestAndGet(TValue token)
		{
			return TestAndGet(TTokenType::Value, (short)token);
		}

		virtual void TestToken(TTokenType type) = 0;
		virtual void TestToken(TTokenType type, int token) = 0;
		void TestToken(TResWord token)
		{
			TestToken(TTokenType::ResWord, (short)token);
		}
		void TestToken(TOperator token)
		{
			TestToken(TTokenType::Operator, (short)token);
		}
		void TestToken(TValue token)
		{
			TestToken(TTokenType::Value, (short)token);
		}

		virtual void GetToken(TTokenType type) = 0;
		virtual void GetToken(TTokenType type, int token) = 0;
		void GetToken(TResWord token)
		{
			GetToken(TTokenType::ResWord, (short)token);
		}
		void GetToken(TOperator token)
		{
			GetToken(TTokenType::Operator, (short)token);
		}
		void GetToken(TValue token)
		{
			GetToken(TTokenType::Value, (short)token);
		}

		virtual void GetToken() = 0;
		static ILexer* Create();
		virtual ~ILexer(){};
	};

	class ITokenPos
	{
	public:
		virtual ILexer* GetLexer()const = 0;
		virtual void Error(char* s, ...)const = 0;
	};

	class TTokenPos: public ITokenPos
	{
		int token_id;
		ILexer* source;
	public:
		TTokenPos()
		{
			token_id = -1;
			source = nullptr;
		}
		void InitPos(ILexer* use_source);
		ILexer* GetLexer()const;
		void Error(char* s, ...)const;
	};
}