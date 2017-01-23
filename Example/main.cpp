
#include "baluScript.h"

#include "../Source/Semantic/SMethod.h"
#include "../Source/Semantic/FormalParam.h"
#include "../Source/Semantic/SStatements.h"
#include "../Source/Syntax/Statements.h"
#include "../Source/Syntax/Method.h"

#include <fstream>
#include <streambuf>

#include "../Source/NativeTypes/base_types.h"

int main(int argc, char* argv[])
{
	
	{
		setlocale(LC_ALL, "windows");

		printf("Compiling ... \n");
		//TTime time;
		//time.Start();
		printf("Compiling ... \n");

		std::string source = base_types;
		std::string source2;
		{
			std::ifstream file("../Data/base_types_test.bscript");
			std::string str((std::istreambuf_iterator<char>(file)),
				std::istreambuf_iterator<char>());
			source2 = str;
		}
		{
			
			//time.Start();
			//unsigned long long t0=time.GetTime();

			TSyntaxAnalyzer syntax;
			try
			{
				syntax.Compile((char*)(("class Script{" + source + source2 + "}").c_str()));

				TSMethod* main_func = syntax.GetMethod("func static Script.Main");
				//int sp[200];
				std::vector<TStackValue> params;
				std::vector<TStaticValue> static_fields;
				std::vector<TStaticValue> static_variables;
				TStackValue result, object;
				TMethodRunContext method_run_context(&static_fields, &params, &result, &object);
				main_func->Run(method_run_context);

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
		}
	}

	printf("All done");
	_CrtDumpMemoryLeaks();
	return 0;
}
