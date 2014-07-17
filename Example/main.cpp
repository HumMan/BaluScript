
#include "baluScript.h"
//#include <boost/filesystem.hpp>

//void ExMethodVecSum(int* &sp,int* first_param,int* object)// ���������: ������� �����, ��������� �� ������ ��������, ��������� �� ������)
//{
//	TVirtualMachine::TVec2* v1=(TVirtualMachine::TVec2*)first_param;
//	TVirtualMachine::TVec2* v2=(TVirtualMachine::TVec2*)(first_param+2);
//	*v1+=*v2;
//	sp-=2;
//}

//#include <tchar.h>

#include "../Source/Semantic/SMethod.h"
#include "../Source/Semantic/FormalParam.h"
#include "../Source/Syntax/Statements.h"

int main(int argc, char* argv[])
{
	setlocale(LC_ALL,"windows");
	//hFind=FindFirstFile("*.bscript", &FindFileData);
	//for(int i=1;i<10;i++)hFind == INVALID_HANDLE_VALUE
	//if(hFind != INVALID_HANDLE_VALUE)
	//do
	//setlocale ( LC_ALL, "Russian" );
	{
		//printf("Compiling '%s' ... \n",FindFileData.cFileName);
		printf("Compiling ... \n");

		char* source;
		{
			//TFileData file("C:\\developments\\BaluFramework\\BaluScript\\Output\\mingw\\script1.bscript"/*FindFileData.cFileName*/,"rb");
			TFileData file("../Data/base_types_test.bscript"/*FindFileData.cFileName*/,"rb");
			source=file.ReadAll();
			source[file.GetSize()]='\0';
		}
		int count=1000;
		int count_print=1000;
		int kk=1;
		int print_info=true;

		//while(true)
		{
			if(kk%count_print==0)
			{
				printf("%i done\n",count_print);
			}
			kk++;
			//count--;
			//if(count<0)break;
			TProgram program;
			TTime time;
			time.Start();
			unsigned long long t0=time.GetTime();
			//int main_func=-1;

			TSyntaxAnalyzer syntax;
			try
			{
				syntax.Compile(source,time);
				TSMethod* main_func = syntax.GetMethod("func static TGame.Main");
				int sp[200];
				std::vector<TStackValue> params;
				std::vector<TStaticValue> static_fields;
				std::vector<TStaticValue> static_variables;
				TStackValue result, object;
				main_func->Run(static_fields, params, result, object);
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
				if(print_info)
					printf("ok (%.3f ms)\n",time.TimeDiff(time.GetTime(),t0)*1000);
			}
			catch(std::string s)
			{
				printf(s.c_str());
			}
		}
		delete source;
#ifdef _MSC_VER
		//MBFreeContext();
#endif
	};//while (FindNextFile(hFind, &FindFileData) != 0);
	//FindClose(hFind);

	printf("All done");
	return 0;
}
