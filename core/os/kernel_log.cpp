#include "kernel.h"
#include "multi_thread.h"
#include "file_dir.h"

namespace os
{
namespace _details
{

#ifndef PLATFORM_DISABLE_LOG

static CriticalSection	_LogWriteCS;
static File				_LogFile;
static Timestamp		_LogTime;
static LogPrefix		_LogPrefix(os::LogPrefix() << '[' << os::_LOG_TIME << "] "); // default value here is moved to its ctor, GCC is not happy with that
static rt::String		_LogPrompt;

#endif
}

void SetLogConsolePrompt(LPCSTR prompt)
{
#ifndef PLATFORM_DISABLE_LOG
	_details::_LogPrompt = prompt;
#endif
}

bool SetLogFile(LPCSTR filename_in, bool append)
{
#ifndef PLATFORM_DISABLE_LOG
	rt::String filename = filename_in;

	os::Timestamp::Fields f = os::Timestamp::Get().GetLocalDateTime();

	filename.Replace("%YEAR%", rt::tos::Number(f.Year));
	filename.Replace("%MONTH%", rt::tos::Number(f.Month).RightAlign(2,'0'));
	filename.Replace("%DAY%", rt::tos::Number(f.Day).RightAlign(2,'0'));

	EnterCSBlock(_details::_LogWriteCS);
	if(_details::_LogFile.IsOpen())_details::_LogFile.Close();
	if(_details::_LogFile.Open(filename, append?os::File::Normal_AppendText:os::File::Normal_WriteText, true))
	{
		if(!append || _details::_LogFile.GetFileSize() == 0)
			if(3 != _details::_LogFile.Write("\xef\xbb\xbf", 3))
			{	_details::_LogFile.Close();
				return false;
			}

		return true;
	}
#endif

	return false;
}

rt::String_Ref GetLogFilename()
{
#ifndef PLATFORM_DISABLE_LOG
	return _details::_LogFile.GetFilename();
#else
	return NULL;
#endif
}

void SetLogPrefix(const LogPrefix& prefix)
{
#ifndef PLATFORM_DISABLE_LOG
    _details::_LogPrefix = prefix;
#endif
}

void LogWriteFlush()
{
#ifndef PLATFORM_DISABLE_LOG
	if(_details::_LogFile.IsOpen())
	{
		EnterCSBlock(_details::_LogWriteCS);
		_details::_LogFile.Flush();
		_details::_LogTime.LoadCurrentTime();
	}
#endif
}

LogPrefix& LogPrefix::operator << (LogPrefixItemCode code)
{
	_item& it = items.push_back();
    it.code = code;
    return *this;
}

LogPrefix& LogPrefix::operator << (char c)
{
	if (items.GetSize() && !items.last().code) // if last item is a string, append the string
    {   items.last().string += c;
    }
    else
    {   _item& it = items.push_back();
        it.string = rt::String(c, 1);
    }
    return *this;
}

LogPrefix& LogPrefix::operator << (LPCSTR s)
{
	if (items.GetSize() && !items.last().code) // if last item is a string, append the string
    {   items.last().string += s;
    }
    else
    {   _item& it = items.push_back();
        it.string = s;
    }
    return *this;
}

const LogPrefix& LogPrefix::operator = (const LogPrefix& x)
{   items.SetSize(x.items.GetSize());
    for (int i = 0; i < (int)x.items.GetSize(); i++)
    {   
        if (x.items[i].code)
        {   items[i].code = x.items[i].code;
        }
        else
        {   items[i].string = x.items[i].string;
        }
    }
    return *this;
}

} // namespace os



#if defined(PLATFORM_WIN)
//////////////////////////////////////////////////////////
// All Windows implementations
#include <windows.h>
#include <Rpc.h>
#include <iostream>
#include <io.h>
#pragma comment(lib,"Rpcrt4.lib")


namespace os
{
namespace _details
{
	void __ConsoleLogWriteDefault(LPCSTR log, int type, LPVOID)
	{	
		static bool ConsoleCreated = false;

		if(!ConsoleCreated)
		{
			if(::GetConsoleWindow()){ ConsoleCreated = true; }
			else
			{
#pragma warning(disable:4311)
				// allocate a console for this app
				AllocConsole();
				// set the screen buffer to be big enough to let us scroll text
                CONSOLE_SCREEN_BUFFER_INFO coninfo;
                GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &coninfo);
				coninfo.dwSize.Y = 9999;
				// How many lines do you want to have in the console buffer
				SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), coninfo.dwSize);
                // redirect unbuffered STDOUT to the console
                freopen("CONOUT$", "w", stdout);
				setvbuf(stdout, NULL, _IONBF, 0);
				// redirect unbuffered STDIN to the console
                freopen("CONIN$", "r", stdin);
				setvbuf(stdin, NULL, _IONBF, 0);
				// redirect unbuffered STDERR to the console
                freopen("CONOUT$", "w", stderr);
                setvbuf(stderr, NULL, _IONBF, 0);

				// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog point to console as well
				std::ios::sync_with_stdio();

				HWND hConsoleWnd = ::GetConsoleWindow();
				ASSERT(hConsoleWnd);
				ConsoleCreated = true;
#pragma warning(default:4311)
			}
		}

		int color[] = { 8, 8, 7, 10, 14, 12 };
		SetConsoleTextAttribute(GetStdHandle( STD_OUTPUT_HANDLE ), color[type&rt::LOGTYPE_LEVEL_MASK]);
		int len = (int)strlen(log);
		os::LPU16CHAR utf16 = (os::LPU16CHAR)alloca(2*len);
		int len_utf16 = (int)os::UTF8Decode(log, len, utf16);
		char* mb = (char*)alloca(3*len_utf16 + 1);
		mb[WideCharToMultiByte(CP_THREAD_ACP, 0, utf16, len_utf16, mb, 3*len_utf16, NULL, NULL)] = 0;

		thread_local bool _last_updating = false;

		if(_last_updating)
			fputs("\r                                                                               \r", stdout);

		if((type&rt::LOGTYPE_LEVEL_MASK) != rt::LOGTYPE_UPDATING)
		{
			if(!_details::_LogPrompt.IsEmpty())putchar('\r');

			if((type&rt::LOGTYPE_IN_CONSOLE_PROMPT) == 0)
				puts(mb);

			SetConsoleTextAttribute(GetStdHandle( STD_OUTPUT_HANDLE ), color[2]);

			if(!_details::_LogPrompt.IsEmpty())
				fputs(_details::_LogPrompt, stdout);

			_last_updating = false;
		}
		else
		{
			fputs(mb, stdout);
			_last_updating = true;
			fflush(stdout);
			SetConsoleTextAttribute(GetStdHandle( STD_OUTPUT_HANDLE ), color[2]);
		}
	}
}

void SetLogConsoleTitle(LPCSTR title)
{
	HWND wnd = ::GetConsoleWindow();
	if(wnd)
		::SetWindowTextW(wnd, os::__UTF16(title));
}

} //  namespace os::_details

#elif defined(PLATFORM_ANDROID)
#include <android/log.h>

namespace os
{
namespace _details
{
	void __ConsoleLogWriteDefault(LPCSTR log, int type, LPVOID)
	{	
		ASSERT(type >=0 && type<rt::LOGTYPE_MAX);
		int cat[] = { ANDROID_LOG_VERBOSE, ANDROID_LOG_DEBUG, ANDROID_LOG_INFO, ANDROID_LOG_WARN, ANDROID_LOG_ERROR };
		__android_log_write(cat[type], "CPF", log);
	}
}} //  namespace os::_details

#elif defined (PLATFORM_IOS)

namespace os
{
namespace _details
{
	void __ConsoleLogWriteDefault(LPCSTR log, int type, LPVOID)
	{	
		puts(log);
	}
}} //  namespace os::_details

#elif defined(PLATFORM_LINUX) || defined (PLATFORM_MAC)
#include <sys/ioctl.h>
#include <stdio.h>

namespace os
{
namespace _details
{
	void __ConsoleLogWriteTerminal(LPCSTR mb, int type, LPVOID)
	{	
		setlocale(LC_ALL, "en_US.UTF-8");
		
		LPCSTR color[] = { "\033[0;37m", "\033[0;37m", "\033[0m", "\033[1;32m", "\033[1;33m", "\033[1;31m" };
        
		thread_local bool _last_is_updating = false;
		
		if(_last_is_updating)
			fputs("\033[2K\r", stdout);
		
		fputs(color[type&rt::LOGTYPE_LEVEL_MASK], stdout);
		if((type&rt::LOGTYPE_LEVEL_MASK) != rt::LOGTYPE_UPDATING)
		{
			if(!_details::_LogPrompt.IsEmpty())putchar('\r');
			
			if((type&rt::LOGTYPE_IN_CONSOLE_PROMPT) == 0)
				fputs(mb, stdout);
			
			puts("\033[0m");
			
			if(!_details::_LogPrompt.IsEmpty())
				fputs(_details::_LogPrompt, stdout);
				
			fflush(stdout);
			_last_is_updating = false;
		}
		else
		{	
			fputs(mb, stdout);
			_last_is_updating = true;
			fputs("\033[0m", stdout);
			fflush(stdout);
		}
	}
    
    void __ConsoleLogWriteDebugger(LPCSTR log, int type, LPVOID)
    {
        puts(log);
    }
    
    void __ConsoleLogWriteDefault(LPCSTR mb, int type, LPVOID)
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        if(w.ws_row && w.ws_col)  // detect terminal/debugger
        {
            SetConsoleLogWriteFunction(__ConsoleLogWriteTerminal, NULL);
            __ConsoleLogWriteTerminal(mb, type, NULL);
        }
        else
        {
            SetConsoleLogWriteFunction(__ConsoleLogWriteDebugger, NULL);
            __ConsoleLogWriteDebugger(mb, type, NULL);
        }
    }
}} //  namespace os::_details

#else
#endif


namespace os
{
namespace _details
{

/////////////////////////////////////////////////////////////////
// Platform independent implementations but depend on some platform dependent functions
void LogWriteDefault(LPCSTR log, LPCSTR file, int line_num, LPCSTR func, int type, LPVOID)
{
#ifndef PLATFORM_DISABLE_LOG
	EnterCSBlock(_LogWriteCS);

	if(type&(rt::LOGTYPE_IN_CONSOLE|rt::LOGTYPE_IN_CONSOLE_FORCE|rt::LOGTYPE_IN_CONSOLE_PROMPT))os::_details::__ConsoleLogWrite(log, type);
	if((type&rt::LOGTYPE_IN_LOGFILE) && _LogFile.IsOpen())
	{
        for (int i = 0; i < (int)_LogPrefix.items.GetSize(); i++)
        {
            const LogPrefix::_item& item = _LogPrefix.items[i];
            if (item.code)
            {   switch (item.code)
                {
                case _LOG_TIME:
                    _LogTime.LoadCurrentTime();
                    _LogFile.Write(rt::tos::TimestampFields<false,true>(_LogTime.GetLocalDateTime()));
                    break;
                case _LOG_FILE:
                    _LogFile.Write(file);
                    break;
                case _LOG_LINE:
                    _LogFile.Write(rt::tos::Number(line_num));
                    break;
                case _LOG_FUNC:
                    _LogFile.Write(func);
                    break;
                case _LOG_MEM:
					ULONGLONG free;
					os::GetSystemMemoryInfo(&free);
					_LogFile.Write("MEM(");
                    _LogFile.Write(rt::tos::Number(free / 1024 / 1024));
                    _LogFile.Write("MB)");
                    break;
                case _LOG_CPU:
					{   thread_local ULONGLONG last_p[2] = {0, 0};
						ULONGLONG p[2] = {0, 0};
                        GetProcessorTimes(p);

						_LogFile.Write("CPU(");
						_LogFile.Write(rt::tos::Number(((p[0] - last_p[0])*100/(float)(p[1] - last_p[1]))));
						_LogFile.Write("%)");

						if(last_p[1] + 2000 < p[1])
						{	ULONGLONG shift = p[1] - last_p[1] - 2000;
							last_p[1] += shift;
							last_p[0] = rt::min(p[0], last_p[0] + shift*last_p[0]/last_p[1]);
						}
                    }
                    break;
                }
            }
            else
            {   _LogFile.Write(item.string);
            }
        }
		_LogFile.Write(rt::String(log));
		_LogFile.Write('\n');
		_LogFile.Flush();
	}

#endif
}


static FUNC_LOG_WRITE	__LogWrtieFunc = LogWriteDefault;
static LPVOID			__LogWrtieFuncCookie = NULL;
static bool				__LogWrtieNoConsoleDisplay = false;

void SetLogWriteFunction(FUNC_LOG_WRITE func, LPVOID cookie)
{
	if(func)
	{	__LogWrtieFunc = func;
		__LogWrtieFuncCookie = cookie;
	}
	else
	{	__LogWrtieFunc = LogWriteDefault;
		__LogWrtieFuncCookie = NULL;
	}
}

static LPVOID					__ConsoleLogWriteCookie = NULL;
static FUNC_CONSOLE_LOG_WRITE	__ConsoleLogWriteFunc = __ConsoleLogWriteDefault;

void __ConsoleLogWrite(LPCSTR log, int type){ __ConsoleLogWriteFunc(log, type, __ConsoleLogWriteCookie); }

void SetConsoleLogWriteFunction(FUNC_CONSOLE_LOG_WRITE func, LPVOID cookie)
{
	if(func)
	{	__ConsoleLogWriteFunc = func;
		__ConsoleLogWriteCookie = cookie;
	}
	else
	{	__ConsoleLogWriteCookie = NULL;
		__ConsoleLogWriteFunc = __ConsoleLogWriteDefault;
	}
}

} // namespace _details

void LogDisplayInConsole(bool yes)
{
	_details::__LogWrtieNoConsoleDisplay = !yes;
}

bool LogIsDisplayInConsole()
{
	return !_details::__LogWrtieNoConsoleDisplay;
}

void LogWrite(LPCSTR log, LPCSTR file, int line_num, LPCSTR func, int type)
{	
	if(_details::__LogWrtieNoConsoleDisplay)type = type&(~rt::LOGTYPE_IN_CONSOLE);
	_details::__LogWrtieFunc(log, file, line_num, func, type, _details::__LogWrtieFuncCookie);
}


} // namespace os
