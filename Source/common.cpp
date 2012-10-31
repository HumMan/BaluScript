#include "baluScript.h"

#include "string.h"

#if defined(WIN32)||defined(_WIN32)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <sys/time.h>
#endif

#define BALU_LIB_KEYS_BEGIN( ENUM_NAME ) \
struct TKeyInfo{char* name;int internal_key;};\
	const TKeyInfo KeyInfo[]= 

#if defined(WIN32)
#define BALU_LIB_KEY( name,internal_key_win,internal_key_linux) {#name,internal_key_win}
#else
#define BALU_LIB_KEY( name,internal_key_win,internal_key_linux) {#name,internal_key_linux}
#endif

#define BALU_LIB_KEYS_END() ;

namespace TBaluVirtKey{
#include "keys.h"
}

#undef BALU_LIB_KEYS_BEGIN
#undef BALU_LIB_KEY
#undef BALU_LIB_KEYS_END

bool KeyDown(int key_char)
{
#if defined(WIN32)||defined(_WIN32)
	short key;
	key=GetKeyState(key_char);
	return (key&(1<<15))!=0;
#else
	Display* display;
	display=XOpenDisplay(0);
	Window window=DefaultRootWindow(display);
	int result=XGrabKey(display,key_char,0,window,1,1,1);
	return result;
#endif
}

bool KeyDown(char key_char)
{
	assert((key_char>='A'&&key_char<='Z')||(key_char>='0'&&key_char<='9'));
	return KeyDown(int(key_char));
}

bool KeyDown(TBaluVirtKey::Enum key_char)
{
	return KeyDown(int(TBaluVirtKey::KeyInfo[key_char].internal_key));
}

TVec2i GetCursorPos()
{
#if defined(WIN32)||defined(_WIN32)
	POINT p;
	GetCursorPos(&p);
	return TVec2i(p.x,p.y);
#else
#endif
}

void SetCursorPos(TVec2i pos)
{
#if defined(WIN32)||defined(_WIN32)
	SetCursorPos(pos[0],pos[1]);
#else
#endif
}

//
//class TTime
//{
//	double dT,curr_fps;
//	int frames;
//	double prev_time,freq,curr_time;
//	double fps;
//public:
//	void Start(){
//		prev_time=GetTickCount()*0.001;
//		curr_fps=0;
//		fps=0;
//		frames=0;
//	}
//	void Tick(){
//		curr_time=GetTickCount()*0.001;
//		dT=curr_time-prev_time;
//		prev_time=curr_time;
//		frames++;
//		curr_fps+=dT;
//		if(curr_fps>0.5f){//��������� FPS �� 0.5 �
//			fps=frames/curr_fps;
//			curr_fps=0;
//			frames=0;
//		}
//	}
//	bool ShowFPS(){
//		return !frames;
//	}
//	double GetTick(){
//		return dT;
//	}
//	double GetFPS(){
//		return fps;
//	}
//	double GetCurrTime(){
//		return curr_time;
//	}
//};


unsigned long long TTime::GetTime()
{
#ifdef WIN32
	LARGE_INTEGER s;
	QueryPerformanceCounter(&s);
	return s.QuadPart;
#else
	timeval tv;
	gettimeofday(&tv,NULL);
	return (int)(tv.tv_sec*1000 + (tv.tv_usec / 1000));
#endif
}
double TTime::TimeDiff(unsigned long long curTime,unsigned long long prevTime)
{
#ifdef WIN32
	return double(((double)(signed __int64)(curTime-prevTime))/((double)(signed __int64)freq));
#else
	return (curTime-prevTime)*0.001;
#endif
}
void TTime::Start(){
#ifdef WIN32
	LARGE_INTEGER s;
	QueryPerformanceFrequency(&s);
	freq=s.QuadPart;
#endif
	prev_time=GetTime();
	curr_fps=0;
	fps=0;
	frames=0;
}
double TTime::GetDelta()
{
	return TimeDiff(GetTime(),prev_time);
}
void TTime::Tick(){
	curr_time=GetTime();
	dT=TimeDiff(curr_time,prev_time);
	prev_time=curr_time;
	frames++;
	curr_fps+=dT;
	if(curr_fps>0.5f){
		fps=frames/curr_fps;
		curr_fps=0;
		frames=0;
	}
}
bool TTime::ShowFPS(){
	return !frames;
}
double TTime::GetTick(){
	return dT;
}
double TTime::GetFPS(){
	return fps;
}

TFileData::TFileData(const char* fname,const char* mode)
{
	file=NULL;
#ifdef _MSC_VER
	if( fopen_s( (FILE**)&file, fname, mode) !=0 )
		throw "File was not opened!";
#else
	file=fopen(fname, "r" );
	if(file==NULL)
		throw "File was not opened!";
#endif
	fsize=0;
	if(mode[0]!='w')
	{
		while(!feof((FILE*)file))
		{
			char buf[256];
			fsize+=fread(buf,1,256,(FILE*)file);
		}
		rewind((FILE*)file);
	}
};
TFileData::~TFileData()
{
	if(file!=NULL)
		fclose((FILE*)file);
};
void TFileData::Close()
{
	fclose((FILE*)file);
	file=NULL;
};
char* TFileData::ReadAll()
{
	assert(file!=NULL);
	rewind((FILE*)file);
	char* data=new char[fsize+1];
	fread(data,fsize,1,(FILE*)file);
	rewind((FILE*)file);
	return data;
};
int TFileData::GetSize()
{
	return fsize;
};
void TFileData::Read(void* v,int size)const
{
	assert(file!=NULL);
	fread(v,size,1,(FILE*)file);
};
void TFileData::Read(void* v,int el_size,int count)const
{
	assert(file!=NULL);//TODO контроль прочитанного количества
	fread(v,el_size,count,(FILE*)file);
};
void TFileData::Write(const void* v,int size)
{
	assert(file!=NULL);
	fwrite(v,size,1,(FILE*)file);
};
void TFileData::Write(const void* v,int el_size,int count)
{
	assert(file!=NULL);
	fwrite(v,el_size,count,(FILE*)file);
};
void TFileData::Read(int& v)const
{
	assert(file!=NULL);
	fread(&v,sizeof(int),1,(FILE*)file);
};
void TFileData::Read(bool& v)const
{
	assert(file!=NULL);
	fread(&v,sizeof(bool),1,(FILE*)file);
};
void TFileData::Write(const int v)const
{
	assert(file!=NULL);
	fwrite(&v,sizeof(int),1,(FILE*)file);
};
void TFileData::Write(const bool v)const
{
	assert(file!=NULL);
	fwrite(&v,sizeof(bool),1,(FILE*)file);
};

void TFileData::Write(const char* v)const
{
	assert(file!=NULL);
	fwrite(v,sizeof(char),strlen(v),(FILE*)file);
	fflush((FILE*)file);
};
