#include "../../../shared_api/os/predefines.h"
#include "../../../shared_api/rt/string_type.h"
#include "../../../shared_api/os/high_level.h"

#include <stdio.h>
#include <tchar.h>
#include <process.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <iostream>


bool SetUserEnv(LPCTSTR name, LPCTSTR value)
{
	HKEY key_env;
	if(ERROR_SUCCESS == ::RegOpenKey(HKEY_CURRENT_USER,_T("Environment"),&key_env))
	{
		if(ERROR_SUCCESS == ::RegSetValueEx(key_env,
												name,0,
												REG_EXPAND_SZ,
												(const BYTE*)value,
												(DWORD)sizeof(TCHAR)*(_tcslen(value)+1)))
		{
			DWORD dwReturnValue;
			SendMessageTimeout(HWND_BROADCAST,   WM_SETTINGCHANGE,   0,   
					(LPARAM)   "Environment",   SMTO_ABORTIFHUNG,   
					5000,   &dwReturnValue);

			return true;
		}
	}

	return false;
}



void SendSaveAll()   
{ 
	printf("Save all open documents\n\n");

	INPUT kb[10];
	ZeroMemory(kb,sizeof(kb));

	// Ctrl + S
	kb[0].type = INPUT_KEYBOARD;
	kb[0].ki.wVk = VK_CONTROL;
	kb[0].ki.dwFlags = 0;

	kb[1].type = INPUT_KEYBOARD;
	kb[1].ki.wVk = 'S';
	kb[1].ki.dwFlags = 0;

	kb[2].type = INPUT_KEYBOARD;
	kb[2].ki.wVk = 'S';
	kb[2].ki.dwFlags = KEYEVENTF_KEYUP;

	kb[3].type = INPUT_KEYBOARD;
	kb[3].ki.wVk = VK_CONTROL;
	kb[3].ki.dwFlags = KEYEVENTF_KEYUP;

	// Ctrl + Shift + S
	kb[4].type = INPUT_KEYBOARD;
	kb[4].ki.wVk = VK_CONTROL;
	kb[4].ki.dwFlags = 0;

	kb[5].type = INPUT_KEYBOARD;
	kb[5].ki.wVk = VK_SHIFT;
	kb[5].ki.dwFlags = 0;

	kb[6].type = INPUT_KEYBOARD;
	kb[6].ki.wVk = 'S';
	kb[6].ki.dwFlags = 0;

	kb[7].type = INPUT_KEYBOARD;
	kb[7].ki.wVk = 'S';
	kb[7].ki.dwFlags = KEYEVENTF_KEYUP;

	kb[8].type = INPUT_KEYBOARD;
	kb[8].ki.wVk = VK_SHIFT;
	kb[8].ki.dwFlags = KEYEVENTF_KEYUP;

	kb[9].type = INPUT_KEYBOARD;
	kb[9].ki.wVk = VK_CONTROL;
	kb[9].ki.dwFlags = KEYEVENTF_KEYUP;

	::SendInput(4,kb,sizeof(INPUT));

	::SetPriorityClass(GetCurrentProcess(),IDLE_PRIORITY_CLASS);
	::Sleep(100); 
}

rt::String		make_message;
rt::String		cur_directory;

void _output_line(rt::String_Ref& line)
{
	rt::String_Ref f[11];
	int co = line.Split(f,10,':');
	int first_num = 1;
	for(;first_num<co;first_num++)
		if(!f[first_num].IsEmpty() && isdigit(f[first_num][0]))
			break;

	rt::String out;
	if(first_num < co)
	{
		int l;
		f[first_num].ToNumber(l);

		if(first_num == co-1 || f[first_num+1].IsEmpty() || !isdigit(f[first_num+1][0]))
		{
			if(first_num == co-1)
			{	out =	rt::String_Ref(line.Begin(), f[first_num-1].End()) + 
					'(' + l + ')';
			}
			else
			{
				out =	rt::String_Ref(line.Begin(), f[first_num-1].End()) + 
						'(' + l + rt::SS("): ") + 
						rt::String_Ref(f[first_num+1].Begin(), line.End());
			}
		}
		else
		{
			int c;
			f[first_num+1].ToNumber(c);
			if(c>0)c--;

			if(first_num < co-2)
			{	out =	rt::String_Ref(line.Begin(), f[first_num-1].End()) + 
						'(' + l + ',' + c + rt::SS("): ") + 
						rt::String_Ref(f[first_num+2].Begin(), line.End());
			}
			else
			{	out =	rt::String_Ref(line.Begin(), f[first_num-1].End()) + 
						'(' + l + ',' + c + ')';
			}
		}
	}
	else
	{	out = line;
	}

	out.Replace("//","/");
	out.Replace(cur_directory,rt::String_Ref());
	if(out.StartsWith("In file included from "))
	{
		std::cout<<out.TrimLeft(22)<<" includes\n";
	}
	else
	{
		std::cout<<out.TrimSpace()<<'\n';
	}
	std::cout.flush();
	fflush(stdout);
}

void _output_cb(char* p, UINT len, LPVOID cookie)
{
	make_message += rt::String_Ref(p,len);

	rt::String_Ref line;
	while(make_message.GetNextLine(line))
	{
		if(line.End() == make_message.End())
		{
			make_message = line;
			return;
		}

		_output_line(line);
	}

	make_message.Empty();

	//std::cout<<rt::String(p,len);
}


int _tmain(int argc, _TCHAR* argv[])
{
	os::EnableCrashDump("make_ndk.dmp", true);
	
	rt::String  cmdline;

	if(argc>1)
	{	
		cmdline = "cmd.exe /c " + rt::String_Ref(argv[1]) + "\\ndk-build.cmd";
		if(argc>2)
		{
			for(int i=2;i<argc;i++)
				cmdline += ' ' + rt::String_Ref(argv[i]);
		}
	}
	else
	{	cmdline = "cmd.exe /c ndk-build.cmd";
	}

	std::cout<<"build with "<<cmdline<<'\n';

	{	char dir[1024];
		::GetCurrentDirectory(1024,dir);
		cur_directory = dir;
		cur_directory += '/';
		cur_directory.Replace('\\','/');
	}

	os::LaunchProcess	make;

	make.SetOutputCallback(_output_cb, NULL);
	if(make.Launch(cmdline, os::LaunchProcess::FLAG_SAVE_OUTPUT))
	{
		//std::cout<<make.GetOutput();
	}
	else
	{	std::cout<<("Failed to launch ndk-build.cmd.\n");
	}

	make.WaitForEnding();

	if(!make_message.IsEmpty())
	{
		_output_line(make_message);
	}
	//_STD_OUT<<"...\n";
	//system(cmdline);

	return make.GetExitCode();
}



