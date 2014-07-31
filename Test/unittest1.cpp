#include "stdafx.h"
#include "CppUnitTest.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Test
{
	TSyntaxAnalyzer* syntax;
	TTime* time;

	std::vector<TStaticValue> static_objects;

	TSMethod* CreateMethod(char* code)
	{
		try
		{
			TMethod* m = new TMethod(syntax->base_class);
			syntax->lexer.ParseSource(code);
			m->AnalyzeSyntax(syntax->lexer);
			syntax->lexer.GetToken(TTokenType::Done);

			TSMethod* ms = new TSMethod(syntax->sem_base_class, m);
			ms->Build();

			std::vector<TSClassField*> static_fields;
			std::vector<TSLocalVar*> static_variables;

			ms->LinkSignature(&static_fields, &static_variables);
			ms->LinkBody(&static_fields, &static_variables);
			ms->CalculateParametersOffsets();

			std::vector<TSClass*> owners;
			syntax->sem_base_class->CalculateSizes(owners);

			InitializeStaticClassFields(static_fields, static_objects);
			InitializeStaticVariables(static_variables, static_objects);

			return ms;
		}
		catch (std::string s)
		{
			printf(s.c_str());
			Logger::WriteMessage(s.c_str());
			//throw;
		}
	}
	TSClass* CreateClass(char* code)
	{
		try
		{
			TClass* cl = new TClass(syntax->base_class);
			syntax->lexer.ParseSource(code);
			cl->AnalyzeSyntax(syntax->lexer);
			syntax->lexer.GetToken(TTokenType::Done);

			TSClass* scl = new TSClass(syntax->sem_base_class, cl);
			syntax->sem_base_class->AddClass(scl);
			scl->Build();

			std::vector<TSClassField*> static_fields;
			std::vector<TSLocalVar*> static_variables;

			scl->LinkSignature(&static_fields, &static_variables);
			scl->InitAutoMethods();
			scl->LinkBody(&static_fields, &static_variables);
			scl->CheckForErrors();
			scl->CalculateMethodsSizes();

			std::vector<TSClass*> owners;
			syntax->sem_base_class->CalculateSizes(owners);

			InitializeStaticClassFields(static_fields, static_objects);
			InitializeStaticVariables(static_variables, static_objects);

			return scl;
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
		ms->Run(static_objects, params, result, object);
		return result;
	}
	TStackValue RunMethod(TSMethod* ms)
	{
		std::vector<TStackValue> params;
		TStackValue result, object;
		ms->Run(static_objects, params, result, object);
		return result;
	}
	TStackValue RunClassMethod(TSClass* scl, char* method_name)
	{
		std::vector<TSMethod*> methods;
		scl->GetMethods(methods, syntax->lexer.GetIdFromName(method_name));
		TSMethod* ms = methods[0];

		std::vector<TStackValue> params;
		TStackValue result, object;
		ms->Run(static_objects, params, result, object);
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
			TFileData file("../../Source/NativeTypes/base_types.bscript", "rb");
			source = file.ReadAll();
			source[file.GetSize()] = '\0';
		}
		syntax = new TSyntaxAnalyzer();
		syntax->Compile((char*)(("class Script{"+std::string(source)+"}").c_str()), *time);
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
			Assert::AreEqual(3 * 6, *(int*)RunCode("func static Test:int{int a,b;a=3;b=6;return a*b;}").get());
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
			TSClass* cl = CreateClass(
				"class TestClass {"
				"class SubClass {"
				"int a,b,c;"
				"constr(int a1,int b1, int c1){}"
				"func Init{a=1;b=2;c=3;}"
				"}"
				"func static Test:int"
				"{"
				"SubClass s;s.Init();return s.b;"
				"}}");
			Assert::AreEqual((int)2, *(int*)RunClassMethod(cl, "Test").get());

			TSClass* cl2 = CreateClass(
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
				"}}");
			Assert::AreEqual((int)4, *(int*)RunClassMethod(cl2, "Test").get());
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

			Assert::AreEqual(asin(1.0f), *(float*)RunCode("func static Test:float{return Asin(1.0);}").get(), 0.0001f);
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
		TEST_METHOD(RecursiveTest)
		{
			TSClass* cl2 = NULL;
			Assert::IsNotNull(cl2 = CreateClass(
				"class TestClass {\n"
				"func static Factorial(int i):int\n"
				"{\n"
				"	if(i==0)return 1;else return i*Factorial(i-1);"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"return Factorial(5);\n"
				"}}"));
			Assert::AreEqual((int)1 * 2 * 3 * 4 * 5, *(int*)RunClassMethod(cl2, "Test").get());
		}
	};
	TEST_CLASS(StaticVariablesTesting)
	{
	public:
		TEST_METHOD(BasicClassFieldInit)
		{
			TSClass* cl2 = CreateClass(
				"class TestClass {\n"
				"class SubClass {\n"
				"int a,b,c;\n"
				"constr{a=3;b=5;c=9;}\n"
				"}\n"
				"class SubClass2 {\n"
				"SubClass static test_static;\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"return SubClass2.test_static.b;\n"
				"}}");
			Assert::AreEqual((int)5, *(int*)RunClassMethod(cl2, "Test").get());
		}
		TEST_METHOD(BasicClassField)
		{
			TSClass* cl2 = CreateClass(
				"class TestClass {\n"
				"class SubClass {\n"
				"int a,b,c;\n"
				"constr{a=3;b=5;c=9;}\n"
				"}\n"
				"class SubClass2 {\n"
				"SubClass tested_field,a,b;\n"
				"SubClass static test_static;\n"
				"constr{test_static.a+=1;test_static.c+=2;}\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"SubClass2.test_static.a+=1;"
				"return SubClass2.test_static.a;\n"
				"}\n"
				"func static Test2:int\n"
				"{\n"
				"SubClass2.test_static.a+=1;"
				"return SubClass2.test_static.a;\n"
				"}}");
			Assert::AreEqual((int)4, *(int*)RunClassMethod(cl2, "Test").get());
			Assert::AreEqual((int)5, *(int*)RunClassMethod(cl2, "Test2").get());
		}
		TEST_METHOD(BasicLocalVariable)
		{
			TSMethod* ms = CreateMethod("func static Test:int{int static i=5+4-8;i+=1;return i;}");
			Assert::AreEqual(2, *(int*)RunMethod(ms).get());
			Assert::AreEqual(3, *(int*)RunMethod(ms).get());
			Assert::AreEqual(4, *(int*)RunMethod(ms).get());
		}
	};
	TEST_CLASS(ConstrCopyDestrTesting)
	{
	public:
		TEST_METHOD(ClassConstructorTest)
		{
			TSClass* cl2 = CreateClass(
				"class TestClass {"
				"class SubClass {"
				"int a,b,c;"
				"constr{a=3;b=5;c=9;}"
				"}"
				"class SubClass2 {"
				"SubClass tested_field,a,b;"
				"}"
				"func static Test:int"
				"{"
				"SubClass2 s;return s.tested_field.b;"
				"}"
				"func static Test2:int"
				"{"
				"SubClass2 s;return s.a.c;"
				"}}");
			Assert::AreEqual((int)5, *(int*)RunClassMethod(cl2, "Test").get());
			Assert::AreEqual((int)9, *(int*)RunClassMethod(cl2, "Test2").get());
		}
		TEST_METHOD(ClassCopyConstructorTest)
		{
			TSClass* cl2 = CreateClass(
				"class TestClass {"
				"class SubClass {"
				"int a,b,c;"
				"constr{a=3;b=5;c=9;}"
				"constr(SubClass& copy_from){a=copy_from.a+1;b=copy_from.b+1;c=copy_from.c+1;}"
				"}"
				"class SubClass2 {"
				"SubClass tested_field,a,b;"
				"}"
				"func static Test3:int"
				"{"
				"SubClass2 s0;"
				"SubClass2 s1(s0); return s1.a.c;"
				"}}");
			Assert::AreEqual((int)10, *(int*)RunClassMethod(cl2, "Test3").get());
		}
		TEST_METHOD(ClassDestructorTest)
		{
			TSClass* cl2 = NULL;
			Assert::IsNotNull(cl2 = CreateClass(
				"class TestClass {\n"
				"int static destr_count;\n"
				"int static constr_count;\n"
				"int static copy_constr_count;\n"
				"class SubClass {\n"
				"int a,b,c;\n"
				"constr{constr_count+=1;a=3;b=5;c=9;}\n"
				"constr(SubClass& copy_from){copy_constr_count+=1;a=copy_from.a+1;b=copy_from.b+1;c=copy_from.c+1;}\n"
				"destr{destr_count+=1;}\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"constr_count=0;\n"
				"copy_constr_count=0;\n"
				"destr_count=0;\n"
				"SubClass s0;\n"
				"SubClass s1(s0); return s1.a;\n"
				"}\n"
				"func static GetDestrCount:int\n"
				"{\n"
				"return destr_count;\n"
				"}\n"
				"func static GetConstrCount:int\n"
				"{\n"
				"return constr_count;\n"
				"}\n"
				"func static GetCopyConstrCount:int\n"
				"{\n"
				"return copy_constr_count;\n"
				"}}"));
			Assert::AreEqual((int)4, *(int*)RunClassMethod(cl2, "Test").get());
			Assert::AreEqual((int)2, *(int*)RunClassMethod(cl2, "GetDestrCount").get());
			Assert::AreEqual((int)1, *(int*)RunClassMethod(cl2, "GetConstrCount").get());
			Assert::AreEqual((int)1, *(int*)RunClassMethod(cl2, "GetCopyConstrCount").get());
		}
		TEST_METHOD(UserConstrCallAutoConstrTest)
		{
			Assert::Fail();
		}
		TEST_METHOD(UserCopyConstrCallAutoConstrTest)
		{
			Assert::Fail();
		}
		TEST_METHOD(UserDestructorCallAutoDestTest)
		{
			Assert::Fail();
		}
	};
	TEST_CLASS(DynArrayTesting)
	{
	public:
		TEST_METHOD(InitializationTest)
		{
			Assert::AreEqual(0, *(int*)RunCode("func static Test:int{TDynArray<int> s;return s.size();}").get());
			Assert::AreEqual(2, *(int*)RunCode("func static Test:int{TDynArray<int> s;s.resize(2);return s.size();}").get());
		}
		TEST_METHOD(ArrayOfArrayInitializationTest)
		{
			Assert::AreEqual(23 + 54 + 2 + 34, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	TDynArray<TDynArray<int>> s;"
				"	s.resize(3);"
				"	s[0].resize(23);"
				"	s[1].resize(54);"
				"	s[2].resize(2);"
				"	s.resize(34);"
				"	return s[0].size()+s[1].size()+s[2].size()+s.size();"
				"}"
				).get());
		}
		TEST_METHOD(CopyConstructorTest)
		{
			Assert::AreEqual(6, *(int*)RunCode(
				"func static Test:int\n"
				"{\n"
				"	TDynArray<int> s;"
				"	s.resize(2);\n"
				"	TDynArray<int> s1(s),s2,s3(s),s4;"
				"	TDynArray<int> sc(s);"
				"	return sc.size()+s1.size()+s3.size();"
				"}"
				).get());

			Assert::AreEqual(23 + 54 + 2 + 4, *(int*)RunCode(
				"func static Test:int\n"
				"{\n"
				"	TDynArray<TDynArray<int>> s;\n"
				"	s.resize(3);\n"
				"	s[0].resize(23);\n"
				"	s[1].resize(54);\n"
				"	s[2].resize(2);\n"
				"	s.resize(4);\n"
				"	TDynArray<TDynArray<int>> sc(s);\n"
				"	return sc[0].size()+sc[1].size()+sc[2].size()+sc.size();\n"
				"}"
				).get());

			Assert::AreEqual(23 + 54 + 2 + 54, *(int*)RunCode(
				"func static Test:int\n"
				"{\n"
				"	TDynArray<TDynArray<int>> s;\n"
				"	s.resize(54);\n"
				"	s[0].resize(23);\n"
				"	s[1].resize(54);\n"
				"	s[2].resize(2);\n"
				"	TDynArray<TDynArray<int>> sc(s);\n"
				"	s.resize(2);\n"
				"	return sc[0].size()+sc[1].size()+sc[2].size()+sc.size();\n"
				"}"
				).get());
		}
		TEST_METHOD(AssignTest)
		{
			Assert::AreEqual(10 + 324 - 213, *(int*)RunCode(
				"func static Test:int\n"
				"{\n"
				"	TDynArray<int> s;\n"
				"	s.resize(10);\n"
				"	s[0]=324;\n"
				"	s[9]=-213;\n"
				"	TDynArray<int> sc;\n"
				"	sc=s;\n"
				"	return sc.size()+sc[0]+sc[9];\n"
				"}"
				).get());
			Assert::AreEqual(10 + 432 - 384, *(int*)RunCode(
				"func static Test:int\n"
				"{\n"
				"	TDynArray<TDynArray<int>> s;\n"
				"	s.resize(10);\n"
				"	s[0].resize(2);\n"
				"	s[9].resize(3);\n"
				"	s[0][1]=432;\n"
				"	s[9][2]=-384;\n"
				"	TDynArray<TDynArray<int>> sc;\n"
				"	sc=s;\n"
				"	return sc.size()+sc[0][1]+sc[9][2];\n"
				"}"
				).get());
			Assert::AreEqual(10 + 432 - 384, *(int*)RunCode(
				"func static Test:int\n"
				"{\n"
				"	TDynArray<TDynArray<int>> s;\n"
				"	s.resize(10);\n"
				"	s[0].resize(2);\n"
				"	s[9].resize(3);\n"
				"	s[0][1]=432;\n"
				"	s[9][2]=-384;\n"
				"	TDynArray<TDynArray<int>> sc;\n"
				"	sc=s;\n"
				"	return sc.size()+sc[0][1]+sc[9][2];\n"
				"}"
				).get());
		}
		TEST_METHOD(AssignInCycle)
		{
			Assert::AreEqual(111, *(int*)RunCode(
				"func static Test:int\n"
				"{\n"
				"	TDynArray<TDynArray<int>> s;\n"
				"	s.resize(100);\n"
				"	for(int i=0;i<100;i+=1){s[i].resize(i+1);for(int k=0;k<i+1;k+=1)s[i][k]=i+k;}"
				"	TDynArray<TDynArray<int>> sc;\n"
				"	sc=s;\n"
				"	return sc.size()+sc[0][0]+sc[9][2];\n"
				"}"
				).get());
		}
		TEST_METHOD(GetElementTest)
		{
			Assert::AreEqual(23, *(int*)RunCode("func static Test:int{TDynArray<int> s;s.resize(1);s[0]=23;return s[0];}").get());
			Assert::AreEqual(49573, *(int*)RunCode("func static Test:int{TDynArray<int> s;s.resize(1000);s[999]=49573;s.resize(1001);return s[999];}").get());
		}
	};
	TEST_CLASS(TempatesTesting)
	{
	public:
		TEST_METHOD(ValueParameters)
		{
			TSClass* cl2 = NULL;
			Assert::IsNotNull(cl2 = CreateClass(
				"class TestClass {\n"
				"class TemplateClass<T,Size> {\n"
				"multifield(Size) T value;\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	TemplateClass<int, 5> v;\n"
				"	return 1;\n"
				"}}"));
			Assert::AreEqual((int)1, *(int*)RunClassMethod(cl2, "Test").get());
		}
	};
	TEST_CLASS(StaticArrayTesting)
	{
	public:
		TEST_METHOD(InitializationTest)
		{
			Assert::AreEqual(3, *(int*)RunCode("func static Test:int{TStaticArray<int,3> s;return s.size();}").get());
			Assert::AreEqual(34, *(int*)RunCode("func static Test:int{TStaticArray<int,34> s;return s.size();}").get());
		}
		TEST_METHOD(ArrayOfArrayInitializationTest)
		{
			Assert::AreEqual(43+243, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	TStaticArray<TStaticArray<int,4>,4> s;"
				"	s[0][0]=43;"
				"	s[3][3]=243;"
				"	return s[0][0]+s[3][3];"
				"}"
				).get());
		}
		TEST_METHOD(ArrayOfArrayInCycleInitializationTest)
		{
			Assert::AreEqual(3+5, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	TStaticArray<TStaticArray<int,4>,4> s;"
				"	for(int i=0;i<4;i+=1){for(int k=0;k<4;k+=1)s[i][k]=i+k;}"
				"	return s[2][1]+s[3][2];"
				"}"
				).get());
		}
		TEST_METHOD(ElementsConstructor)
		{
			Assert::Fail();
		}
		TEST_METHOD(ConstructorDestructorCount)
		{
			TSClass* cl2 = NULL;
			Assert::IsNotNull(cl2 = CreateClass(
				"class TestClass {\n"
				"int static constr_count;"
				"int static destr_count;"
				"class TemplateClass {\n"
				"	int v;\n"
				"	constr {v=347;constr_count+=1;}\n"
				"	destr {v=0;destr_count+=1;}\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	constr_count=0;\n"
				"	destr_count=0;\n"
				"	TStaticArray<TemplateClass,4> s;\n"
				"	return s[0].v+s[1].v+s[2].v+s[3].v;\n"
				"}\n"
				"func static Test2:int\n"
				"{\n"
				"	return constr_count;\n"
				"}\n"
				"func static Test3:int\n"
				"{\n"
				"	return destr_count;\n"
				"}}"));
			Assert::AreEqual((int)347 * 4, *(int*)RunClassMethod(cl2, "Test").get());
			Assert::AreEqual((int) 4, *(int*)RunClassMethod(cl2, "Test2").get());
			Assert::AreEqual((int)4, *(int*)RunClassMethod(cl2, "Test3").get());
		}
		TEST_METHOD(ElementsDestructor)
		{
			Assert::Fail();
		}
		TEST_METHOD(CopyConstructor)
		{
			Assert::AreEqual(3 + 5, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	TStaticArray<TStaticArray<int,4>,4> s;"
				"	for(int i=0;i<4;i+=1){for(int k=0;k<4;k+=1)s[i][k]=i+k;}"
				"	TStaticArray<TStaticArray<int,4>,4> sc(s);"
				"	return sc[2][1]+sc[3][2];"
				"}"
				).get());
		}
		TEST_METHOD(AssignTest)
		{
			Assert::AreEqual(3 + 5, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	TStaticArray<TStaticArray<int,4>,4> s;"
				"	for(int i=0;i<4;i+=1){for(int k=0;k<4;k+=1)s[i][k]=i+k;}"
				"	TStaticArray<TStaticArray<int,4>,4> sc;"
				"	sc=s;"
				"	return sc[2][1]+sc[3][2];"
				"}"
				).get());
		}
		TEST_METHOD(CustomAssignTest)
		{
			Assert::Fail();
		}
		TEST_METHOD(GetElementTest)
		{
			Assert::AreEqual(7, *(int*)RunCode("func static Test:int{TStaticArray<int,3> s;s[0]=5;s[2]=2;return s[0]+s[2];}").get());
		}
	};
	TEST_CLASS(EnumerationTesting)
	{
	public:
		TEST_METHOD(Initialization)
		{
			TSClass* cl2 = NULL;
			Assert::IsNotNull(cl2 = CreateClass(
				"class TestClass {\n"
				"enum TestEnum{One,Two,Three}"
				"func static Test:int\n"
				"{\n"
				"	TestEnum e;\n"
				"	return 1;\n"
				"}}"));
		}
	};
	TEST_CLASS(ArraysSpecialSyntaxTesting)
	{
	public:
		TEST_METHOD(StaticInitialization)
		{
			Assert::AreEqual(7, *(int*)RunCode("func static Test:int{int[3] s;s[0]=5;s[2]=2;return s[0]+s[2];}").get());
		}
		TEST_METHOD(DynamicInitialization)
		{
			Assert::AreEqual(7, *(int*)RunCode("func static Test:int{int[] s;s.resize(3);s[0]=5;s[2]=2;return s[0]+s[2];}").get());
		}
		TEST_METHOD(MixedInitialization)
		{
			Assert::AreEqual(7, *(int*)RunCode("func static Test:int{int[][3] s;s.resize(3);s[0][2]=5;s[2][1]=2;return s[0][2]+s[2][1];}").get());
		}
		TEST_METHOD(MixedWithSubclassInitialization)
		{
				TSClass* cl2 = NULL;
				Assert::IsNotNull(cl2 = CreateClass(
					"class TestClass {\n"
					"int static constr_count;\n"
					"int static destr_count;\n"
					"class TemplateClass {\n"
					"	int v;\n"
					"	class Sub{int member;}\n"
					"	constr {v=347;constr_count+=1;}\n"
					"	destr {v=0;destr_count+=1;}\n"
					"}\n"
					"func static sum(TemplateClass.Sub[][3] v):int\n"
					"{\n"
					"	int result=0;\n"
					"	for(int i=0;i<v.size();i+=1)result+=v[i][0]+v[i][1]+v[i][1];\n"
					"	return result;\n"
					"}\n"
					"func static Test:int\n"
					"{\n"
					"	TestClass.TemplateClass.Sub[][3] arr;\n"
					"	arr.resize(100);\n"
					"	for(int i=0;i<100;i+=1){arr[i][0].member=0;arr[i][1].member=1;arr[i][2].member=i;}\n"
					"	return sum(arr);\n"
					"}}"));
				Assert::AreEqual((int)1, *(int*)RunClassMethod(cl2, "Test").get());
		}
	};
	//TODO Вызов авто коснтруктора из пользовательского
}