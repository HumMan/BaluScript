#include "stdafx.h"
#include "CppUnitTest.h"

#include "../Source/memleaks.h"

#include <fstream>
#include <streambuf>

#include "../Source/NativeTypes/vec.h"

#include "../Source/NativeTypes/base_types.h"

#include "../Source/SemanticInterface/SemanticTreeApi.h"
#include "../Source/TreeRunner/TreeRunner.h"

#include <windows.h>
#include <delayimp.h>

#pragma comment(lib, "delayimp")  

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace Test
{
	ISyntaxAnalyzer* syntax;

	std::vector<TStaticValue> *static_objects;

	TRefsList* refs_list;

	TGlobalRunContext global_context;

	//std::vector < SemanticApi::ISMethod* > *smethods;

	SemanticApi::ISMethod* CreateMethod(const char* code)
	{		
		try
		{
			syntax->Compile(code);
			std::vector<SemanticApi::ISMethod*> methods;
			syntax->GetCompiledBaseClass()->GetMethods(methods);
			return methods.back();
		}
		catch (std::string s)
		{
			printf(s.c_str());
			Logger::WriteMessage(s.c_str());
		}
		return nullptr;
	}
	SemanticApi::ISClass* CreateClass(char* code)
	{
		try
		{
			if (static_objects != nullptr)
				delete static_objects;
			if (syntax != nullptr)
				ISyntaxAnalyzer::Destroy(syntax);
			if (refs_list != nullptr)
				delete refs_list;

			static_objects = new std::vector<TStaticValue>();
			syntax = ISyntaxAnalyzer::Create();
			refs_list = new TRefsList();
			
			global_context = TGlobalRunContext(static_objects, refs_list);

			syntax->Compile(code);
			auto sem_class = syntax->GetCompiledBaseClass();
			
			TreeRunner::InitializeStaticClassFields(syntax->GetStaticFields(), global_context);
			TreeRunner::InitializeStaticVariables(syntax->GetStaticVariables(), global_context);
		}
		catch (std::string s)
		{
			printf(s.c_str());
			Logger::WriteMessage(s.c_str());
		}
		return nullptr;
	}
	TStackValue RunCode(const char* code)
	{
		static_objects = new std::vector<TStaticValue>();
		refs_list = new TRefsList();
		global_context = TGlobalRunContext(static_objects, refs_list);
		syntax = ISyntaxAnalyzer::Create();

		SemanticApi::ISMethod* ms = CreateMethod(code);
		std::vector<TStackValue> params;
		TStackValue result, object;
		TMethodRunContext method_run_context(global_context, &params, &result, &object);

		TreeRunner::InitializeStaticClassFields(syntax->GetStaticFields(), global_context);
		TreeRunner::InitializeStaticVariables(syntax->GetStaticVariables(), global_context);
		TreeRunner::Run(ms, method_run_context);
		TreeRunner::DeinitializeStatic(global_context);

		ISyntaxAnalyzer::Destroy(syntax);
		delete static_objects;
		delete refs_list;

		refs_list = nullptr;
		syntax = nullptr;
		static_objects = nullptr;

		return result;
	}
	TStackValue RunMethod(SemanticApi::ISMethod* ms)
	{
		std::vector<TStackValue> params;
		TStackValue result, object;
		TMethodRunContext method_run_context(global_context, &params, &result, &object);
		TreeRunner::Run(ms,method_run_context);
		return result;
	}
	
	TStackValue RunClassMethod(const char* method_name)
	{
		SemanticApi::ISMethod* ms = syntax->GetMethod(method_name);

		std::vector<TStackValue> params;
		TStackValue result, object;
		TMethodRunContext method_run_context(global_context, &params, &result, &object);

		TreeRunner::Run(ms, method_run_context);

		return result;
	}

	TStackValue RunClassMethod(SemanticApi::ISClass* scl, const char* method_name)
	{
		std::vector<SemanticApi::ISMethod*> methods;

		return RunClassMethod((std::string("func static Script.TestClass.") + method_name + ":int").c_str());
	}

	void Init()
	{
		//setlocale(LC_ALL, "windows");
		//printf("Compiling ... \n");

		//static_objects = new std::vector<TStaticValue>();
		//smethods = new std::vector<std::unique_ptr<TSMethod>>();

		//std::string source = base_types;
		//syntax = new TSyntaxAnalyzer();
		//syntax->Compile((char*)(("class Script{" + source + "}").c_str()));
	}
	void Cleanup()
	{
		if (static_objects != nullptr)
		{
			TreeRunner::DeinitializeStatic(global_context);
			delete static_objects;
			static_objects = nullptr;
		}
		if (syntax != nullptr)
		{
			ISyntaxAnalyzer::Destroy(syntax);
			syntax = nullptr;
		}
		if (refs_list != nullptr)
		{
			delete refs_list;
			refs_list = nullptr;
		}
	}
	TEST_MODULE_INITIALIZE(ModuleInitialize)
	{
	}

	TEST_MODULE_CLEANUP(ModuleCleanup)
	{
		//если была утечка внутри dll, то _CrtDumpMemoryLeaks упадёт с ошибкой
		//auto unload_result = __FUnloadDelayLoadedDLL2("BaluScript.dll");
		_CrtDumpMemoryLeaks();
	}
	void BaseTypesTestsInitialize()
	{
		Init();
	}
	void BaseTypesTestsCleanup()
	{
		Cleanup();
	}
	TEST_CLASS(IntTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
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
			SemanticApi::ISClass* cl = CreateClass(
				"class TestClass {"
				"class SubClass {"
				"int a,b,c;"
				"copy(int a1,int b1, int c1){}"
				"func Init{a=1;b=2;c=3;}"
				"}"
				"func static Test:int"
				"{"
				"SubClass s;s.Init();return s.b;"
				"}}");
			Assert::AreEqual((int)2, *(int*)RunClassMethod(cl, "Test").get());

			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {"
				"func static Make(int& a,int& b,int& c){a=2;b=4;c=6;}"
				"class SubClass {"
				"int a,b,c;"
				"copy(int a1,int b1, int c1){}"
				"func Init{Make(a,b,c);}"
				"}"
				"func static Test:int"
				"{"
				"SubClass s;s.Init();return s.b;"
				"}}");
			Assert::AreEqual((int)4, *(int*)RunClassMethod(nullptr, "Test").get());
		}
	};
	TEST_CLASS(FloatTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
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
	TEST_CLASS(Vec2Testing)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(Vec2Constr)
		{
			using namespace BaluLib;
			TVec2 t;
			t = *(TVec2*)RunCode("func static Test:vec2{vec2 s; s=5.0;return s;}").get();
			Assert::AreEqual(5.0f, t[0]);
			Assert::AreEqual(5.0f, t[1]);
			t = *(TVec2*)RunCode("func static Test:vec2{vec2 s,b;b=5.0;s=b;return s;}").get();
			Assert::AreEqual(5.0f, t[0]);
			Assert::AreEqual(5.0f, t[1]);
			t = *(TVec2*)RunCode("func static Test:vec2{vec2 s(5);return s;}").get();
			Assert::AreEqual(5.0f, t[0]);
			Assert::AreEqual(5.0f, t[1]);
			t = *(TVec2*)RunCode("func static Test:vec2{vec2 s(8+2-8,-2);return s;}").get();
			Assert::AreEqual(2.0f, t[0]);
			Assert::AreEqual(-2.0f, t[1]);
		}
		TEST_METHOD(Vec2Func)
		{
			using namespace BaluLib;
			TVec2 t;
			t = *(TVec2*)RunCode("func static Test:vec2{vec2 s; s=5.0;vec2 a; a=6.0;return s+a;}").get();
			Assert::AreEqual(11.0f, t[0]);
			Assert::AreEqual(11.0f, t[1]);
		}
	};
	TEST_CLASS(Vec2iTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(Vec2iConstr)
		{
			using namespace BaluLib;
			TVec2i t;
			t = *(TVec2i*)RunCode("func static Test:vec2i{vec2i s; s=5.0;return s;}").get();
			Assert::AreEqual(5, t[0]);
			Assert::AreEqual(5, t[1]);
			t = *(TVec2i*)RunCode("func static Test:vec2i{vec2i s,b;b=5.0;s=b;return s;}").get();
			Assert::AreEqual(5, t[0]);
			Assert::AreEqual(5, t[1]);
			t = *(TVec2i*)RunCode("func static Test:vec2i{vec2i s(5);return s;}").get();
			Assert::AreEqual(5, t[0]);
			Assert::AreEqual(5, t[1]);
			t = *(TVec2i*)RunCode("func static Test:vec2i{vec2i s(8+2-8,-2);return s;}").get();
			Assert::AreEqual(2, t[0]);
			Assert::AreEqual(-2, t[1]);
		}
		TEST_METHOD(Vec2iFunc)
		{
			using namespace BaluLib;
			TVec2i t;
			t = *(TVec2i*)RunCode("func static Test:vec2i{vec2i s; s=5;vec2i a; a=6;return s+a;}").get();
			Assert::AreEqual(11, t[0]);
			Assert::AreEqual(11, t[1]);
		}
	};
	TEST_CLASS(PtrTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(PtrTestingSimpleType)
		{
			CreateClass(
				"class TestClass {\n"
				"func static Test:int\n"
				"{\n"
				"	TPtr<int> p;\n"
				"	p.data()=45;\n"
				"	return p.data();\n"
				"}}");
			Assert::AreEqual((int)45, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(PtrTestingToDynType)
		{
			CreateClass(
				"class TestClass {\n"
				"func static Test:int\n"
				"{\n"
				"	TPtr<TDynArray<int>> p;\n"
				"	p.data().resize(1000);\n"
				"	p.data()[45]=11;\n"
				"	return p.data()[45];\n"
				"}}");
			Assert::AreEqual((int)11, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(PtrTestingToDynTypeCopy)
		{
			CreateClass(
				"class TestClass {\n"
				"func static Test:int\n"
				"{\n"
				"	TPtr<TDynArray<int>> p;\n"
				"	p.data().resize(1000);\n"
				"	p.data()[45]=11;\n"
				"	var p2=p;\n"
				"	return p2.data()[45];\n"
				"}}");
			Assert::AreEqual((int)11, *(int*)RunClassMethod(nullptr, "Test").get());
		}
	};
	TEST_CLASS(StatementsTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(VarKeyword)
		{
			CreateClass(
				"class TestClass {\n"
				"func static Test:int\n"
				"{\n"
				"	vec2 v(3,5),r(-2,8);\n"
				"	var s=(v+r);\n"
				"	int result(Dot(s,v-r));\n"
				"	return result;\n"
				"}}");
			Assert::AreEqual((int)-34, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(VarKeyword2)
		{
			CreateClass(
				"class TestClass {\n"
				"func static Test:int\n"
				"{\n"
				"	float v(3);\n"
				"	var s=v;\n"
				"	int result(s);\n"
				"	return result;\n"
				"}}");
			Assert::AreEqual((int)3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(VarKeyword3)
		{
			CreateClass(
				"class TestClass {\n"
				"func static Test:int\n"
				"{\n"
				"	int v=3;\n"
				"	var s=v;\n"
				"	return s;\n"
				"}}");
			Assert::AreEqual((int)3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(VarKeyword4)
		{
			CreateClass(
				"class TestClass {\n"
				"func static Test:int\n"
				"{\n"
				"	var s=3;\n"
				"	return s;\n"
				"}}");
			Assert::AreEqual((int)3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(VarKeyword5)
		{
			CreateClass(
				"class TestClass {\n"
				"func static Test:int\n"
				"{\n"
				"	vec2 v(3,5),r(-2,8);\n"
				"	var s=Dot((v+r),v-r);\n"
				"	int result(s);\n"
				"	return result;\n"
				"}}");
			Assert::AreEqual((int)-34, *(int*)RunClassMethod(nullptr, "Test").get());
		}
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
			Assert::AreEqual(401, *(int*)RunCode("func static Test:int{int i=0;while(i<=400){++i;}return i;}").get());
		}
		TEST_METHOD(ForTest)
		{
			Assert::AreEqual(13, *(int*)RunCode("func static Test:int{int s=3;for(int i=0;i<5;i+=1)s+=i;return s;}").get());
			Assert::AreEqual(389, *(int*)RunCode("func static Test:int{for(int i=0;i<500;++i){if(i==389)return i;}return 1;}").get());
		}
		TEST_METHOD(RecursiveTest)
		{
			
			CreateClass(
				"class TestClass {\n"
				"func static Factorial(int i):int\n"
				"{\n"
				"	if(i==0)return 1;else return i*Factorial(i-1);"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"return Factorial(5);\n"
				"}}");
			Assert::AreEqual((int)1 * 2 * 3 * 4 * 5, *(int*)RunClassMethod(nullptr, "Test").get());
		}
	};
	TEST_CLASS(StaticVariablesTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(BasicClassFieldInit)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {\n"
				"class SubClass {\n"
				"int a,b,c;\n"
				"default{a=3;b=5;c=9;}\n"
				"}\n"
				"class SubClass2 {\n"
				"SubClass static test_static;\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"return (new SubClass2).test_static.b;\n"
				"}}");
			Assert::AreEqual((int)5, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(BasicClassField)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {\n"
				"class SubClass {\n"
				"int a,b,c;\n"
				"default{a=3;b=5;c=9;}\n"
				"}\n"
				"class SubClass2 {\n"
				"SubClass tested_field,a,b;\n"
				"SubClass static test_static;\n"
				"default{test_static.a+=1;test_static.c+=2;}\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"(new SubClass2).test_static.a+=1;"
				"return (new SubClass2).test_static.a;\n"
				"}\n"
				"func static Test2:int\n"
				"{\n"
				"(new SubClass2).test_static.a+=1;"
				"return (new SubClass2).test_static.a;\n"
				"}}");
			Assert::AreEqual((int)4, *(int*)RunClassMethod(nullptr, "Test").get());
			Assert::AreEqual((int)5, *(int*)RunClassMethod(nullptr, "Test2").get());
		}
		TEST_METHOD(BasicStaticClassField)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {\n"
				"class SubClass2 {\n"
				"int static test_static;\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"(new SubClass2).test_static=3;"
				"return (new SubClass2).test_static;\n"
				"}}");
			Assert::AreEqual((int)3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(BasicLocalVariable)
		{
			CreateClass("func static Test:int{int static i=5+4-8;i+=1;return i;}");
			Assert::AreEqual(2, *(int*)RunClassMethod("func static Script.Test:int").get());
			Assert::AreEqual(3, *(int*)RunClassMethod("func static Script.Test:int").get());
			Assert::AreEqual(4, *(int*)RunClassMethod("func static Script.Test:int").get());
		}
	};
	TEST_CLASS(ConstrCopyDestrTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(ClassConstructorTest)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {"
				"class SubClass {"
				"int a,b,c;"
				"default{a=3;b=5;c=9;}"
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
			Assert::AreEqual((int)5, *(int*)RunClassMethod(nullptr, "Test").get());
			Assert::AreEqual((int)9, *(int*)RunClassMethod(nullptr, "Test2").get());
		}
		TEST_METHOD(ClassCopyConstructorTest)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {"
				"class SubClass {"
				"int a,b,c;"
				"default{a=3;b=5;c=9;}"
				"copy(SubClass& copy_from){a=copy_from.a+1;b=copy_from.b+1;c=copy_from.c+1;}"
				"}"
				"class SubClass2 {"
				"SubClass tested_field,a,b;"
				"}"
				"func static Test3:int"
				"{"
				"SubClass2 s0;"
				"SubClass2 s1(s0); return s1.a.c;"
				"}}");
			Assert::AreEqual((int)10, *(int*)RunClassMethod(nullptr, "Test3").get());
		}
		TEST_METHOD(ClassDestructorTest)
		{
			CreateClass(
				"class TestClass {\n"
				"int static destr_count;\n"
				"int static constr_count;\n"
				"int static copy_constr_count;\n"
				"class SubClass {\n"
				"int a,b,c;\n"
				"default{constr_count+=1;a=3;b=5;c=9;}\n"
				"copy(SubClass& copy_from){copy_constr_count+=1;a=copy_from.a+1;b=copy_from.b+1;c=copy_from.c+1;}\n"
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
				"}}");
			Assert::AreEqual((int)4, *(int*)RunClassMethod("func static Script.TestClass.Test:int").get());
			Assert::AreEqual((int)2, *(int*)RunClassMethod("func static Script.TestClass.GetDestrCount:int").get());
			Assert::AreEqual((int)1, *(int*)RunClassMethod("func static Script.TestClass.GetConstrCount:int").get());
			Assert::AreEqual((int)1, *(int*)RunClassMethod("func static Script.TestClass.GetCopyConstrCount:int").get());
		}
		TEST_METHOD(UserConstrCallAutoConstrTest)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {\n"
				"class SubClass {\n"
				"TDynArray<int> arr;\n"
				"default{arr.resize(3);arr[0]=3;arr[1]=5;arr[2]=9;}\n"
				"}\n"
				"func static Sum(SubClass &obj):int\n"
				"{\n"
				"return obj.arr[0]+obj.arr[1]+obj.arr[2];\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	SubClass s;\n"
				"	return Sum(s);\n"
				"}}");
			Assert::AreEqual((int)3 + 5 + 9, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(UserCopyConstrCallAutoConstrTest)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {"
				"class SubClass {"
				"TDynArray<int> arr;"
				"default{arr.resize(3);arr[0]=3;arr[1]=5;arr[2]=9;}"
				"copy(int _a,int _b,int _c){arr.resize(3);arr[0]=_a;arr[1]=_b;arr[2]=_c;}"
				"}"
				"func static Sum(SubClass obj):int"
				"{"
				"return obj.arr[0]+obj.arr[1]+obj.arr[2];\n"
				"}"
				"func static Test:int"
				"{"
				"	SubClass s(1,2,3);\n"
				"	return Sum((new SubClass)(1,2,3));"
				"}}");
			Assert::AreEqual((int)1 + 2 + 3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(UserDestructorCallAutoDestTest)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {"
				"class SubClass {"
				"TDynArray<int> arr;"
				"default{arr.resize(3);arr[0]=3;arr[1]=5;arr[2]=9;}"
				"copy(int _a,int _b,int _c){arr.resize(3);arr[0]=_a;arr[1]=_b;arr[2]=_c;}"
				"destr{bool nothing_to_do=true;}"
				"}"
				"func static Sum(SubClass obj):int"
				"{"
				"return obj.arr[0]+obj.arr[1]+obj.arr[2];\n"
				"}"
				"func static Test:int"
				"{"
				"	SubClass s(1,2,3);\n"
				"	return Sum((new SubClass)(1,2,3));"
				"}}");
			Assert::AreEqual((int)1 + 2 + 3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(TempObjectConstructorTest)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {"
				"class SubClass {"
				"int a,b,c;"
				"default{a=3;b=5;c=9;}"
				"copy(int _a,int _b,int _c){a=_a;b=_b;c=_c;}"
				"}"
				"func static Sum(SubClass obj):int"
				"{"
				"return obj.a+obj.b+obj.c;"
				"}"
				"func static Test:int"
				"{"
				"return Sum((new SubClass)(1,2,3));"
				"}}");
			Assert::AreEqual((int)1+2+3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(ParameterRValueCopy)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {"
				"class SubClass {"
				"TDynArray<int> arr;"
				"default{arr.resize(3);arr[0]=3;arr[1]=5;arr[2]=9;}"
				"copy(int _a,int _b,int _c){arr.resize(3);arr[0]=_a;arr[1]=_b;arr[2]=_c;}"
				"destr{bool nothing_to_do=true;}"
				"}"
				"func static Sum(SubClass obj):int"
				"{"
				"return obj.arr[0]+obj.arr[1]+obj.arr[2];\n"
				"}"
				"func static Test:int"
				"{"
				"	SubClass s(1,2,3);\n"
				"	return Sum((new SubClass)(1,2,3));"
				"}}");
			Assert::AreEqual((int)1 + 2 + 3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(ParameterRValueDestructor)
		{
			SemanticApi::ISClass* cl2 = CreateClass(
				"class TestClass {"
				"bool static destructor_called;"
				"class SubClass {"
				"TDynArray<int> arr;"
				"default{arr.resize(3);arr[0]=3;arr[1]=5;arr[2]=9;}"
				"copy(int _a,int _b,int _c){arr.resize(3);arr[0]=_a;arr[1]=_b;arr[2]=_c;}"
				"destr{destructor_called=true;}"
				"}"
				"func static Sum(SubClass obj):int"
				"{"
				"return obj.arr[0]+obj.arr[1]+obj.arr[2];\n"
				"}"
				"func static GetDestrCalled:bool"
				"{"
				"return destructor_called;\n"
				"}"
				"func static Test:int"
				"{"
				"	destructor_called = false;"
				"	SubClass s(1,2,3);\n"
				"	return Sum((new SubClass)(1,2,3));"
				"}}");
			Assert::AreEqual((int)1 + 2 + 3, *(int*)RunClassMethod(nullptr, "Test").get());
			Assert::AreEqual((bool)true, *(bool*)RunClassMethod("func static Script.TestClass.GetDestrCalled:bool").get());
		}
	};
	TEST_CLASS(DynArrayTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
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
				"	for(int i=0;i<100;i+=1){s[i].resize(i+1);for(int k=0;k<i+1;++k)s[i][k]=i+k;}"
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
	TEST_CLASS(ConversionsTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(ConversionInLocalVarConstructor)
		{
			CreateClass(
				"class TestClass {\n"
				"class B {\n"
				"int a,b;\n"
				"conversion static (B& value):float {return value.a+value.b;}\n"
				"}\n"
				"func static Test:float\n"
				"{\n"
				"	B r;\n"
				"	r.a = 3;\n"
				"	r.b = -6;\n"
				"	float s(r);\n"
				"	return s;\n"
				"}}");
			Assert::AreEqual((float)-3, *(float*)RunClassMethod("func static Script.TestClass.Test:float").get());
		}
		TEST_METHOD(ConversionInLocalVarAssignInit)
		{
			CreateClass(
				"class TestClass {\n"
				"class B {\n"
				"int a,b;\n"
				"conversion static (B& value):int {return value.a+value.b;}\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	B r;\n"
				"	r.a = 3;\n"
				"	r.b = -6;\n"
				"	int s=r;\n"
				"	return s;\n"
				"}}");
			Assert::AreEqual((int)-3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(ConversionInLocalVarAssign)
		{
			CreateClass(
				"class TestClass {\n"
				"class B {\n"
				"int a,b;\n"
				"conversion static (B& value):int {return value.a+value.b;}\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	B r;\n"
				"	r.a = 3;\n"
				"	r.b = -6;\n"
				"	int s;\n"
				"	s=r;\n"
				"	return s;\n"
				"}}");
			Assert::AreEqual((int)-3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(ConversionInMethodCall)
		{
			CreateClass(
				"class TestClass {\n"
				"class B {\n"
				"int a,b;\n"
				"conversion static (B& value):int {return value.a+value.b;}\n"
				"}\n"
				"func static Do(int v):int\n"
				"{\n"
				"	return v;\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	B r;\n"
				"	r.a = 3;\n"
				"	r.b = -6;\n"
				"	return Do(r);\n"
				"}}");
			Assert::AreEqual((int)-3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(ConversionInConstructTempObject)
		{
			CreateClass(
				"class TestClass {\n"
				"class B {\n"
				"int a,b;\n"
				"copy(int use_a, int use_b){a=use_a;b=use_b;}\n"
				"conversion static (B& value):int {return value.a+value.b;}\n"
				"conversion static (B value):int {return value.a+value.b;}\n" //TODO доделать ссылку на временный объект
				"}\n"
				"func static Do(int v):int\n"
				"{\n"
				"	return v;\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	return Do((new B)(3,-6));\n"
				"}}");
			Assert::AreEqual((int)-3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(ConversionInOperatorCall)
		{
			CreateClass(
				"class TestClass {\n"
				"class B {\n"
				"int a,b;\n"
				"copy(int use_a, int use_b){a=use_a;b=use_b;}\n"
				"conversion static (B value):int {return value.a+value.b;}\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	return 30 + (new B)(3,-6);\n"
				//"}\n"
				//"func static Test2:int\n"
				//"{\n"
				//"	return new B(3,-6) + 30;\n"
				"}}");

			Assert::AreEqual((int)30 + 3 - 6, *(int*)RunClassMethod(nullptr, "Test").get());
			
			//TODO проверки не проходящие проверку
			//Assert::AreEqual((int)30 + 3 - 6, *(int*)RunClassMethod(nullptr, "Test2").get());
		}
	};
	TEST_CLASS(OperatorsOverloadingTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(ArithmenticOperatorsOverload)
		{
			CreateClass(
				"class TestClass {\n"
				"class Vec2<T,Size> {\n"
				"TStaticArray<T,Size> value;\n"
				"copy(T v0, T v1){value[0]=v0;value[1]=v1;}\n"
				"copy(Vec2 l){value[0]=l[0];value[1]=l[1];}\n"
				"operator +(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l[0]+r[0],l[1]+r[1]);}\n"
				"operator -(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l[0]-r[0],l[1]-r[1]);}\n"
				"operator [](Vec2& l, int i):T {return l.value[i];}\n" //TODO return value компилируется (возвр 0) хотя должны быть ошибка
				"func Dot(Vec2 r):T {return value[0]*r[0]+value[1]*r[1];}\n"
				"\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	Vec2<int, 2> v(3,5),r(-2,8);\n"
				"	Vec2<int, 2> s=(v+r);\n"
				"	return s.Dot(v-r);\n"
				"}}");
			Assert::AreEqual((int)-34, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(IncrementDecrementPostfixOperatorsOverload)
		{
			CreateClass(
				"class TestClass {\n"
				"class Vec2<T,Size> {\n"
				"TStaticArray<T,Size> value;\n"
				"copy(T v0, T v1){value[0]=v0;value[1]=v1;}\n"
				"\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	Vec2<int, 2> v(3,5);\n"
				"	int a=5,b;\n"
				"	b=-a;\n"
				"	++a;\n"
				"	return a+b;\n"
				"}}");
			Assert::AreEqual(1, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(SimpleTemplateParameters)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class Vec2<T, Size> {\n"
				"TStaticArray<T,Size> value;\n"
				"\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	Vec2<int,2> v;\n"
				"	return 5;\n"
				"}}");
			Assert::AreEqual(5, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(ComplexTemplateParameters)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class Vec2<T, Size> {\n"
				"TStaticArray<T,Size> value;\n"
				"\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	Vec2<Vec2<int,3>,2> v;\n"
				"	return 5;\n"
				"}}");
			Assert::AreEqual(5, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(UnaryMinusOverload)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class Vec2<T,Size> {\n"
				"TStaticArray<T,Size> value;\n"
				"copy(T v0, T v1){value[0]=v0;value[1]=v1;}\n"
				"copy(Vec2 l){value[0]=l[0];value[1]=l[1];}\n"
				"operator [](Vec2& l, int i):T {return l.value[i];}\n"
				"operator - (Vec2 l):Vec2 {return (new Vec2)(-l[0],-l[1]);}"
				"conversion (Vec2 l):int {return l[0]+l[1];}\n"
				"\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	Vec2<int, 2> v(3,5);\n"
				"	return -v;\n"
				"}}");
			Assert::AreEqual((int)-8, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(LogicalOperatorsOverload)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class Vec2<T> {\n"
				"T value;\n"
				"copy(T v){value=v;}\n"
				"operator &&(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l.value&&r.value);}\n"
				"operator ||(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l.value||r.value);}\n"
				"operator !(Vec2 l):Vec2 {return (new Vec2)(!l.value);}\n"
				"conversion (Vec2 l):bool {return l.value;}\n"
				"\n"
				"}\n"
				"func static Test:bool\n"
				"{\n"
				"	Vec2<bool> r(false),v(true);\n"
				"	return r&&v;\n"
				"}\n"
				"func static Test1:bool\n"
				"{\n"
				"	Vec2<bool> r(false),v(true);\n"
				"	return r||v;\n"
				"}\n"
				"func static Test2:bool\n"
				"{\n"
				"	Vec2<bool> r(false);\n"
				"	return !r;\n"
				"}}");
			Assert::AreEqual((bool)false, *(bool*)RunClassMethod("func static Script.TestClass.Test:bool").get());
			Assert::AreEqual((bool)true, *(bool*)RunClassMethod("func static Script.TestClass.Test1:bool").get());
			Assert::AreEqual((bool)true, *(bool*)RunClassMethod("func static Script.TestClass.Test2:bool").get());
		}
		TEST_METHOD(CompareOperatorsOverload)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class Vec2<T> {\n"
				"T value;\n"
				"copy(T v){value=v;}\n"
				"operator ==(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l.value==r.value);}\n"
				"operator !=(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l.value!=r.value);}\n"
				"operator >(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l.value>r.value);}\n"
				"operator <(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l.value<r.value);}\n"
				"operator >=(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l.value>r.value);}\n"
				"operator <=(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l.value<r.value);}\n"
				"conversion (Vec2 l):int {return l.value;}\n"
				"\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	Vec2<int> r(3),v(4);\n"
				"	return r==v;\n"
				"}\n"
				"func static Test1:int\n"
				"{\n"
				"	Vec2<int> r(3),v(4);\n"
				"	return r!=v;\n"
				"}\n"
				"func static Test2:int\n"
				"{\n"
				"	Vec2<int> r(3),v(4);\n"
				"	return r>v;\n"
				"}\n"
				"func static Test3:int\n"
				"{\n"
				"	Vec2<int> r(3),v(4);\n"
				"	return r<v;\n"
				"}\n"
				"func static Test4:int\n"
				"{\n"
				"	Vec2<int> r(3),v(4);\n"
				"	return r>=v;\n"
				"}\n"
				"func static Test5:int\n"
				"{\n"
				"	Vec2<int> r(3),v(4);\n"
				"	return r<=v;\n"
				"}}");
			Assert::AreEqual((int)0, *(int*)RunClassMethod(nullptr, "Test").get());
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Test1").get());
			Assert::AreEqual((int)0, *(int*)RunClassMethod(nullptr, "Test2").get());
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Test3").get());
			Assert::AreEqual((int)0, *(int*)RunClassMethod(nullptr, "Test4").get());
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Test5").get());
		}
		TEST_METHOD(AssignOperatorsOverload)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class Vec2<T,Size> {\n"
				"TStaticArray<T,Size> value;\n"
				"copy(T v0, T v1){value[0]=v0;value[1]=v1;}\n"
				"copy(Vec2 l){value[0]=l[0];value[1]=l[1];}\n"
				"operator +(Vec2 l, Vec2 r):Vec2 {return (new Vec2)(l[0]+r[0],l[1]+r[1]);}\n"
				"operator +=(Vec2& l, Vec2 r):Vec2 {l = (new Vec2)(l[0]+r[0],l[1]+r[1]);}\n"
				//TODO для операторов присваивания - первый параметр должен быть ссылкой
				//"operator +=(Vec2 l, Vec2 r):Vec2 {l = (new Vec2)(l[0]+r[0],l[1]+r[1]);}\n"
				"operator -=(Vec2& l, Vec2 r):Vec2 {l = (new Vec2)(l[0]-r[0],l[1]-r[1]);}\n"
				"operator *=(Vec2& l, Vec2 r):Vec2 {l = (new Vec2)(l[0]*r[0],l[1]*r[1]);}\n"
				"operator /=(Vec2& l, Vec2 r):Vec2 {l = (new Vec2)(l[0]/r[0],l[1]/r[1]);}\n"
				"operator %=(Vec2& l, Vec2 r):Vec2 {l = (new Vec2)(l[0]%r[0],l[1]%r[1]);}\n"
				"operator &&=(Vec2& l, Vec2 r):Vec2 {l = (new Vec2)((new bool)(l[0])&&(new bool)(r[0]),(new bool)(l[1])&&(new bool)(r[1]));}\n"
				"operator ||=(Vec2& l, Vec2 r):Vec2 {l = (new Vec2)((new bool)(l[0])||(new bool)(r[0]),(new bool)(l[1])||(new bool)(r[1]));}\n"
				"operator [](Vec2& l, int i):T {return l.value[i];}\n"
				"\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	{"
				"	Vec2<int, 2> v(3,5),r(-2,8);\n"
				"	Vec2<int, 2> s=(v+r);\n"
				"	s%=v;\n"
				"	s&&=v;\n"
				"	s||=v;\n"
				"	}"
				"	Vec2<int, 2> v(3,5),r(-2,8);\n"
				"	Vec2<int, 2> s=(v+r);\n"
				"	s+=v;\n"
				"	s-=v;\n"
				"	s*=v;\n"
				"	s/=v;\n"
				"	return s[0]+s[1];\n"
				"}}");
			Assert::AreEqual((int)3-2+5+8, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(GetArrayElementOverload)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class Vec2<T,Size> {\n"
				"TStaticArray<T,Size> value;\n"
				"operator [](Vec2& l, int i):&T {return l.value[i];}\n"
				"operator [](Vec2& l, int i, int k):T {return l.value[i]+l.value[k];}\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	Vec2<int, 2> v;\n"
				"	v[0]=3;\n"
				"	v[1]=5;\n"
				"	return v[0]+v[1];\n"
				"}\n"
				"func static Test2:int\n"
				"{\n"
				"	Vec2<int, 2> v;\n"
				"	v[0]=3;\n"
				"	v[1]=5;\n"
				"	return v[0,1];\n"
				"}}");
			Assert::AreEqual((int)3+5, *(int*)RunClassMethod(nullptr, "Test").get());
			Assert::AreEqual((int)3 + 5, *(int*)RunClassMethod(nullptr, "Test2").get());
		}
		TEST_METHOD(AssignOperatorsOverloadSimple)
		{

			CreateClass(
				"class TestClass {\n"
				"func static Test:int\n"
				"{\n"
				"    var s = (new vec2)(1,1)[0];\n"
				"    return 1;\n"
				//"    return int(vec2(1,1)[0]);\n"
				"}}");
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(CallParamsOverload)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class Vec2<T,Size> {\n"
				"TStaticArray<T,Size> value;\n"
				"operator ()(Vec2& l, int i, int k):T {return l.value[0]+l.value[1]+i+k;}\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	Vec2<int, 2> v;\n"
				"	v.value[0]=3;\n"
				"	v.value[1]=5;\n"
				"	return v(5,8);\n"
				"}}");
			Assert::AreEqual((int)3+5+5+8, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(GetByReferenceOverload)
		{
			//TODO
		}
	};
	TEST_CLASS(TempatesTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(ValueParameters)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class TemplateClass<T,Size> {\n"
				"multifield(Size) T value;\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	TemplateClass<int, 5> v;\n"
				"	return 1;\n"
				"}}");
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(TemplateOfTemplate)
		{
			
			CreateClass(
				"class TestClass{\n"
				"class Test1<T>\n"
				"{\n"
				"	T f;\n"
				"}\n"
				"class Test2<T>\n"
				"{\n"
				"	T f;\n"
				"}\n"
				"func static Main:int\n"
				"{\n"
				"	Test1<Test2<int>> ddd;\n"
				"	Test1<Test2<Test1<int>>> ddd2;\n"
				"	Test1<Test1<Test1<int>>> ddd3;\n"
				"	Test2<Test2<Test1<int>>> ddd4;\n"
				"	return 1;\n"
				"}}\n"
				);
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Main").get());
		}
		TEST_METHOD(TemplateSubclass)
		{
			
			CreateClass(
				"class TestClass{\n"
				"class Test1<T>\n"
				"{\n"
				"	class SubItem\n"
				"	{\n"
				"		T f;\n"
				"		func Set(T val){f=val;}\n"
				"	}\n"
				"	T f;\n"
				"}\n"
				"func static Main:int\n"
				"{\n"
				"	Test1<Test1<int>>.SubItem sub_item_instance;\n"
				"	return 1;\n"
				"}}\n"
				);
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Main").get());
		}
		TEST_METHOD(TemplateOfTemplateWithMethodBody)
		{
			
			CreateClass(
				"class TestClass{\n"
				"class Test1<T>\n"
				"{\n"
				"	T f;\n"
				"	func SetValue(Test1<T> v)\n"
				"	{\n"
				"		Test1<int> tested_variable;\n"
				"	}\n"
				"	func SetValue2(Test2<T> v)\n"
				"	{\n"
				"		Test2<int> tested_variable;\n"
				"	}\n"
				"	func SetValue12(Test1<T> v1,Test2<T> v2)\n"
				"	{\n"
				"		Test1<int> tested_variable;\n"
				"		Test2<int> tested_variable2;\n"
				"	}\n"
				"}\n"
				"class Test2<T>\n"
				"{\n"
				"	T f;\n"
				"}\n"
				"func static Main:int\n"
				"{\n"
				"	Test1<int> ddd;\n"
				"	return 1;\n"
				"}}\n"
				);
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Main").get());
		}
		TEST_METHOD(TemplateOfTemplateCrossRef)
		{
			
			CreateClass(
				"class TestClass{\n"
				"class Test1<T>\n"
				"{\n"
				"	T f;\n"
				"	func SetValue(Test2<T> v, Test1<T> vvd)\n"
				"	{\n"
				"		Test2<Test1<Test2<float>>> df;\n"
				"		Test1<int> ddd;\n"
				"		TTmm<int, float> dsfsf;\n"
				"	}\n"
				"}\n"
				"class Test2<T>\n"
				"{\n"
				"	T f;\n"
				"	func SetValue(Test1<T> v)\n"
				"	{\n"
				"		Test2<Test1<Test2<float>>> df;\n"
				"		Test1<int> ddd;\n"
				"		TTmm<int, float> dsfsf;\n"
				"	}\n"
				"}\n"
				"class TTmm<T, K>\n"
				"{\n"
				"	T f1;\n"
				"	K f21;\n"
				"	func SetValue(T a, K b, TTmm<K, T> dsf)\n"
				"	{\n"
				"		Test2<Test1<Test2<float>>> df;\n"
				"	}\n"
				"}\n"
				"func static Main:int\n"
				"{\n"
				"	Test1<int> ddd;\n"
				"	TTmm<int, float> dsfsf;\n"
				"	return 1;\n"
				"}}\n"
				);
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Main").get());
		}
	};
	TEST_CLASS(StaticArrayTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
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
				"	for(int i=0;i<4;++i){for(int k=0;k<4;++k)s[i][k]=i+k;}"
				"	return s[2][1]+s[3][2];"
				"}"
				).get());
		}
		TEST_METHOD(ConstructorDestructorCount)
		{
			CreateClass(
				"class TestClass {\n"
				"int static constr_count;"
				"int static destr_count;"
				"class TemplateClass {\n"
				"	int v;\n"
				"	default {v=347;constr_count+=1;}\n"
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
				"}}");
			Assert::AreEqual((int)347 * 4, *(int*)RunClassMethod(nullptr, "Test").get());
			Assert::AreEqual((int) 4, *(int*)RunClassMethod(nullptr, "Test2").get());
			Assert::AreEqual((int)4, *(int*)RunClassMethod(nullptr, "Test3").get());
		}
		TEST_METHOD(ElementsDestructor)
		{
			CreateClass(
				"class TestClass {\n"
				"class TemplateClass {\n"
				"	TDynArray<int> v;\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	TStaticArray<TemplateClass,10> ss;\n"
				"	for(int i=0;i<10;i+=1){ss[i].v.resize(i+5);ss[i].v[2]=i;}\n"
				"	return ss[1].v[2]+ss[4].v[2];\n"
				"}}");
			Assert::AreEqual((int)1 + 4, *(int*)RunClassMethod(nullptr, "Test").get());
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
				"func static Test:int\n"
				"{\n"
				"	TStaticArray<TStaticArray<int,4>,4> s;\n"
				"	for(int i=0;i<4;i+=1){for(int k=0;k<4;k+=1)s[i][k]=i+k;}\n"
				"	TStaticArray<TStaticArray<int,4>,4> sc;\n"
				"	sc=s;\n"
				"	return sc[2][1]+sc[3][2];\n"
				"}\n"
				).get());
		}
		TEST_METHOD(CustomElementAssignTest0)
		{
			CreateClass(
				"class TestClass {\n"
				"class TemplateClass {\n"
				"	TDynArray<int> v;\n"
				"	operator static =(TemplateClass& l,TemplateClass& r)\n"
				"	{l.v=r.v;}"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	TStaticArray<TemplateClass,10> ss;\n"
				"	TStaticArray<TemplateClass,10> ss2;\n"
				"	ss2[1].v.resize(3);\n"
				"	ss=ss2;\n"
				"	return ss[1].v.size();\n"
				"}}");
			Assert::AreEqual((int)3, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(CustomElementAssignTest1)
		{
			CreateClass(
				"class TestClass {\n"
				"class TemplateClass {\n"
				"	TDynArray<int> v;\n"
				"	default{v.resize(10);for(int i=0;i<10;i+=1)v[i]=i;}"
				"	operator static =(TemplateClass& l,TemplateClass& r)\n"
				"	{l.v.resize(r.v.size());for(int i=0;i<l.v.size();i+=1)l.v[i]=r.v[i]+5;}\n"
				"	destr{v.resize(1);v[0]=223;}"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	TStaticArray<TemplateClass,10> ss;\n"
				"	TStaticArray<TemplateClass,10> ss2;\n"
				"	ss=ss2;\n"
				"	return ss[1].v[2]+ss[4].v[2];\n"
				"}}");
			Assert::AreEqual((int)2 + 5 + 2 + 5, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(GetElementTest)
		{
			Assert::AreEqual(7, *(int*)RunCode("func static Test:int{TStaticArray<int,3> s;s[0]=5;s[2]=2;return s[0]+s[2];}").get());
		}
	};
	TEST_CLASS(EnumerationTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(Initialization)
		{
			CreateClass(
				"class TestClass {\n"
				"enum TestEnum{One,Two,Three}"
				"func static Test:TestEnum\n"
				"{\n"
				"	TestEnum e;\n"
				"	e = (new TestEnum).Three;\n"
				"	return e;\n"
				"}}");
			Assert::AreEqual((int)2, *(int*)RunClassMethod("func static Script.TestClass.Test:TestEnum").get());
			//CreateClass(
			//	"class TestClass {\n"
			//	"enum TestEnum{One,Two,Three}"
			//	"func static Test:TestEnum\n"
			//	"{\n"
			//	"	TestEnum e(TestEnum.Two);\n"
			//	"	return e;\n"
			//	"}}");
			//Assert::AreEqual((int)1, *(int*)RunClassMethod("func static Script.TestClass.Test:TestEnum").get());
		}
	};
	TEST_CLASS(RefsRangeCheck)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(DynArrTest)
		{
			CreateClass(
				"class TestClass {\n"
				"func static SomFunc(TDynArray<int>& arr, int& arr_el)\n"
				"{\n"
				"	arr.resize(5);\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	TDynArray<int> source;\n"
				"	source.resize(2);\n"
				"	SomFunc(source, source[1]);\n"
				"	return 2;\n"
				"}}");
			bool raised = false;
			try
			{
				Assert::AreEqual((int)2, *(int*)RunClassMethod("func static Script.TestClass.Test:int").get());
			}
			catch (const RuntimeException& ex)
			{
				if(ex.id==RuntimeExceptionId::DynArray_UserAfterFree)
					raised = true;
			}
			Assert::IsTrue(raised);
		}
	};
	TEST_CLASS(ArraysSpecialSyntaxTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(StaticInitialization)
		{
			//TODO
			//Assert::AreEqual(7, *(int*)RunCode("func static Test:int{int[3] s;s[0]=5;s[2]=2;return s[0]+s[2];}").get());
		}
		TEST_METHOD(DynamicInitialization)
		{
			//TODO
			//Assert::AreEqual(7, *(int*)RunCode("func static Test:int{int[] s;s.resize(3);s[0]=5;s[2]=2;return s[0]+s[2];}").get());
		}
		TEST_METHOD(MixedInitialization)
		{
			//TODO
			//Assert::AreEqual(7, *(int*)RunCode("func static Test:int{int[][3] s;s.resize(3);s[0][2]=5;s[2][1]=2;return s[0][2]+s[2][1];}").get());
		}
		TEST_METHOD(MixedWithSubclassInitialization)
		{
			//TODO
			return;
			Assert::Fail();
				
				CreateClass(
					"class TestClass {\n"
					"int static constr_count;\n"
					"int static destr_count;\n"
					"class TemplateClass {\n"
					"	int v;\n"
					"	class Sub{int member;}\n"
					"	default {v=347;constr_count+=1;}\n"
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
					"}}");
				Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Test").get());
		}
	};
	TEST_CLASS(OperatorsTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(CallMethodFromMember)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class ITestBase { func GetValue:int {return 547;}}"
				"class ITest { func GetBase:ITestBase{return (new ITestBase)();}}"
				"func static Test:int\n"
				"{\n"
				"	ITest e;\n"
				"	int v = e.GetBase().GetValue();\n"
				"	return v;\n"
				"}}");
			Assert::AreEqual((int)547, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(CallStaticMethodFromMember)
		{
			//TODO утечка
			CreateClass(
				"class TestClass {\n"
				"class ITestBase { TDynArray<int> s;func GetValue:int {s.resize(1);return 547;}}"
				"class ITest { func static GetBase:ITestBase{return (new ITestBase)();}}"
				"func static Test:int\n"
				"{\n"
				"	ITest e;\n"
				"	int v = (new ITest).GetBase().GetValue();\n"
				"	return v;\n"
				"}}");
			Assert::AreEqual((int)547, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(CallMethodWithParamFromMember)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class ITestBase { func GetValue(string s):int {return 547;}}"
				"class ITest { func GetBase:ITestBase{return (new ITestBase)();}}"
				"func static Test:int\n"
				"{\n"
				"	ITest e;\n"
				"	int v = e.GetBase().GetValue(\"test\");\n"
				"	return v;\n"
				"}}");
			Assert::AreEqual((int)547, *(int*)RunClassMethod(nullptr, "Test").get());
		}
	};
	//TODO Вызов авто конструктора из пользовательского

	TEST_CLASS(StringTesting)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		TEST_METHOD(StringDefConstr)
		{
			Assert::AreEqual(0, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	string s;"
				"	return s.length();"
				"}"
				).get());
		}
		TEST_METHOD(StringLength)
		{
			Assert::AreEqual(0, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	string s;"
				"	return s.length();"
				"}"
				).get());
		}
		TEST_METHOD(StringAssign)
		{
			Assert::AreEqual(4, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	string s;"
				"	s=\"test\";"
				"	return s.length();"
				"}"
				).get());
		}
		TEST_METHOD(StringAssignOp)
		{
			Assert::AreEqual(7, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	string s=\"abc\";"
				"	s+=\"test\";"
				"	return s.length();"
				"}"
				).get());
		}
		TEST_METHOD(StringCopyConstr)
		{
			Assert::AreEqual(4, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	string s(\"test\");"
				"	return s.length();"
				"}"
				).get());
		}
		TEST_METHOD(StringGetChar)
		{
			Assert::AreEqual('e', *(char*)RunCode(
				"func static Test:char"
				"{"
				"	string s(\"test\");"
				"	return s[1];"
				"}"
				).get());
		}
		//TODO тип char
		//TEST_METHOD(StringGetCharValue)
		//{
		//	Assert::AreEqual('e', *(char*)RunCode(
		//		"func static Test:char"
		//		"{"
		//		"	return 'e';"
		//		"}"
		//		).get());
		//}
		TEST_METHOD(StringGetLengthFromTemp0)
		{
			//TODO тут имеется утечка String.h(24) - нужно убрать
			Assert::AreEqual(4, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	return (\"test\").length();"
				"}"
				).get());
		}
		//TODO добавить деструктор для временных объектов
		TEST_METHOD(StringGetLengthFromTemp)
		{
			Assert::AreEqual(16, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	return ((\"test\"+\"zopa\")+(\"test\"+\"zopa\")).length();"
				"}"
				).get());
		}

		TEST_METHOD(StringGetLengthFromTemp2)
		{
			Assert::AreEqual(16, *(int*)RunCode(
				"func static Test:int"
				"{"
				"	string temp = ((\"test\"+\"zopa\")+(\"test\"+\"zopa\"));"
				"	return temp.length();"
				"}"
				).get());
		}

		TEST_METHOD(StringGetLengthFromTemp3)
		{
			Assert::AreEqual(16, *(int*)RunCode(
				"func static Test:int\n"
				"{\n"
				"	string temp((\"zopa\"+\"zopa\")+(\"zopa\"+\"zopa\"));\n"
				"	return temp.length();\n"
				"}"
				).get());
		}

		TEST_METHOD(StringGetMembersFromRef)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class ITest1 { vec2 v; string abc; default { v.x = 1; v.y = 2; abc = \"zopa\";}}"
				"class ITest2  { ITest1 v; }"
				"func static ReturnResult:ITest2\n"
				"{ ITest2 v; return v;}\n"
				"func static Test:int\n"
				"{\n"
				"	vec2 test = ReturnResult().v.v;\n"
				"	string abc = ReturnResult().v.abc;\n"
				"	return abc.length();\n"
				"}}");
			Assert::AreEqual((int)4, *(int*)RunClassMethod(nullptr, "Test").get());
		}

		TEST_METHOD(StringGetMembersFromRef0)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class ITest1 {string abc; int v; }"
				"func static Test:int\n"
				"{\n"
				"	int test;\n"
				"	test = (new ITest1)().v;\n"
				"	string abc;\n"
				"	abc= (new ITest1)().abc;\n"
				"	return 4;\n"
				"}}");
			Assert::AreEqual((int)4, *(int*)RunClassMethod(nullptr, "Test").get());
		}

		TEST_METHOD(StringGetMembersFromRef1)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class ITest1 { vec2 v; string abc; default { v.x = 1; v.y = 2; abc = \"zopa\";}}"
				"class ITest2  { ITest1 v; }"
				"func static ReturnResult:ITest2\n"
				"{ ITest2 v; return v;}\n"
				"func static Test:int\n"
				"{\n"
				"	return (ReturnResult().v.abc+ReturnResult().v.abc).length();\n"
				"}}");
			Assert::AreEqual((int)8, *(int*)RunClassMethod(nullptr, "Test").get());
		}

		TEST_METHOD(StringGetMemberStringFromRef)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class ITest1 { string abc; }"
				"func static ReturnResult:ITest1\n"
				"{ return (new ITest1)();}\n"
				"func static Test:int\n"
				"{\n"
				"	string abc;\n"
				"	abc = ReturnResult().abc;\n"
				"	return abc.length();\n"
				"}}");
			Assert::AreEqual((int)0, *(int*)RunClassMethod(nullptr, "Test").get());
		}

		TEST_METHOD(StringGetMemberStringFromRef2)
		{
			
			CreateClass(
				"class TestClass {\n"
				"class ITest1 { string abc; }"
				"func static Test:int\n"
				"{\n"
				"	ITest1 abc;\n"
				"	abc = (new ITest1)();\n"
				"	return abc.abc.length();\n"
				"}}");
			Assert::AreEqual((int)0, *(int*)RunClassMethod(nullptr, "Test").get());
		}
	};
	TEST_CLASS(RefErrorsCheckTest)
	{
	public:
		TEST_METHOD_INITIALIZE(TestMethodInit)
		{
			BaseTypesTestsInitialize();
		}
		TEST_METHOD_CLEANUP(TestMethodCleanup)
		{
			BaseTypesTestsCleanup();
		}
		/*TEST_METHOD(BasicRefErrorsCheck)
		{
			
			CreateClass(
				"class TestClass {\n"
				"func static DoWork(TDynArray<TDynArray<int>>& arr, TDynArray<int>& el):int\n"
				"{\n"
				"	arr.resize(20);\n"
				"	el.resize(20);\n"
				"	return 1;\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	TDynArray<TDynArray<int>> arr;\n"
				"	arr.resize(1);\n"
				"	arr[0].resize(1);\n"
				"	int result=DoWork(arr, arr[0]);\n"
				"	return result;\n"
				"}}");
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Test").get());
		}
		TEST_METHOD(StackRefCheck)
		{
			
			CreateClass(
				"class TestClass {\n"
				"func static DoWork:&TDynArray<int>\n"
				"{\n"
				"	TDynArray<int> arr;\n"
				"	arr.resize(100);\n"
				"	return arr;\n"
				"}\n"
				"func static Test:int\n"
				"{\n"
				"	TDynArray<int> result=DoWork();\n"
				"	result.resize(1);\n"
				"	return 1;\n"
				"}}");
			Assert::AreEqual((int)1, *(int*)RunClassMethod(nullptr, "Test").get());
		}*/
	};
}