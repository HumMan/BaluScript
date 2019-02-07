#include "lexer.h"

#include "BytecodeBaseOps/Op.h"

#include <math.h>


#include <stdarg.h>
#include <string>
#include <assert.h>
#include <vector>

#if defined(WIN32)||defined(_WIN32)
#else
#include <string.h>
template<typename... Args>
int _snprintf_s(char* buf, int len, const char* value, Args... args)
{
	return snprintf(buf, len, value, args...);
}

template<typename... Args>
int _vsnprintf_s(char* buf, int len, const char* value, Args... args)
{
	return vsnprintf(buf, len, value, args...);
}
void strncpy_s(
	char *strDest,
	size_t numberOfElements,
	const char *strSource,
	size_t count)
{
	strcpy(strDest, strSource);
}
#define sprintf_s sprintf
#endif

template<class T, int block_size>
class TLexerAllocator
{
private:
	class TBlock
	{
	public:
		T values[block_size];
		TBlock* prev;
	};
	int count;
	TBlock* end;
#ifdef _DEBUG
	int total_count;
#endif
public:
	TLexerAllocator() :count(0), end(nullptr)
#ifdef _DEBUG
		, total_count(0)
#endif
	{}
	~TLexerAllocator()
	{
		TBlock* curr = end;
		while (curr != nullptr)
		{
			TBlock* temp = curr;
			curr = curr->prev;
			delete temp;
		}
	}
	T* New()
	{
		if (end != nullptr)
		{
			if (count == block_size)
			{
				TBlock* b = new TBlock();
				count = 0;
				b->prev = end;
				end = b;
			}
		}
		else
		{
			end = new TBlock();
			count = 0;
			end->prev = nullptr;
		}
#ifdef _DEBUG
		total_count++;
#endif
		return &end->values[count++];
	}
};

template<class TKey, class TExtKey, class TData, int hash_bits_count = 8>
class THash
{
private:
	enum
	{
		table_size = 1 << hash_bits_count
	};
	class TNode
	{
	public:
		TKey key;
		TData data;
		TNode* next;
	};
	TNode* table[table_size];
	TLexerAllocator<TNode, 255> nodes;

public:
	THash()
	{
		for (size_t i = 0; i < table_size; i++)
			table[i] = nullptr;
	}
	bool Find(int use_hash, TExtKey use_key, TKey* &key, TData* &data)
	{
		assert(use_hash >= 0 && use_hash < table_size);
		if (table[use_hash] != nullptr)
		{
			TNode* curr = table[use_hash];
			do
			{
				if (curr->key == use_key)
				{
					data = &curr->data;
					key = &curr->key;
					return true;
				}
				curr = curr->next;
			} while (curr != nullptr);
		}
		return false;
	}
	bool Add(int use_hash, TExtKey use_key, TData use_data, TKey* &key, TData* &data)
		//use_hash - хеш для доступа к таблице
		//use_key - ключевое поле для сравнений в ¤чейке таблицы
		//use_data - данные которые будут добавлены если ключ отсутствует
		//key - указатель на уже имеющееся ключевое поле
		//data - указатель на уже имеющиеся данные
		//return: true - данные добавлены; false - данные уже имеются
	{
		assert(use_hash >= 0 && use_hash < table_size);
		TNode* curr = table[use_hash];
		TNode* last = nullptr;
		if (curr != nullptr)
		{
			do{
				last = curr;
				if (curr->key == use_key)
				{
					data = &curr->data;
					key = &curr->key;
					return false;
				}
				curr = curr->next;
			} while (curr != nullptr);
		}
		TNode* node = nodes.New();
		node->key = use_key;
		node->data = use_data;
		node->next = nullptr;
		if (table[use_hash] == nullptr)
			table[use_hash] = node;
		else
			last->next = node;
		key = &node->key;
		data = &node->data;
		return true;
	}
	bool Add(TExtKey use_key, TData use_data)
	{
		TKey* key;
		TData* data;
		return Add(GetHash(use_key), use_key, use_data, key, data);
	}
	int GetHash(const char* str)
	{
		assert(hash_bits_count >= 1 && hash_bits_count <= 16);
		unsigned char h1, h2;
		int h;
		assert(str != nullptr);
		h1 = *str;
		if (hash_bits_count > 8)
			h2 = *str + 1;
		str++;
		while (*str)
		{
			h1 = h1 ^ *str;
			if (hash_bits_count > 8)
				h2 = h2 ^ *str;
			str++;
		}
		if (hash_bits_count > 8)
			h = int(h1 << 8) | int(h2);
		else
			h = h1;
		return h % table_size;
	}
};

namespace Lexer
{
	class TToken
	{
	public:
		TTokenType type;
		short token;
		union
		{
			int int_attrib;
			float float_attrib;
			int identifier;
		};
		int line, col;
		TToken() {}

		TToken(TTokenType use_type) :type(use_type) {}
		TToken(TTokenType use_type, short use_token) :type(use_type), token(use_token) {}
		TToken(TTokenType use_type, short use_token, int use_att) :type(use_type), token(use_token), int_attrib(use_att) {}
		TToken(TTokenType use_type, short use_token, float use_att) :type(use_type), token(use_token), float_attrib(use_att) {}
		TToken(TTokenType use_type, short use_token, TNameId use_identifier)
			:type(use_type), token(use_token), identifier(use_identifier.GetId()) {}

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

	class TLexerPrivate : public ILexer
	{
	public:
		THash<std::string, const char*, int, 16> ids_table;
		THash<std::string, const char*, int, 16> string_literals_table;
		THash<std::string, const char*, TToken, 8> res_words;

		int curr_unique_id;
		std::vector<std::string*> ids;

		const char* source;
		const char* curr_char;
		int col;
		int line;
		char c;

		std::vector<TToken> tokens;
		int curr_token;


		inline void NextChar()
		{
			c = *(++curr_char);
			col++;
		}
		TNameId AddIdentifier(const char* use_name)
		{
			int hash = ids_table.GetHash(use_name);
			int* d;
			std::string* key;
			if (ids_table.Add(hash, use_name, curr_unique_id, key, d))
			{
				ids.push_back(key);
#ifdef _DEBUG
				curr_unique_id++;
				return TNameId(curr_unique_id - 1, ids[curr_unique_id - 1]);
			}
			return TNameId(*d, ids[*d]);
#else
				return TNameId(curr_unique_id++);
		}
			return TNameId(*d);
#endif
		}
		TLexerPrivate();
		bool IsPrevTokenNotId();
		int GetCurrentToken();
		void ParseSource(const char* use_source);
		void Error(const char* s, int token_id = -1, va_list args = nullptr);
		void SetCurrentToken(int use_curr_token);
		TNameId NameId();
		//TODO не должно использоваться, только через lexer по name_id (т.к. много одинаковых)
		std::string StringValue();
		TNameId String();
		float Float();
		int Int();
		bool Bool();
		char Char();
		int GetAttrib();//используется в байткоде чтобы не делать лишних приведений типа
		std::string GetNameFromId(TNameId use_id);
		TNameId GetIdFromName(const char* use_name);
		TTokenType Type();
		int Token();
		bool Test(TTokenType type);
		bool Test(TTokenType type, int token);
		bool TestAndGet(TTokenType type, int token);
		bool TestAndGet(TTokenType type);
		void TestToken(TTokenType type, int token);
		void TestToken(TTokenType type);
		void GetToken(TTokenType type, int token);
		void GetToken(TTokenType type);
		void GetToken();
	};

	int TLexerPrivate::GetCurrentToken()
	{
		return curr_token;
	}

	void TLexerPrivate::Error(const char* s, int token_id, va_list args)
	{
		const int err_head_length = 600;
		const int err_length = 500;
		char err_head[err_head_length + 1];
		char err[err_length + 1];

		if (_vsnprintf_s(err, err_length, s, args) < 0)
			err[err_length] = '\0';

		if (token_id == -1)token_id = curr_token;

		if (_snprintf_s(err_head, err_head_length, "Ошибка (строка %i символ %i): %s\n", tokens[token_id].line, tokens[token_id].col, err) < 0)
			err_head[err_head_length] = '\0';

		throw std::string(err_head);
	}
	void TLexerPrivate::SetCurrentToken(int use_curr_token)
	{
		curr_token = use_curr_token;
	}
	TNameId TLexerPrivate::NameId()
	{
		if (!(tokens[curr_token].type == TTokenType::Identifier))
			Error("Ожидался идентификатор!");
#ifdef _DEBUG
		return TNameId(tokens[curr_token].identifier, ids[tokens[curr_token].identifier]);
#else
		return tokens[curr_token].identifier;
#endif
	}
	//TODO не должно использоваться, только через lexer по name_id (т.к. много одинаковых)
	std::string TLexerPrivate::StringValue()
	{
		if (!(tokens[curr_token].type == TTokenType::Value && tokens[curr_token].token == (short)TValue::String))
			Error("Ожидалась строковая константа!");
		return std::string(*ids[tokens[curr_token].identifier]);
	}
	TNameId TLexerPrivate::String()
	{
		if (!(tokens[curr_token].type == TTokenType::Value&&tokens[curr_token].token == (short)TValue::String))
			Error("Ожидалась строковая константа!");
#ifdef _DEBUG
		return TNameId(tokens[curr_token].identifier, ids[tokens[curr_token].identifier]);
#else
		return tokens[curr_token].identifier;
#endif
	}
	float TLexerPrivate::Float()
	{
		if (!(tokens[curr_token].type == TTokenType::Value&&tokens[curr_token].token == (short)TValue::Float))
			Error("Ожидалось действительное число!");
		return tokens[curr_token].float_attrib;
	}
	int TLexerPrivate::Int()
	{
		if (!(tokens[curr_token].type == TTokenType::Value&&tokens[curr_token].token == (short)TValue::Int))
			Error("Ожидалось целое число!");
		return tokens[curr_token].int_attrib;
	}
	bool TLexerPrivate::Bool()
	{
		//TODO возможно лишние проверки
		if (!(tokens[curr_token].type == TTokenType::Value&&tokens[curr_token].token == (short)TValue::Bool))
			Error("Ожидалась булева константа!");
		return tokens[curr_token].int_attrib != 0;
	}
	char TLexerPrivate::Char()
	{
		if (!(tokens[curr_token].type == TTokenType::Value&&tokens[curr_token].token == (short)TValue::Char))
			Error("Ожидалась символьная константа!");
		return tokens[curr_token].int_attrib;
	}
	int TLexerPrivate::GetAttrib()//используетс¤ в байткоде чтобы не делать лишних приведений типа
	{
		if (!(tokens[curr_token].type == TTokenType::Value &&
			(tokens[curr_token].token == (short)TValue::Bool ||
				tokens[curr_token].token == (short)TValue::Int ||
				tokens[curr_token].token == (short)TValue::Float)))
			Error("Ожидалось целая, булева или действительная константа!");
		return tokens[curr_token].int_attrib;
	}
	std::string TLexerPrivate::GetNameFromId(TNameId use_id)
	{
		return *(ids[use_id.id]);
	}
	TNameId TLexerPrivate::GetIdFromName(const char* use_name)
	{
		return AddIdentifier(use_name);
	}
	TTokenType TLexerPrivate::Type()
	{
		return (TTokenType)tokens[curr_token].type;
	}
	int TLexerPrivate::Token()
	{
		return tokens[curr_token].token;
	}
	bool TLexerPrivate::Test(TTokenType type)
	{
		return tokens[curr_token].type == type;
	}
	bool TLexerPrivate::Test(TTokenType type, int token)
	{
		return tokens[curr_token].type == type&&tokens[curr_token].token == token;
	}
	bool TLexerPrivate::TestAndGet(TTokenType type, int token)
	{
		if (tokens[curr_token].type == type&&tokens[curr_token].token == token)
		{
			curr_token++;
			return true;
		}
		else return false;
	}
	bool TLexerPrivate::TestAndGet(TTokenType type)
	{
		if (tokens[curr_token].type == type)
		{
			curr_token++;
			return true;
		}
		else return false;
	}
	void TLexerPrivate::TestToken(TTokenType type, int token)
	{
		if (type != tokens[curr_token].type&&token == tokens[curr_token].token)
			Error("Ожидался другой токен!");
	}
	void TLexerPrivate::TestToken(TTokenType type)
	{
		if (type != tokens[curr_token].type)Error("Ожидался другой токен!");
	}
	void TLexerPrivate::GetToken(TTokenType type, int token)
	{
		if (type == tokens[curr_token].type&&token == tokens[curr_token].token)
			curr_token++;
		else Error("Ожидался другой токен!");
	}
	void TLexerPrivate::GetToken(TTokenType type)
	{
		if (type == tokens[curr_token].type)
			curr_token++;
		else Error("Ожидался другой токен!");
	}
	void TLexerPrivate::GetToken()
	{
		curr_token++;
	}

	TLexerPrivate::TLexerPrivate()
	{
		source = nullptr;
		curr_char = nullptr;
		col = 1;
		line = 1;
		tokens.reserve(5000);
		curr_token = 0;
		tokens.resize(0);
		curr_unique_id = 0;
		res_words.Add("if", TToken(TTokenType::ResWord, (short)TResWord::If));
		res_words.Add("else", TToken(TTokenType::ResWord, (short)TResWord::Else));
		res_words.Add("for", TToken(TTokenType::ResWord, (short)TResWord::For));
		res_words.Add("while", TToken(TTokenType::ResWord, (short)TResWord::While));

		res_words.Add("class", TToken(TTokenType::ResWord, (short)TResWord::Class));
		res_words.Add("enum", TToken(TTokenType::ResWord, (short)TResWord::Enum));
		res_words.Add("sealed", TToken(TTokenType::ResWord, (short)TResWord::Sealed));
		res_words.Add("return", TToken(TTokenType::ResWord, (short)TResWord::Return));
		res_words.Add("multifield", TToken(TTokenType::ResWord, (short)TResWord::Multifield));
		res_words.Add("new", TToken(TTokenType::ResWord, (short)TResWord::New));

		res_words.Add("true", TToken(TTokenType::Value, (short)TValue::Bool, 1));
		res_words.Add("false", TToken(TTokenType::Value, (short)TValue::Bool, 0));

		res_words.Add("static", TToken(TTokenType::ResWord, (short)TResWord::Static));
		res_words.Add("extern", TToken(TTokenType::ResWord, (short)TResWord::Extern));
		res_words.Add("private", TToken(TTokenType::ResWord, (short)TResWord::Private));
		res_words.Add("public", TToken(TTokenType::ResWord, (short)TResWord::Public));
		res_words.Add("readonly", TToken(TTokenType::ResWord, (short)TResWord::Readonly));
		res_words.Add("this", TToken(TTokenType::ResWord, (short)TResWord::This));
		res_words.Add("bytecode", TToken(TTokenType::ResWord, (short)TResWord::Bytecode));
		res_words.Add("var", TToken(TTokenType::ResWord, (short)TResWord::Var));

		res_words.Add("func", TToken(TTokenType::ResWord, (short)TResWord::Func));
		res_words.Add("default", TToken(TTokenType::ResWord, (short)TResWord::Default));
		res_words.Add("copy", TToken(TTokenType::ResWord, (short)TResWord::Copy));
		res_words.Add("destr", TToken(TTokenType::ResWord, (short)TResWord::Destr));
		res_words.Add("operator", TToken(TTokenType::ResWord, (short)TResWord::Operator));
		res_words.Add("conversion", TToken(TTokenType::ResWord, (short)TResWord::Conversion));

		for (size_t i = 1; i < TOpcode::OP_END; i++)
			res_words.Add(GetBytecodeString((TOpcode::Enum)i), TToken(TTokenType::Bytecode, i));
	}

	//используется при проверке унарных операторов,
	//перед унарным оператором может быть всё кроме:
	//ResWord Value Identifier RBracketRParenthRBrace Vertical Question Ampersand Dot
	bool TLexerPrivate::IsPrevTokenNotId()
	{
		TTokenType prev = (TTokenType)(tokens.size() > 0 ? (short)tokens.back().type : -1);
		return (
			prev == TTokenType::ResWord ||
			prev == TTokenType::Operator ||
			prev == TTokenType::Comma ||
			prev == TTokenType::Semicolon ||
			prev == TTokenType::Colon ||
			prev == TTokenType::LParenth ||
			prev == TTokenType::LBrace ||
			prev == TTokenType::LBracket);
	}

	void TLexerPrivate::ParseSource(const char* use_source)
	{
		source = use_source;
		curr_char = source;
		col = 1;
		line = 1;
		c = *curr_char;
		int token_col = col;
		int token_line = line;
		std::vector<char> id_buf;
		id_buf.reserve(128);
		while (c != '\0')
		{
			if (c == ' ' || c == '\t' || c == 13) {
				NextChar();
				token_col = col;
				token_line = line;
			}
			else if (c == '/'&&curr_char[1] == '/')
			{
				while (c != '\n'&&c != '\0')NextChar();
			}
			else if (c == 10)
			{
				NextChar(); line++; col = 1;
				token_col = col;
				token_line = line;
			}
			else if (isdigit(c))
			{
				bool is_int = (c != '.');
				double val = c - '0';
				NextChar();
				while (isdigit(c))
				{
					val *= 10;
					val += c - '0';
					NextChar();
				}
				if (c == '.')
				{
					is_int = false;
					double temp = 10.0;
					NextChar();
					if (!isdigit(c))Error("Ошибка в числовой константе!");
					while (isdigit(c))
					{
						val += (c - '0') / temp;
						temp *= 10.0;
						NextChar();
					};
				}
				if (c == 'E' || c == 'e')
				{
					double epow = 0.0;
					NextChar();
					if (c == '+') { NextChar(); }
					else if (c == '-') { NextChar(); }
					else if (!isdigit(c))Error("Ошибка в числовой константе!");
					epow = c - '0';
					NextChar();
					while (isdigit(c))
					{
						epow *= 10;
						epow += c - '0';
						NextChar();
					};
					val = pow(val, epow);
					is_int = (floor(epow) == epow);
				}
				if (is_int)tokens.push_back(TToken(TTokenType::Value, (short)TValue::Int, int(val)));
				else tokens.push_back(TToken(TTokenType::Value, (short)TValue::Float, float(val)));
				tokens.back().SetLineCol(token_line, token_col);
			}
			else if (isalpha(c) || c == '_')
			{
				int chars_count = 0;
				while (isalnum(c) || c == '_')
				{
					NextChar(); chars_count++;
				}
				id_buf.resize(chars_count + 1);
				strncpy_s(&id_buf[0], chars_count + 1, (char*)(curr_char - chars_count), chars_count);
				id_buf[chars_count] = '\0';
				std::string* key;
				TToken* data;
				if (res_words.Find(res_words.GetHash(&id_buf[0]), &id_buf[0], key, data))
					tokens.push_back(*data);
				else
					tokens.push_back(TToken(TTokenType::Identifier, -1, AddIdentifier(&id_buf[0])));
				tokens.back().SetLineCol(token_line, token_col);
			}
			else if (c == '"')
			{
				int chars_count = 0;
				NextChar();
				while (c != '"')
				{
					if (c == '\0')Error("Конец файла в константе!");
					if (c == '\n')Error("Символ новой строки в константе!");
					if (c == '\\' && (*(char*)(curr_char + 1)) == '"') {
						NextChar(); chars_count++;
					}
					NextChar(); chars_count++;
				}
				id_buf.resize(chars_count + 1);
				char* currh = (char*)(curr_char - chars_count);
				int t = 0;
				while (currh < curr_char)
				{
					if (*currh == '\\')
					{
						currh++;
						switch (*currh)
						{
						case 'n':id_buf[t] = '\n'; break;
						case 't':id_buf[t] = '\t'; break;
						case '\\':id_buf[t] = '\\'; break;
						case '"':id_buf[t] = '"'; break;
						default:Error("Неизвестный символ в строке!");
						}
					}
					else id_buf[t] = *currh;
					currh++;
					t++;
				}
				id_buf[t] = '\0';
				NextChar();

				tokens.push_back(TToken(TTokenType::Value, (short)TValue::String, AddIdentifier(&id_buf[0])));
				tokens.back().SetLineCol(token_line, token_col);
			}
			else if (c == '\'')
			{
				NextChar();
				{
					if (c == '\0')Error("Конец файла в константе!");
					if (c == '\n')Error("Символ новой строки в константе!");
				}
				char result;
				if (c == '\\')
				{
					NextChar();
					switch (c)
					{
					case 'n':result = '\n'; break;
					case 't':result = '\t'; break;
					case '\\':result = '\\'; break;
					case '\'':result = '\''; break;
					default:Error("Неизвестный символ!");
					}
				}
				else result = c;
				NextChar();
				if (c != '\'')Error("Символьное значение должно состоять из одного символа!");
				NextChar();
				tokens.push_back(TToken(TTokenType::Value, (short)TValue::Char, result));
				tokens.back().SetLineCol(token_line, token_col);
			}
			else
			{
				switch (c)
				{
				case '"':break;
				case '&':
					NextChar();
					if (c == '&')
					{
						NextChar();
						if (c == '=')
						{
							NextChar();
							tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::AndA));
						}
						else tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::And));
					}
					else tokens.push_back(TToken(TTokenType::Ampersand));
					break;
				case '|':
					NextChar();
					if (c == '|') {
						NextChar();
						if (c == '=') {
							NextChar();
							tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::OrA));
						}
						else tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Or));
					}
					else tokens.push_back(TToken(TTokenType::Vertical));
					break;
				case '+':
					NextChar();
					if (c == '=')
					{
						NextChar();
						tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::PlusA));
					}
					else if (c == '+')
					{
						NextChar();
						tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Increment));
					}
					else
					{
						tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Plus));
					}
					break;
				case '-':
					NextChar();
					if (c == '=')
					{
						NextChar();
						tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::MinusA));
					}
					else if (c == '-')
					{
						NextChar();
						tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Decrement));
					}
					else
					{
						if (IsPrevTokenNotId())
							tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::UnaryMinus));
						else if (c == '>')
							tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::GetByReference));
						else
							tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Minus));
					}
					break;
				case '*':NextChar(); if (c == '=') { NextChar(); tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::MulA)); }
						 else tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Mul));
						 break;
				case '/':NextChar(); if (c == '=') { NextChar(); tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::DivA)); }
						 else tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Div));
						 break;
				case '%':NextChar(); if (c == '=') { NextChar(); tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::PercentA)); }
						 else tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Percent));
						 break;
				case '<':NextChar(); if (c == '=') { NextChar(); tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::LessEqual)); }
						 else tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Less));
						 break;
				case '=':NextChar(); if (c == '=') { NextChar(); tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Equal)); }
						 else tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Assign));
						 break;
				case '!':NextChar(); if (c == '=') { NextChar(); tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::NotEqual)); }
						 else tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Not));
						 break;
				case '>':NextChar(); if (c == '=') { NextChar(); tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::GreaterEqual)); }
						 else tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::Greater));
						 break;
				case '[':NextChar(); if (c == ']') { NextChar(); tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::GetArrayElement)); }
						 else tokens.push_back(TToken(TTokenType::LBracket));
						 break;
				case ']':NextChar(); tokens.push_back(TToken(TTokenType::RBracket));
					break;
				case '(':NextChar(); if (c == ')') { NextChar(); tokens.push_back(TToken(TTokenType::Operator, (short)TOperator::ParamsCall)); }
						 else tokens.push_back(TToken(TTokenType::LParenth));
						 break;
				case ')':NextChar(); tokens.push_back(TToken(TTokenType::RParenth));
					break;
				case '{':NextChar(); tokens.push_back(TToken(TTokenType::LBrace));
					break;
				case '}':NextChar(); tokens.push_back(TToken(TTokenType::RBrace));
					break;
				case ';':NextChar(); tokens.push_back(TToken(TTokenType::Semicolon));
					break;
				case ':':NextChar(); tokens.push_back(TToken(TTokenType::Colon));
					break;
				case '?':NextChar(); tokens.push_back(TToken(TTokenType::Question));
					break;
				case ',':NextChar(); tokens.push_back(TToken(TTokenType::Comma));
					break;
				case '.':NextChar(); tokens.push_back(TToken(TTokenType::Dot));
					break;
				default:
					Error("Неизвестный идентификатор!");
				}
				tokens.back().SetLineCol(token_line, token_col);
			}
		}
		tokens.push_back(TToken(TTokenType::Done));
		source = nullptr;
		curr_char = nullptr;
	}

	void TTokenPos::InitPos(ILexer* use_source)
	{
		assert(source == nullptr);
		source = use_source;
		token_id = use_source->GetCurrentToken();
	}
	ILexer* TTokenPos::GetLexer()const
	{
		return source;
	}
	void TTokenPos::Error(const char* s, ...)const
	{
		va_list args;
		va_start(args, s);
		source->Error(s, token_id, args);
	}

	ILexer* ILexer::Create()
	{
		return new TLexerPrivate();
	}

}