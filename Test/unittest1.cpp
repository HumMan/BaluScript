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
			ms->LinkSignature();
			ms->LinkBody();
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
			scl->LinkSignature();
			scl->LinkBody();
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
		ms->Run(params, result, object);
		return result;
	}
	TStackValue RunClassWithCode(char* code)
	{
		TSMethod* ms = CreateClass(code);
		std::vector<TStackValue> params;
		TStackValue result, object;
		ms->Run(params, result, object);
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
			TFileData file("c:/Users/Alexsey/Documents/GitHub/BaluScript/Source/NativeTypes/int_type.bscript", "rb");
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
			//Assert::AreEqual(5, *(int*)RunCode("func static Test:int{int s,b;b=5 s=b;return s;}").get());
			//Assert::AreEqual(5, *(int*)RunCode("func static Test:int{int s(5);return s;}").get());
			//Assert::AreEqual(5, *(int*)RunCode("func static Test:int{int s=5;return s;}").get());
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
}