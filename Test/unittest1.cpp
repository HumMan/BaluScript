#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Test
{
	TSyntaxAnalyzer* syntax;
	TTime* time;
	TSMethod* CreateMethod(char* code)
	{
		try
		{
			TMethod* m = new TMethod(syntax->base_class);
			syntax->lexer.ParseSource(code);
			m->AnalyzeSyntax(syntax->lexer);

			TSMethod* ms = new TSMethod(syntax->sem_base_class, m);
			ms->Build();
			std::vector<TSClassField*> static_fields;
			std::vector<TSLocalVar*> static_variables;
			ms->LinkSignature(&static_fields,&static_variables);
			ms->LinkBody(&static_fields, &static_variables);
			ms->CalculateParametersOffsets();
			return ms;
		}
		catch (std::string s)
		{
			printf(s.c_str());
			Logger::WriteMessage(s.c_str());
			//throw;
		}
	}
	TSMethod* CreateClass(char* code)
	{
		try
		{
			TClass* cl = new TClass(syntax->base_class);
			syntax->lexer.ParseSource(code);
			cl->AnalyzeSyntax(syntax->lexer);

			TSClass* scl = new TSClass(syntax->sem_base_class, cl);
			scl->Build();
			std::vector<TSClassField*> static_fields;
			std::vector<TSLocalVar*> static_variables;
			scl->LinkSignature(&static_fields, &static_variables);
			scl->LinkBody(&static_fields, &static_variables);
			std::vector<TSClass*> owners;
			scl->CalculateSizes(owners);
			scl->CheckForErrors();
			scl->CalculateMethodsSizes();
			std::vector<TSMethod*> methods;
			scl->GetMethods(methods, syntax->lexer.GetIdFromName("Test"));
			return methods[0];
		}
		catch (std::string s)
		{
			printf(s.c_str());
			Logger::WriteMessage(s.c_str());
			//throw;
		}
	}
	TStackValue RunCode(char* code)
	{
		TSMethod* ms = CreateMethod(code);
		std::vector<TStackValue> params;
		TStackValue result, object;
		std::vector<TStaticValue> static_fields;
		std::vector<TStaticValue> static_variables;
		ms->Run(static_fields, params, result, object);
		return result;
	}
	TStackValue RunClassWithCode(char* code)
	{
		TSMethod* ms = CreateClass(code);
		std::vector<TStackValue> params;
		TStackValue result, object;
		std::vector<TStaticValue> static_fields;
		std::vector<TStaticValue> static_variables;
		ms->Run(static_fields, params, result, object);
		return result;
	}
	TEST_MODULE_INITIALIZE(BaseTypesTestsInitialize)
	{
		setlocale(LC_ALL, "windows");
		printf("Compiling ... \n");
		time = new TTime();
		time->Start();
		char* source;
		{
			TFileData file("../../Source/NativeTypes/int_type.bscript", "rb");
			source = file.ReadAll();
			source[file.GetSize()] = '\0';
		}
		syntax = new TSyntaxAnalyzer();
		syntax->Compile(source, *time);
		delete source;
	}
	TEST_MODULE_CLEANUP(BaseTypesTestsCleanup)
	{
		delete time;
		delete syntax;
	}
	TEST_CLASS(IntTesting)
	{
	public:

		TEST_METHOD(IntConstr)
		{		
			Assert::AreEqual(5, *(int*)RunCode("func static Test:int{int s; s=5;return s;}").get());
			Assert::AreEqual(5, *(int*)RunCode("func static Test:int{int s,b;b=5;s=b;return s;}").get());
			Assert::AreEqual(5, *(int*)RunCode("func static Test:int{int s(5);return s;}").get());
			Assert::AreEqual(2, *(int*)RunCode("func static Test:int{int s=8+2-8;return s;}").get());
			Assert::AreEqual(5, *(int*)RunCode("func static Test:int{int s=8+2-8,b=s*2;int c=b+1;return c;}").get());
			Assert::AreEqual(15, *(int*)RunCode("func static Test:int{int s=8+2-8,b=s*2;int c=b+1;{int e=c*3;return e;}return c;}").get());
			Assert::AreEqual(7, *(int*)RunCode("func static Test:int{int s(7.4);return s;}").get());
			Assert::AreEqual(18, *(int*)RunCode("func static Test:int{int s(7.4+11.2);return s;}").get());
			Assert::AreEqual(17, *(int*)RunCode("func static Test:int{int s(7.4+11.2),b=8+2-9,c(2);return s+b-c;}").get());
		}
		TEST_METHOD(IntBinOperator)
		{
			Assert::AreEqual(9, *(int*)RunCode("func static Test:int{int a,b;a=3;b=6;return a+b;}").get());
			Assert::AreEqual(-3, *(int*)RunCode("func static Test:int{int a,b;a=3;b=6;return a-b;}").get());
			Assert::AreEqual(3*6, *(int*)RunCode("func static Test:int{int a,b;a=3;b=6;return a*b;}").get());
			Assert::AreEqual(0, *(int*)RunCode("func static Test:int{int a,b;a=3;b=6;return a/b;}").get());
			Assert::AreEqual(0, *(int*)RunCode("func static Test:int{int a,b;a=6;b=3;return a%b;}").get());
			Assert::AreEqual(2, *(int*)RunCode("func static Test:int{int a,b;a=8;b=3;return a%b;}").get());
		}
		TEST_METHOD(IntCompareByRefOperator)
		{
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{int a,b;a=8;b=3;return a==b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{int a,b;a=8;b=3;return a!=b;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{int a,b;a=8;b=3;return a<b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{int a,b;a=-2;b=3;return a<b;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{int a,b;a=8;b=3;return a<=b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{int a,b;a=3;b=3;return a<=b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{int a,b;a=8;b=3;return a>b;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{int a,b;a=3;b=3;return a>b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{int a,b;a=8;b=3;return a>=b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{int a,b;a=3;b=3;return a>=b;}").get());
		}
		TEST_METHOD(IntCompareByValueOperator)
		{
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{return 2==3;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{return 2!=3;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{return 8<3;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{return -2<3;}").get());
		}
		TEST_METHOD(IntBinAssign)
		{
			Assert::AreEqual(7, *(int*)RunCode("func static Test:int{int s;s=5;s+=2;return s;}").get());
			Assert::AreEqual(3, *(int*)RunCode("func static Test:int{int s;s=5;s-=2;return s;}").get());
			Assert::AreEqual(10, *(int*)RunCode("func static Test:int{int s;s=5;s*=2;return s;}").get());
			Assert::AreEqual(2, *(int*)RunCode("func static Test:int{int s;s=5;s/=2;return s;}").get());
			Assert::AreEqual(1, *(int*)RunCode("func static Test:int{int s;s=5;s%=2;return s;}").get());
		}
		TEST_METHOD(IntUnaryOperator)
		{
			Assert::AreEqual(-3, *(int*)RunCode("func static Test:int{int a;a=3;return -a;}").get());
			Assert::AreEqual(-34, *(int*)RunCode("func static Test:int{return -34;}").get());
		}
		TEST_METHOD(IntConvert)
		{
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{int a;a=3;return a;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{int a;a=-3;return a;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{int a;a=0;return a;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{return 4;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{return -4;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{return 0;}").get());

			Assert::AreEqual((float)3, *(float*)RunCode("func static Test:float{int a;a=3;return a;}").get());
			Assert::AreEqual((float)-3, *(float*)RunCode("func static Test:float{int a;a=-3;return a;}").get());
			Assert::AreEqual((float)0, *(float*)RunCode("func static Test:float{int a;a=0;return a;}").get());
			Assert::AreEqual((float)4, *(float*)RunCode("func static Test:float{return 4;}").get());
			Assert::AreEqual((float)-4, *(float*)RunCode("func static Test:float{return -4;}").get());
			Assert::AreEqual((float)0, *(float*)RunCode("func static Test:float{return 0;}").get());
		}
		TEST_METHOD(IntFunc)
		{
			Assert::AreEqual(5, *(int*)RunCode("func static Test:int{return Abs(-5);}").get());
			Assert::AreEqual(5, *(int*)RunCode("func static Test:int{return Abs(5);}").get());
			Assert::AreEqual(0, *(int*)RunCode("func static Test:int{return Abs(0);}").get());
			Assert::AreEqual(7, *(int*)RunCode("func static Test:int{return Max(-4,7);}").get());
			Assert::AreEqual(4, *(int*)RunCode("func static Test:int{return Max(4,-7);}").get());
			Assert::AreEqual(-4, *(int*)RunCode("func static Test:int{return Max(-4,-7);}").get());
			Assert::AreEqual(-4, *(int*)RunCode("func static Test:int{return Min(-4,7);}").get());
			Assert::AreEqual(-7, *(int*)RunCode("func static Test:int{return Min(4,-7);}").get());
			Assert::AreEqual(-7, *(int*)RunCode("func static Test:int{return Min(-4,-7);}").get());
			Assert::AreEqual(-1, *(int*)RunCode("func static Test:int{return Sign(-4);}").get());
			Assert::AreEqual(1, *(int*)RunCode("func static Test:int{return Sign(4);}").get());
			Assert::AreEqual(0, *(int*)RunCode("func static Test:int{return Sign(0);}").get());
		}
		TEST_METHOD(IntComplex)
		{
			Assert::AreEqual((int)2, *(int*)RunClassWithCode(
				"class TestClass {"
					"class SubClass {"
					"int a,b,c;"
					"constr(int a1,int b1, int c1){}"
					"func Init{a=1;b=2;c=3;}"
				"}"
				"func static Test:int"
				"{"
					"SubClass s;s.Init();return s.b;"
				"}}"
				).get());

			Assert::AreEqual((int)4, *(int*)RunClassWithCode(
				"class TestClass {"
				"func static Make(int& a,int& b,int& c){a=2;b=4;c=6;}"
				"class SubClass {"
				"int a,b,c;"
				"constr(int a1,int b1, int c1){}"
				"func Init{Make(a,b,c);}"
				"}"
				"func static Test:int"
				"{"
				"SubClass s;s.Init();return s.b;"
				"}}"
				).get());
		}
	};
	TEST_CLASS(FloatTesting)
	{
	public:

		TEST_METHOD(FloatConstr)
		{
			Assert::AreEqual(5.0f, *(float*)RunCode("func static Test:float{float s; s=5.0;return s;}").get());
			Assert::AreEqual(5.0f, *(float*)RunCode("func static Test:float{float s,b;b=5.0;s=b;return s;}").get());
			Assert::AreEqual(5.0f, *(float*)RunCode("func static Test:float{float s(5);return s;}").get());
			Assert::AreEqual(2.0f, *(float*)RunCode("func static Test:float{float s=8+2-8;return s;}").get());
			Assert::AreEqual(5.0f, *(float*)RunCode("func static Test:float{float s=8+2-8,b=s*2;float c=b+1;return c;}").get());
			Assert::AreEqual(15.0f, *(float*)RunCode("func static Test:float{float s=8+2-8,b=s*2;float c=b+1;{float e=c*3;return e;}return c;}").get());
			Assert::AreEqual(18, *(int*)RunCode("func static Test:int{int s(7.4+11.2);return s;}").get());
		}
		TEST_METHOD(FloatBinOperator)
		{
			Assert::AreEqual(9.0f, *(float*)RunCode("func static Test:float{float a,b;a=3;b=6;return a+b;}").get());
			Assert::AreEqual(-3.0f, *(float*)RunCode("func static Test:float{float a,b;a=3;b=6;return a-b;}").get());
			Assert::AreEqual(3 * 6.0f, *(float*)RunCode("func static Test:float{float a,b;a=3;b=6;return a*b;}").get());
			Assert::AreEqual(0.5f, *(float*)RunCode("func static Test:float{float a,b;a=3;b=6;return a/b;}").get());
		}
		TEST_METHOD(FloatCompareByRefOperator)
		{
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{float a,b;a=8.2;b=3.2;return a==b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{float a,b;a=8;b=3;return a!=b;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{float a,b;a=8;b=3;return a<b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{float a,b;a=-2.9;b=3;return a<b;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{float a,b;a=8;b=3;return a<=b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{float a,b;a=3.1;b=3.1;return a<=b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{float a,b;a=8;b=3;return a>b;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{float a,b;a=3;b=3;return a>b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{float a,b;a=8;b=3;return a>=b;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{float a,b;a=3.5;b=3.5;return a>=b;}").get());
		}
		TEST_METHOD(FloatCompareByValueOperator)
		{
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{return 2.0==3.0;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{return 2.2!=3.4;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{return 8.8<3.3;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{return -2.3<3;}").get());
		}
		TEST_METHOD(FloatBinAssign)
		{
			Assert::AreEqual(7.0f, *(float*)RunCode("func static Test:float{float s;s=5;s+=2;return s;}").get());
			Assert::AreEqual(3.0f, *(float*)RunCode("func static Test:float{float s;s=5;s-=2;return s;}").get());
			Assert::AreEqual(10.0f, *(float*)RunCode("func static Test:float{float s;s=5;s*=2;return s;}").get());
			Assert::AreEqual(2.5f, *(float*)RunCode("func static Test:float{float s;s=5;s/=2;return s;}").get());
		}
		TEST_METHOD(FloatUnaryOperator)
		{
			Assert::AreEqual(-3.0f, *(float*)RunCode("func static Test:float{float a;a=3;return -a;}").get());
			Assert::AreEqual(-34.0f, *(float*)RunCode("func static Test:float{return -34;}").get());
		}
		TEST_METHOD(FloatConvert)
		{
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{float a;a=3;return a;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{float a;a=-3;return a;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{float a;a=0;return a;}").get());

			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{return 4;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{return -4;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{return 0;}").get());

			Assert::AreEqual((float)3, *(float*)RunCode("func static Test:float{float a;a=3;return a;}").get());
			Assert::AreEqual((float)-3, *(float*)RunCode("func static Test:float{float a;a=-3;return a;}").get());
			Assert::AreEqual((float)0, *(float*)RunCode("func static Test:float{float a;a=0;return a;}").get());

			Assert::AreEqual((float)4, *(float*)RunCode("func static Test:float{return 4;}").get());
			Assert::AreEqual((float)-4, *(float*)RunCode("func static Test:float{return -4;}").get());
			Assert::AreEqual((float)0, *(float*)RunCode("func static Test:float{return 0;}").get());
		}
		TEST_METHOD(FloatFunc)
		{
			Assert::AreEqual(5.4f, *(float*)RunCode("func static Test:float{return Abs(-5.4);}").get());
			Assert::AreEqual(5.0f, *(float*)RunCode("func static Test:float{return Abs(5.0);}").get());
			Assert::AreEqual(0.0f, *(float*)RunCode("func static Test:float{return Abs(0.0);}").get());

			Assert::AreEqual(asin(1.0f), *(float*)RunCode("func static Test:float{return Asin(1.0);}").get(),0.0001f);
			Assert::AreEqual(asin(0.0f), *(float*)RunCode("func static Test:float{return Asin(0.0);}").get(), 0.0001f);
			Assert::AreEqual(asin(-1.0f), *(float*)RunCode("func static Test:float{return Asin(-1.0);}").get(), 0.0001f);

			Assert::AreEqual(acos(1.0f), *(float*)RunCode("func static Test:float{return Acos(1.0);}").get(), 0.0001f);
			Assert::AreEqual(acos(0.0f), *(float*)RunCode("func static Test:float{return Acos(0.0);}").get(), 0.0001f);
			Assert::AreEqual(acos(-1.0f), *(float*)RunCode("func static Test:float{return Acos(-1.0);}").get(), 0.0001f);

			Assert::AreEqual(atan(1.0f), *(float*)RunCode("func static Test:float{return Atan(1.0);}").get(), 0.0001f);
			Assert::AreEqual(atan(0.0f), *(float*)RunCode("func static Test:float{return Atan(0.0);}").get(), 0.0001f);
			Assert::AreEqual(atan(-1.0f), *(float*)RunCode("func static Test:float{return Atan(-1.0);}").get(), 0.0001f);

			Assert::AreEqual(7.0f, *(float*)RunCode("func static Test:float{return Max(-4.0,7.0);}").get());
			Assert::AreEqual(4.0f, *(float*)RunCode("func static Test:float{return Max(4.0,-7.0);}").get());
			Assert::AreEqual(-4.0f, *(float*)RunCode("func static Test:float{return Max(-4.0,-7.0);}").get());

			Assert::AreEqual(-4.2f, *(float*)RunCode("func static Test:float{return Min(-4.2,7.0);}").get());
			Assert::AreEqual(-7.0f, *(float*)RunCode("func static Test:float{return Min(4.0,-7.0);}").get());
			Assert::AreEqual(-7.0f, *(float*)RunCode("func static Test:float{return Min(-4.0,-7.0);}").get());

			Assert::AreEqual(-1.0f, *(float*)RunCode("func static Test:float{return Sign(-4.0);}").get());
			Assert::AreEqual(1.0f, *(float*)RunCode("func static Test:float{return Sign(4.0);}").get());
			Assert::AreEqual(0.0f, *(float*)RunCode("func static Test:float{return Sign(0.0);}").get());
		}
	};
	TEST_CLASS(StatementsTesting)
	{
	public:

		TEST_METHOD(IfTest)
		{
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{if(true)return false; else return true;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{if(true||true)return true&&false&&true; else return true;}").get());
			Assert::AreEqual(true, *(bool*)RunCode("func static Test:bool{if(true&&false)return true&&false&&true; else return true;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{if(!true)return true&&false&&true; else if(!false) return false; else return true;}").get());
			Assert::AreEqual(false, *(bool*)RunCode("func static Test:bool{if(2<3) return false; else return true;}").get());
		}
		TEST_METHOD(WhileTest)
		{
			Assert::AreEqual(400, *(int*)RunCode("func static Test:int{int i=0;while(i<400){i+=1;}return i;}").get());
			Assert::AreEqual(401, *(int*)RunCode("func static Test:int{int i=0;while(i<=400){i+=1;}return i;}").get());
		}
		TEST_METHOD(ForTest)
		{
			Assert::AreEqual(13, *(int*)RunCode("func static Test:int{int s=3;for(int i=0;i<5;i+=1)s+=i;return s;}").get());
			Assert::AreEqual(389, *(int*)RunCode("func static Test:int{for(int i=0;i<500;i+=1){if(i==389)return i;}return 1;}").get());
		}
	};
}