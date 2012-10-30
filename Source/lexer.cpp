#include "baluScript.h"

TLexer::TLexer():source(NULL),curr_char(NULL),col(1),line(1)
{
	tokens.SetReserve(5000);
	curr_unique_id=0;
	res_words.Add("if",			TToken(TTokenType::ResWord, TResWord::If));
	res_words.Add("else",		TToken(TTokenType::ResWord, TResWord::Else));		
	res_words.Add("for",		TToken(TTokenType::ResWord, TResWord::For));
	res_words.Add("while",		TToken(TTokenType::ResWord, TResWord::While));	

	res_words.Add("class",		TToken(TTokenType::ResWord, TResWord::Class));
	res_words.Add("enum",		TToken(TTokenType::ResWord, TResWord::Enum));
	res_words.Add("sealed",		TToken(TTokenType::ResWord, TResWord::Sealed));
	res_words.Add("return",		TToken(TTokenType::ResWord, TResWord::Return));

	res_words.Add("true",		TToken(TTokenType::Value, TValue::Bool, 1));
	res_words.Add("false",		TToken(TTokenType::Value, TValue::Bool, 0));

	res_words.Add("static",		TToken(TTokenType::ResWord, TResWord::Static));
	res_words.Add("extern",		TToken(TTokenType::ResWord, TResWord::Extern));
	res_words.Add("private",	TToken(TTokenType::ResWord, TResWord::Private));
	res_words.Add("protected",	TToken(TTokenType::ResWord, TResWord::Protected));
	res_words.Add("public",		TToken(TTokenType::ResWord, TResWord::Public));	
	res_words.Add("readonly",	TToken(TTokenType::ResWord, TResWord::Readonly));
	res_words.Add("this",		TToken(TTokenType::ResWord, TResWord::This));
	res_words.Add("bytecode",	TToken(TTokenType::ResWord, TResWord::Bytecode));

	res_words.Add("func",		TToken(TTokenType::ResWord, TResWord::Func));		
	res_words.Add("constr",		TToken(TTokenType::ResWord, TResWord::Constr));	
	res_words.Add("destr",		TToken(TTokenType::ResWord, TResWord::Destr));		
	res_words.Add("operator",	TToken(TTokenType::ResWord, TResWord::Operator));
	res_words.Add("conversion",	TToken(TTokenType::ResWord, TResWord::Conversion));
	
	for(int i=1;i<TOpcode::OP_END;i++)
		res_words.Add(GetBytecodeString((TOpcode::Enum)i),TToken(TTokenType::Bytecode, i));
}

void TLexer::ParseSource(char* use_source)
{
	source=use_source;
	curr_char=source;
	col=1;
	line=1;
	c=*curr_char;
	curr_token=0;
	tokens.SetCount(0);
	int token_col=col;
	int token_line=line;
	TVector<char,128> id_buf;
	while(c!='\0')
	{
		if(c==' '||c=='\t'||c==13){
			NextChar();
			token_col=col;
			token_line=line;
		}else if(c=='/'&&curr_char[1]=='/')
		{
			while(c!='\n'&&c!='\0')NextChar();
		}
		else if(c==10)
		{
			NextChar();line++;col=1;
			token_col=col;
			token_line=line;
		}
		else if(isdigit(c))
		{
			bool is_int=(c!='.');
			double val=c-'0';
			NextChar();
			while(isdigit(c))
			{
				val*=10;
				val+=c-'0';
				NextChar();
			}
			if(c=='.')
			{
				is_int=false;
				double temp=10.0;
				NextChar();
				if(!isdigit(c))Error("Ошибка в числовой константе!");
				while(isdigit(c))
				{
					val+=(c-'0')/temp;
					temp*=10.0;
					NextChar();
				};
			}
			if(c=='E'||c=='e')
			{
				double temp=1.0;
				double epow=0.0;
				NextChar();
				if(c=='+'){temp=1.0;NextChar();}
				else if(c=='-'){temp=-1.0;NextChar();}
				else if(!isdigit(c))Error("Ошибка в числовой константе!");
				epow=c-'0';
				NextChar();
				while(isdigit(c))
				{
					epow*=10;
					epow+=c-'0';
					NextChar();
				};
				val=pow(val,epow);
				is_int=(floor(epow)==epow);
			}
			if(is_int)tokens.Push(TToken(TTokenType::Value,TValue::Int,int(val)));
			else tokens.Push(TToken(TTokenType::Value,TValue::Float,float(val))); 
			tokens.GetTop(0).SetLineCol(token_line,token_col);
		}
		else if (isalpha(c)||c=='_')
		{
			int chars_count=0;
			while(isalnum(c)||c=='_')
			{
				NextChar();chars_count++;
			}
			id_buf.SetCount(chars_count+1);
			strncpy(&id_buf[0],(char*)(curr_char-chars_count),chars_count);
			id_buf[chars_count]='\0';
			std::string* key;
			TToken* data;
			if(res_words.Find(res_words.GetHash(&id_buf[0]),&id_buf[0],key,data))
				tokens.Push(*data);
			else
				tokens.Push(TToken(TTokenType::Identifier,-1,AddIdentifier(&id_buf[0])));
			tokens.GetTop(0).SetLineCol(token_line,token_col);
		}
		else if (c=='"')
		{
			int chars_count=0;
			NextChar();
			while(c!='"')
			{
				if(c=='\0')Error("Конец файла в константе!");
				if(c=='\n')Error("Символ новой строки в константе!");
				if(c=='\\'&&(*(char*)(curr_char+1))=='"'){
					NextChar();chars_count++;
				}
				NextChar();chars_count++;
			}
			id_buf.SetCount(chars_count+1);
			char* currh=(char*)(curr_char-chars_count);
			int t=0;
			while(currh<curr_char)
			{
				if(*currh=='\\')
				{
					currh++;
					switch(*currh)
					{
					case 'n':id_buf[t]='\n';break;
					case 't':id_buf[t]='\t';break;
					case '\\':id_buf[t]='\\';break;
					case '"':id_buf[t]='"';break;
					default:Error("Неизвестный символ в строке!");
					}
				}else id_buf[t]=*currh;
				currh++;
				t++;
			}
			id_buf[t]='\0';
			NextChar();

			tokens.Push(TToken(TTokenType::Value,TValue::String,AddIdentifier(&id_buf[0])));
			tokens.GetTop(0).SetLineCol(token_line,token_col);
		}
		else if (c=='\'')
		{
			NextChar();
			{
				if(c=='\0')Error("Конец файла в константе!");
				if(c=='\n')Error("Символ новой строки в константе!");
			}
			char result;
			if(c=='\\')
			{
				NextChar();
				switch(c)
				{
				case 'n':result='\n';break;
				case 't':result='\t';break;
				case '\\':result='\\';break;
				case '\'':result='\'';break;
				default:Error("Неизвестный символ!");
				}
			}else result=c;
			NextChar();
			if(c!='\'')Error("Символьное значение должно состоять из одного символа!");
			NextChar();
			tokens.Push(TToken(TTokenType::Value,TValue::Char,result));
			tokens.GetTop(0).SetLineCol(token_line,token_col);
		}
		else
		{
			switch(c)
			{
			case '"':break;		
			case '&':
				NextChar();
				if(c=='&')
				{
					NextChar();
					if(c=='=')
					{
						NextChar();
						tokens.Push(TToken(TTokenType::Operator, TOperator::AndA));
					}
					else tokens.Push(TToken(TTokenType::Operator, TOperator::And));
				}
				else tokens.Push(TToken(TTokenType::Ampersand));
				break;
			case '|':
				NextChar();
				if(c=='|'){
					NextChar();
					if(c=='='){
						NextChar();
						tokens.Push(TToken(TTokenType::Operator, TOperator::OrA));
					}
					else tokens.Push(TToken(TTokenType::Operator, TOperator::Or));
				}
				else tokens.Push(TToken(TTokenType::Vertical));
				break;
			case '+':NextChar();if(c=='='){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::PlusA));}
					 else tokens.Push(TToken(TTokenType::Operator, TOperator::Plus));
					 break;
			case '-':
				NextChar();
				if(c=='='){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::MinusA));}
				else 
				{
					TTokenType::Enum prev=(TTokenType::Enum)(tokens.GetHigh()>-1?(TTokenType::Enum)tokens.GetTop(0).type:-1);
					if(prev==TTokenType::ResWord||
						prev==TTokenType::Operator||
						prev==TTokenType::Comma||
						prev==TTokenType::Semicolon||
						prev==TTokenType::Colon||
						prev==TTokenType::LParenth||
						prev==TTokenType::LBrace||
						prev==TTokenType::LBracket)
						tokens.Push(TToken(TTokenType::Operator, TOperator::UnaryMinus));
					else
						tokens.Push(TToken(TTokenType::Operator, TOperator::Minus));
				}
				break;
			case '*':NextChar();if(c=='='){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::MulA));}
					 else tokens.Push(TToken(TTokenType::Operator, TOperator::Mul));
					 break;
			case '/':NextChar();if(c=='='){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::DivA));}
					 else tokens.Push(TToken(TTokenType::Operator, TOperator::Div));
					 break;
			case '%':NextChar();if(c=='='){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::PercentA));}
					 else tokens.Push(TToken(TTokenType::Operator, TOperator::Percent));
					 break; 
			case '<':NextChar();if(c=='='){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::LessEqual));}
					 else tokens.Push(TToken(TTokenType::Operator, TOperator::Less));
					 break;
			case '=':NextChar();if(c=='='){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::Equal));}
					 else tokens.Push(TToken(TTokenType::Operator, TOperator::Assign));
					 break;
			case '!':NextChar();if(c=='='){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::NotEqual));}
					 else tokens.Push(TToken(TTokenType::Operator, TOperator::Not));
					 break;
			case '>':NextChar();if(c=='='){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::GreaterEqual));}
					 else tokens.Push(TToken(TTokenType::Operator, TOperator::Greater));
					 break;
			case '[':NextChar();if(c==']'){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::GetArrayElement));}
					 else tokens.Push(TToken(TTokenType::LBracket));
					 break;
			case ']':NextChar();tokens.Push(TToken(TTokenType::RBracket));
					 break;
			case '(':NextChar();if(c==')'){NextChar();tokens.Push(TToken(TTokenType::Operator, TOperator::ParamsCall));}
					 else tokens.Push(TToken(TTokenType::LParenth));
					 break;
			case ')':NextChar();tokens.Push(TToken(TTokenType::RParenth));
					 break;
			case '{':NextChar();tokens.Push(TToken(TTokenType::LBrace));
					 break;
			case '}':NextChar();tokens.Push(TToken(TTokenType::RBrace));
					 break;
			case ';':NextChar();tokens.Push(TToken(TTokenType::Semicolon));
					 break;
			case ':':NextChar();tokens.Push(TToken(TTokenType::Colon));
					 break;
			case '?':NextChar();tokens.Push(TToken(TTokenType::Question));
					 break;
			case ',':NextChar();tokens.Push(TToken(TTokenType::Comma));
					 break;
			case '.':NextChar();tokens.Push(TToken(TTokenType::Dot));
					 break;
			default: 
				Error("Неизвестный идентификатор!");
			}
			tokens.GetTop(0).SetLineCol(token_line,token_col);
		}
	}
	tokens.Push(TToken(TTokenType::Done));
	source=NULL;
	curr_char=NULL;
}

