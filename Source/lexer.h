#pragma once

#include <stdarg.h>
#include <string>
#include <baluLib.h>

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

		End //используетс¤ дл¤ получени¤ количества зарезервированных слов
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



template<class TKey,class TExtKey, class TData, int hash_bits_count=8>
class THash
{
private:
	enum
	{
		table_size=1<<hash_bits_count
	};
	struct TNode
	{
		TKey key;
		TData data;
		TNode* next;
	};
	TNode* table[table_size];
	BaluLib::TAllocator<TNode, 255> nodes;
	
public:
	THash()
	{
		for(int i=0;i<table_size;i++)
			table[i]=NULL;
	}
	bool Find(int use_hash, TExtKey use_key, TKey* &key, TData* &data)
	{
		assert(use_hash>=0&&use_hash<table_size);
		if(table[use_hash]!=NULL)
		{
			TNode* curr=table[use_hash];
			TNode* last;
			do
			{
				last=curr;
				if(curr->key==use_key)
				{
					data=&curr->data;
					key=&curr->key;
					return true;
				}
				curr=curr->next;
			}while(curr!=NULL);
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
		assert(use_hash>=0&&use_hash<table_size);
		TNode* curr=table[use_hash];
		TNode* last=NULL;
		if(curr!=NULL)
		{
			do{
				last=curr;
				if(curr->key==use_key)
				{
					data=&curr->data;
					key=&curr->key;
					return false;
				}
				curr=curr->next;
			}while(curr!=NULL);
		}
		TNode* node=nodes.New();
		node->key = use_key;
		node->data = use_data;
		node->next = NULL;
		if(table[use_hash]==NULL)
			table[use_hash]=node;
		else
			last->next = node;
		key=&node->key;
		data=&node->data;
		return true;
	}
	bool Add(TExtKey use_key, TData use_data)
	{
		TKey* key;
		TData* data;
		return Add(GetHash(use_key),use_key,use_data,key,data);
	}
	int GetHash(char* str)
	{
		assert(hash_bits_count>=1&&hash_bits_count<=16);
		unsigned char h1, h2;
		int h;
		assert(str!=NULL);
		h1 = *str; 
		if(hash_bits_count>8)
			h2 = *str + 1;
		str++;
		while (*str) 
		{
			h1 = h1 ^ *str;
			if(hash_bits_count>8)
				h2 = h2 ^ *str;
			str++;
		}
		if(hash_bits_count>8)
			h = int(h1 << 8)|int(h2);
		else
			h=h1;
		return h % table_size;
	}
};

class TLexer;

class TNameId
{
	friend class TLexer;
private:
	int id;
#ifdef _DEBUG
	std::string* s;
	TNameId(int use_id,std::string* use_s)
	{
		id=use_id;
		s=use_s;
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
		id=-1;
	}
	TNameId(const TNameId& use_id)
	{
		id=use_id.id;
#ifdef _DEBUG
		s=use_id.s;
#endif
	}
	int GetId()const
	{
		return id;
	}
	bool operator==(TNameId right)const
	{
		assert(!(id==-1||right.id==-1));
		return id==right.id;
	}
	bool operator!=(TNameId right)const
	{
		return id!=right.id;
	}
	bool IsNull()const
	{
		assert(id>=-1);
		return id==-1;
	}
};

class TLexer
{
private:

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
		int line,col;
		TToken(){}

		TToken(TTokenType::Enum use_type):type(use_type){}
		TToken(TTokenType::Enum use_type,short use_token):type(use_type),token(use_token){}
		TToken(TTokenType::Enum use_type,short use_token, int use_att):type(use_type),token(use_token),int_attrib(use_att){}
		TToken(TTokenType::Enum use_type,short use_token, float use_att):type(use_type),token(use_token),float_attrib(use_att){}
		TToken(TTokenType::Enum use_type,short use_token, TNameId use_identifier)
		:type(use_type),token(use_token),identifier(use_identifier.GetId()){}

		void SetLineCol(int use_line,int use_col)
		{
			line=use_line;
			col=use_col;
		}
		bool operator==(const TToken& v)const
		{
			return type==v.type&&token==v.token;
		}
	};

private:
	THash<std::string,char*,int,16> ids_table;
	THash<std::string,char*,int,16> string_literals_table;
	THash<std::string,char*,TToken,8> res_words;

	int curr_unique_id;
	std::vector<std::string*> ids;

	char* source;
	char* curr_char;
	int col;
	int line;
	char c;

	std::vector<TToken> tokens;
	int curr_token;

	__forceinline void NextChar()
	{
		c=*(++curr_char);
		col++;
	}
	TNameId AddIdentifier(char* use_name)
	{
		int hash=ids_table.GetHash(use_name);
		int* d;
		std::string* key;
		if(ids_table.Add(hash,use_name,curr_unique_id,key,d))
		{
			ids.push_back(key);
#ifdef _DEBUG
			curr_unique_id++;
			return TNameId(curr_unique_id-1,ids[curr_unique_id-1]);
		}
		return TNameId(*d,ids[*d]);
#else
			return TNameId(curr_unique_id++);
		}
		return TNameId(*d);
#endif
	}
public:
	TLexer();
	int GetCurrentToken()
	{
		return curr_token;
	}
	void ParseSource(char* use_source);
	void Error(char* s,int token_id=-1,va_list args=NULL)
	{
		const int err_head_length=600;
		const int err_length=500;
		char err_head[err_head_length+1];
		char err[err_length+1];

		if(_vsnprintf_s(err,err_length,s,args)<0)
			err[err_length]='\0';

		if(token_id==-1)token_id=curr_token;

		if(_snprintf_s(err_head,err_head_length,"Ошибка (строка %i символ %i): %s\n",tokens[token_id].line,tokens[token_id].col,err)<0)
			err_head[err_head_length]='\0';

		throw std::string(err_head);
	}
	void SetCurrentToken(int use_curr_token)
	{
		curr_token=use_curr_token;
	}
	TNameId NameId()
	{
		if(!(tokens[curr_token].type==TTokenType::Identifier))
			Error("Ожидался идентификатор!");
#ifdef _DEBUG
		return TNameId(tokens[curr_token].identifier,ids[tokens[curr_token].identifier]);
#else
		return tokens[curr_token].identifier;
#endif
	}
	TNameId String()
	{
		if(!(tokens[curr_token].type==TTokenType::Value&&tokens[curr_token].token==TValue::String))
			Error("Ожидалась строковая константа!");
#ifdef _DEBUG
		return TNameId(tokens[curr_token].identifier,ids[tokens[curr_token].identifier]);
#else
		return tokens[curr_token].identifier;
#endif
	}
	float Float()
	{
		if(!(tokens[curr_token].type==TTokenType::Value&&tokens[curr_token].token==TValue::Float))
			Error("Ожидалось действительное число!");
		return tokens[curr_token].float_attrib;
	}
	int Int()
	{
		if(!(tokens[curr_token].type==TTokenType::Value&&tokens[curr_token].token==TValue::Int))
			Error("Ожидалось целое число!");
		return tokens[curr_token].int_attrib;
	}
	bool Bool()
	{
		//TODO возможно лишние проверки
		if(!(tokens[curr_token].type==TTokenType::Value&&tokens[curr_token].token==TValue::Bool))
			Error("Ожидалась булева константа!");
		return tokens[curr_token].int_attrib!=0;
	}
	char Char()
	{
		if(!(tokens[curr_token].type==TTokenType::Value&&tokens[curr_token].token==TValue::Char))
			Error("Ожидалась символьная константа!");
		return tokens[curr_token].int_attrib;
	}
	int GetAttrib()//используетс¤ в байткоде чтобы не делать лишних приведений типа
	{
		if(!(tokens[curr_token].type==TTokenType::Value&&
		(tokens[curr_token].token==TValue::Bool||
		tokens[curr_token].token==TValue::Int||
		tokens[curr_token].token==TValue::Float)))
			Error("Ожидалось целая, булева или действительная константа!");
		return tokens[curr_token].int_attrib;
	}
	std::string GetNameFromId(TNameId use_id)
	{
		return *(ids[use_id.id]);
	}
	TNameId GetIdFromName(char* use_name)
	{
		return AddIdentifier(use_name);
	}
	TTokenType::Enum Type()
	{
		return (TTokenType::Enum)tokens[curr_token].type;
	}
	int Token()
	{
		return tokens[curr_token].token;
	}
	bool Test(int type)
	{
		return tokens[curr_token].type==type;
	}
	bool Test(int type,int token)
	{
		return tokens[curr_token].type==type&&tokens[curr_token].token==token;
	}
	bool TestAndGet(int type,int token)
	{
		if(tokens[curr_token].type==type&&tokens[curr_token].token==token)
		{
			curr_token++;
			return true;
		}else return false;
	}
	bool TestAndGet(int type)
	{
		if(tokens[curr_token].type==type)
		{
			curr_token++;
			return true;
		}else return false;
	}
	void TestToken(int type,int token)
	{
		if(type!=tokens[curr_token].type&&token==tokens[curr_token].token)
			Error("Ожидался другой токен!");
	}
	void TestToken(int type)
	{
		if(type!=tokens[curr_token].type)Error("Ожидался другой токен!");
	}
	void GetToken(int type,int token)
	{
		if(type==tokens[curr_token].type&&token==tokens[curr_token].token)
			curr_token++;
		else Error("Ожидался другой токен!");
	}
	void GetToken(int type)
	{
		if(type==tokens[curr_token].type)
			curr_token++;
		else Error("Ожидался другой токен!");
	}
	void GetToken()
	{
		curr_token++;
	}
};

class TTokenPos
{
	int token_id;
public:
	TLexer* source;
	void InitPos(TLexer& use_source)
	{
		source = &use_source;
		token_id = use_source.GetCurrentToken();
	}
	void Error(char* s, ...)
	{
		va_list args;
		va_start(args, s);
		source->Error(s, token_id, args);
	}
};
