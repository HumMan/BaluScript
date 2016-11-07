#pragma once

#include <string>
#include <assert.h>
#include <vector>

namespace Lexer
{

	//TODO русские буквы непереваривает
	namespace TTokenType
	{
		enum Enum
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
	}

	namespace TResWord
	{
		enum _Enum
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
	}
	namespace TValue
	{
		enum Enum
		{
			Int,
			Float,
			Bool,
			String,
			Char
		};
	}

	namespace TOperator
	{
		enum Enum
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

			UnaryMinus,

			End //используется для получения количества операторов
		};
	}

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
		short type;
		short token;
		union
		{
			int int_attrib;
			float float_attrib;
			int identifier;
		};
		int line, col;
		TToken(){}

		TToken(TTokenType::Enum use_type) :type(use_type){}
		TToken(TTokenType::Enum use_type, short use_token) :type(use_type), token(use_token){}
		TToken(TTokenType::Enum use_type, short use_token, int use_att) :type(use_type), token(use_token), int_attrib(use_att){}
		TToken(TTokenType::Enum use_type, short use_token, float use_att) :type(use_type), token(use_token), float_attrib(use_att){}
		TToken(TTokenType::Enum use_type, short use_token, TNameId use_identifier)
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
		virtual TTokenType::Enum Type() = 0;
		virtual int Token() = 0;
		virtual bool Test(int type) = 0;
		virtual bool Test(int type, int token) = 0;
		virtual bool TestAndGet(int type, int token) = 0;
		virtual bool TestAndGet(int type) = 0;
		virtual void TestToken(int type, int token) = 0;
		virtual void TestToken(int type) = 0;
		virtual void GetToken(int type, int token) = 0;
		virtual void GetToken(int type) = 0;
		virtual void GetToken() = 0;
		static ILexer* Create();
		virtual ~ILexer(){};
	};

	class TTokenPos
	{
		int token_id;
	public:
		ILexer* source;
		void InitPos(ILexer* use_source);
		void Error(char* s, ...);
	};

}