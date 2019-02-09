
#include "baluScript.h"

#include <fstream>
#include <streambuf>

#include "../Source/SemanticInterface/SemanticTreeApi.h"
#include "../Source/TreeRunner/TreeRunner.h"

#include <windows.h>
#include <delayimp.h>

#pragma comment(lib, "delayimp")  

int main(int argc, char* argv[])
{
	
	{
		setlocale(LC_ALL, "windows");

		printf("Compiling ... \n");
		//TTime time;
		//time.Start();
		printf("Compiling ... \n");

		std::string source;
		{
			std::ifstream file("../Data/base_types_test.bscript");
			std::string str((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());
			source = str;
		}
		{
			
			//time.Start();
			//unsigned long long t0=time.GetTime();

			ISyntaxAnalyzer* syntax = ISyntaxAnalyzer::Create();
			try
			{
				syntax->Compile(source.c_str());

				SemanticApi::ISMethod* main_func = syntax->GetMethod("func static Script.Main");
				//int sp[200];
				std::vector<TStackValue> params;
				std::vector<TStaticValue> static_fields;
				std::vector<TStaticValue> static_variables;
				TStackValue result, object;
				auto refs_list = new TRefsList();
				TGlobalRunContext global_context(&static_fields, refs_list);
				TMethodRunContext method_run_context(global_context, &params, &result, &object);
				
				TreeRunner::Run(main_func, method_run_context);
				delete refs_list;

		/*		if(print_info)
					printf("ok (%.3f ms)\n",time.TimeDiff(time.GetTime(),t0)*1000);
				t0=time.GetTime();
				main_func=syntax.GetMethod("func static TGame.Main");
				if(print_info)
					printf("Linking extenral methods (%.3f ms)\n",time.TimeDiff(time.GetTime(),t0)*1000);
				t0=time.GetTime();
				if(print_info)
					printf("Generating program ...\n");
				syntax.GetProgram(program,time);
				if(print_info)
					printf("ok (%i ops, op_size = %i, %.3f ms)\n",
					program.instructions.size(),
					sizeof(TOp),
					time.TimeDiff(time.GetTime(),t0)*1000);
				t0=time.GetTime();


				TVirtualMachine machine(2*10e6);
				machine.SetProgram(&program);
				if(print_info)printf("Executing ... \n");
				for(int k=0;k<1;k++)
				{
					machine.ConstructStaticVars();
					machine.Execute(main_func);
					machine.DestructStaticVars();
				}*/
				//printf("ok (%.3f ms)\n",time.TimeDiff(time.GetTime(),t0)*1000);
			}
			catch(std::string s)
			{
				printf(s.c_str());
			}
			ISyntaxAnalyzer::Destroy(syntax);
		}
	}

	printf("All done");
	auto unload_result = __FUnloadDelayLoadedDLL2("BaluScript.dll");
	_CrtDumpMemoryLeaks();
	return 0;
}
