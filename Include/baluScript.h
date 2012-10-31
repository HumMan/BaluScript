#pragma once

#include <string>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>

#ifndef _MSC_VER
#define __forceinline __inline
#define _INTSIZEOF(x) ((((sizeof(x)-1))/4+1)*4)
#endif

#ifdef COMPILE_TIME_ERR
#error "COMPILE_TIME_ERR already used"
#endif
#ifndef _DEBUG
#define COMPILE_TIME_ERR(x) ((void)0)
#else
#define COMPILE_TIME_ERR(x) {char c[x?1:-1];}
#endif

//#include "..\Source\mballoc.h" 
//TODO ���� � baluLib ����� ����� � ���� ���������� � ��������� ����� ����������� � ����� �� �����������

#include "../Source/stdfuncs.h"
#include "../Source/vec.h"
#include "../Source/common.h"
#include "../Source/arrays.h"

#include "../Source/virtualMachine.h"
#include "../Source/lexer.h"
#include "../Source/syntaxAnalyzer.h"

#define BSCRIPT_VOID_METHOD(class_name,name) \
	void static name(int*& stack,int* first_param,int* _obj)\
	{\
		class_name* obj=(class_name*)_obj;\
		obj->name();\
	}\
	void name()

#define BSCRIPT_VOID_METHOD1(class_name,name,type1,arg1) \
	void static name(int*& stack,int* first_param,int* _obj)\
	{\
		class_name* obj=(class_name*)_obj;\
		type1* p1=(type1*)first_param;\
		obj->name(*p1);\
	}\
	void name(type1& arg1)

#define BSCRIPT_VOID_METHOD2(class_name,name,type1,arg1,type2,arg2) \
	void static name(int*& stack,int* first_param,int* _obj)\
	{\
		class_name* obj=(class_name*)_obj;\
		type1* p1=(type1*)first_param;\
		type2* p2=(type2*)((int*)p1+_INTSIZEOF(type1)/4);\
		obj->name(*p1,*p2);\
	}\
	void name(type1& arg1,type2& arg2)

#define BSCRIPT_VOID_METHOD3(class_name,name,type1,arg1,type2,arg2,type3,arg3) \
	void static name(int*& stack,int* first_param,int* _obj)\
	{\
		class_name* obj=(class_name*)_obj;\
		type1* p1=(type1*)first_param;\
		type2* p2=(type2*)((int*)p1+_INTSIZEOF(type1)/4);\
		type3* p3=(type3*)((int*)p2+_INTSIZEOF(type2)/4);\
		obj->name(*p1,*p2,*p3);\
	}\
	void name(type1& arg1,type2& arg2,type3& arg3)

#define BSCRIPT_METHOD1(ret_type,class_name,name,type1,arg1) \
	void static name(int*& stack,int* first_param,int* _obj)\
	{\
		class_name* obj=(class_name*)_obj;\
		type1* p1=(type1*)first_param;\
		int p1_size=_INTSIZEOF(type1)/4;\
		int ret_size=_INTSIZEOF(ret_type)/4;\
		stack+=ret_size;\
		ret_type* r=(ret_type*)(first_param+p1_size);\
		*r=obj->name(*p1);\
	}\
	ret_type name(type1& arg1)
