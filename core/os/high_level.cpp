#include "high_level.h"

namespace os
{


os::Daemon* os::Daemon::_pDaemon = NULL;

os::Daemon::Daemon()
{
	ASSERT(_pDaemon == NULL);
	_pDaemon = this;
#if defined(PLATFORM_WIN)
	_hServiceHandler = NULL;
#else
#endif
}

os::Daemon::~Daemon()
{
	ASSERT(_pDaemon);
	_pDaemon = NULL;
}

#if defined(PLATFORM_WIN)

bool os::Daemon::InitializeDaemonController(LPCSTR svc_name)
{
	ASSERT(_hServiceHandler == 0);
	_DaemonName = svc_name;
	
	struct _Func
	{	
		static VOID WINAPI OnServiceManagerControl(DWORD fdwControl)
		{
#if defined(PLATFORM_WIN)
			switch(fdwControl)
			{
				case SERVICE_CONTROL_CONTINUE: _pDaemon->OnDaemonControl(DAEMON_CONTROL_CONTINUE); break;
				case SERVICE_CONTROL_PAUSE: _pDaemon->OnDaemonControl(DAEMON_CONTROL_PAUSE); break;
				case SERVICE_CONTROL_SHUTDOWN: 
				case SERVICE_CONTROL_STOP:	_pDaemon->OnDaemonControl(DAEMON_CONTROL_STOP); break;
			}
#endif
		}
		static VOID WINAPI SvcMain(DWORD dwArgc,LPSTR* lpszArgv)
		{
			_pDaemon->_hServiceHandler = ::RegisterServiceCtrlHandlerA(_pDaemon->_DaemonName,OnServiceManagerControl);
		}
		static DWORD Dispatcher(LPVOID)
		{
			SERVICE_TABLE_ENTRYA svc_tab[] = 
			{	{_pDaemon->_DaemonName,_Func::SvcMain},
				{NULL,NULL}
			};
			
			::StartServiceCtrlDispatcherA(svc_tab);
			return 0;
		}
	};

	// start dispatch thread
	_DispatchThread.Create(_Func::Dispatcher,0);
	Sleep(10);
	while(_hServiceHandler == NULL)
	{	
		if(_DispatchThread.WaitForEnding(200))
		{	//ASSERT(0); // StartServiceCtrlDispatcher failed
			return false;
		}
	}
	Sleep(0);

	ReportDaemonStatus(SERVICE_START_PENDING);
	return _hServiceHandler != 0;
}

void os::Daemon::ReportDaemonStatus(DWORD state)
{
	if(_hServiceHandler)
	{	
		SERVICE_STATUS SvcStatus;

		static DWORD dwCheckPoint = 1;

		SvcStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS|SERVICE_INTERACTIVE_PROCESS;
		SvcStatus.dwCurrentState = state;
		SvcStatus.dwWin32ExitCode = NO_ERROR;
		SvcStatus.dwServiceSpecificExitCode = NO_ERROR;
		SvcStatus.dwWaitHint = 0;

		switch(state)
		{
		case SERVICE_START_PENDING:
		case SERVICE_CONTINUE_PENDING:
		case SERVICE_PAUSE_PENDING:
		case SERVICE_STOPPED:
			SvcStatus.dwControlsAccepted = 0;
			break;
		case SERVICE_PAUSED:
		case SERVICE_RUNNING:
			SvcStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;//|SERVICE_ACCEPT_PAUSE_CONTINUE;
			break;
		default:
			SvcStatus.dwControlsAccepted = 0;
		}

		if ( (state == SERVICE_RUNNING) || (state == SERVICE_STOPPED) )
			SvcStatus.dwCheckPoint = 0;
		else
			SvcStatus.dwCheckPoint = dwCheckPoint++;

		VERIFY(::SetServiceStatus((SERVICE_STATUS_HANDLE)_hServiceHandler,&SvcStatus));
	}
}




////////////////////////////////////////////////////////////
// CLaunchProcess
os::LaunchProcess::LaunchProcess()
{
	_hProcess = INVALID_HANDLE_VALUE;
	hChildStdoutRdDup = INVALID_HANDLE_VALUE;
	hChildStdinRd = INVALID_HANDLE_VALUE;
	hChildStdinWrDup = INVALID_HANDLE_VALUE;
	hChildStdoutWr = INVALID_HANDLE_VALUE;

	_Callback = NULL;

	_ExitCode = 0;
	_ExitTime = 0;
	_ExecutionTime = 0;
}

os::LaunchProcess::~LaunchProcess()
{
	IsRunning();
	ClearAll();
}


void os::LaunchProcess::ClearAll()
{	
	_OutputHookThread.WantExit() = true;
	_OutputHookThread.WaitForEnding();

	#define _SafeCloseHandle(x) { if((x)!=INVALID_HANDLE_VALUE){ ::CloseHandle(x); x=INVALID_HANDLE_VALUE; } }

	_SafeCloseHandle(_hProcess);

	_SafeCloseHandle(hChildStdoutRdDup);	//make hook thread exit
	_SafeCloseHandle(hChildStdinRd);
	_SafeCloseHandle(hChildStdinWrDup);
	_SafeCloseHandle(hChildStdoutWr);


	#undef _SafeCloseHandle
}


bool os::LaunchProcess::Launch(LPCSTR cmdline, DWORD flag, DWORD window_show, LPCSTR pWorkDirectory, LPCSTR pEnvVariable)
{
	VERIFY(!IsRunning());

	rt::String cmd(cmdline);
	if(cmd.IsEmpty())return false;

	_Flag = flag;
	bool hook_output = (FLAG_SAVE_OUTPUT&flag) || (FLAG_ROUTE_OUTPUT&flag) || _Callback!=NULL;

	PROCESS_INFORMATION piProcInfo;
	STARTUPINFOW siStartInfo;

	rt::Zero( &piProcInfo, sizeof(PROCESS_INFORMATION) );
	rt::Zero( &siStartInfo, sizeof(STARTUPINFO) );
	siStartInfo.cb = sizeof(STARTUPINFO);
	siStartInfo.wShowWindow = (WORD)window_show;
	siStartInfo.dwFlags = STARTF_USESHOWWINDOW;

	if(hook_output)
	{	
		HANDLE hChildStdoutRd,hChildStdinWr;
		/////////////////////////////////////////////////////////////////////////
		// Creating a Child Process with Redirected Input and Output
		// 1. create pipes
		SECURITY_ATTRIBUTES saAttr;
		bool fSuccess; 

		// Set the bInheritHandle flag so pipe handles are inherited. 
		saAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
		saAttr.bInheritHandle = true; 
		saAttr.lpSecurityDescriptor = NULL;

		// Create a pipe for the child process's STDOUT. 
		::CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &saAttr, 0);

		// Create non-inheritable read handle and close the inheritable read handle. 
		fSuccess = DuplicateHandle(	GetCurrentProcess(), hChildStdoutRd,
									GetCurrentProcess(),&hChildStdoutRdDup , 0,
									false, DUPLICATE_SAME_ACCESS);
		CloseHandle(hChildStdoutRd);

		// Create a pipe for the child process's STDIN. 
		::CreatePipe(&hChildStdinRd, &hChildStdinWr, &saAttr, 0);

		// Duplicate the write handle to the pipe so it is not inherited. 
		fSuccess = DuplicateHandle(	GetCurrentProcess(), hChildStdinWr, 
									GetCurrentProcess(), &hChildStdinWrDup, 0, 
									false, DUPLICATE_SAME_ACCESS); 
		CloseHandle(hChildStdinWr);

		///////////////////////////////////////////////////////
		// 2. create child process
		siStartInfo.hStdError = hChildStdoutWr;
		siStartInfo.hStdOutput = hChildStdoutWr;
		siStartInfo.hStdInput = hChildStdinRd;
		siStartInfo.dwFlags |= STARTF_USESTDHANDLES;

		_hProcess = INVALID_HANDLE_VALUE;
		_Output.Empty();
		_OutputHookThread.Create(OutputHookThread, this);
		_OutputHookThread.SetPriority(os::Thread::PRIORITY_HIGH);
	}

	_ExitCode = STILL_ACTIVE;

	rt::Buffer<WCHAR>	env;
	if(pEnvVariable)
	{
		LPCWSTR s = ::GetEnvironmentStringsW();
		UINT s_len=0;
		for(;;s_len++)
			if(s[s_len] == 0 && s[s_len+1] == 0)
				break;

		UINT a_len=0;
		for(;;a_len++)
			if(pEnvVariable[a_len] == 0 && pEnvVariable[a_len+1] == 0)
				break;

		os::__UTF16 addon(rt::String_Ref(pEnvVariable, a_len));

		env.SetSize(a_len + 1 + s_len + 1 + 1);
		env.Zero();
		memcpy(&env[0], s, sizeof(WCHAR)*s_len);
		memcpy(&env[s_len+1], addon.Begin(), sizeof(WCHAR)*a_len);
		
		pEnvVariable = (LPCSTR)env.Begin();
	}

	// Create the child process. 
	bool ret =
	CreateProcessW(	NULL, 
					os::__UTF16(cmd).Begin(),	// command line 
					NULL,				// process security attributes 
					NULL,				// primary thread security attributes 
					true,				// handles are inherited 
					pEnvVariable?CREATE_UNICODE_ENVIRONMENT:0,				// creation flags 
					(LPVOID)pEnvVariable,
					os::__UTF16(pWorkDirectory),
					&siStartInfo,		// STARTUPINFO pointer 
					&piProcInfo);		// receives PROCESS_INFORMATION 

	if(ret)
	{	
		_hProcess = piProcInfo.hProcess;
		CloseHandle( piProcInfo.hThread );
		return true;
	}
	else
	{
		_LOG_WARNING("Error launching process: "<<cmd<<" ERR: "<<GetLastError());
		ClearAll();
		return false;
	}
}

LPCSTR os::LaunchProcess::GetOutput()
{
	return _Output;
}

UINT os::LaunchProcess::GetOutputLen()
{
	return (UINT)_Output.GetLength();
}

void os::LaunchProcess::CopyOutput(rt::String& out)
{
	EnterCSBlock(_CCS);
	out = _Output;
}

bool os::LaunchProcess::WaitForEnding(DWORD timeout)
{
	os::TickCount t;
	t.LoadCurrentTick();

	::Sleep(500);

	while(IsRunning())
	{	::Sleep(500);
		if((DWORD)t.TimeLapse()>timeout)return false;
	}

	return true;
}

void os::LaunchProcess::Terminate()
{
	if(IsRunning())
	{
		::TerminateProcess(_hProcess,-1);
		os::Sleep(0);
		while(IsRunning())os::Sleep(1);
		ClearAll();
	}
}

bool os::LaunchProcess::IsRunning()
{
	if(_hProcess!=INVALID_HANDLE_VALUE)
	{
		bool exited = false;
		VERIFY(::GetExitCodeProcess(_hProcess,(LPDWORD)&_ExitCode));
		exited = (_ExitCode!=STILL_ACTIVE);

		if(exited)
		{	
			FILETIME creat,exit,foo;
			GetProcessTimes(_hProcess,&creat,&exit,&foo,&foo);

			if(*((__int64*)&exit))
			{
				_ExecutionTime = (UINT)((((ULONGLONG&)exit) - ((ULONGLONG&)creat))/10000);
				_ExitTime = (*((__int64*)&exit))/10000LL - 11644473600000LL;
			
				ClearAll();
				return false;
			}
		}
		return true;
	}

	return false;
}

void os::LaunchProcess::_HookedOutput(char* buffer, UINT dwRead)
{
	if(_Flag&FLAG_ROUTE_OUTPUT){ buffer[dwRead]=0; printf(buffer); }
	if(_Flag&FLAG_SAVE_OUTPUT)
	{	EnterCSBlock(_CCS);
		int i = (int)_Output.GetLength();
		_RemoveCarriageReturn(_Output, rt::String_Ref(buffer, dwRead));
	}
	if(_Callback)
	{	_Callback(buffer, dwRead, _Callback_Cookie);
	}
}


DWORD os::LaunchProcess::OutputHookThread(LPVOID p)
{
	LaunchProcess* pThis = (LaunchProcess*)p;

	char buffer[1024];

	pThis->_ExitCode = STILL_ACTIVE;

	DWORD dwRead,exitcode;
	exitcode = STILL_ACTIVE;
	while(exitcode==STILL_ACTIVE)
	{	
		if(PeekNamedPipe(pThis->hChildStdoutRdDup,NULL,0,NULL,&dwRead,0))
		{	if( dwRead )
			{	if( ReadFile( pThis->hChildStdoutRdDup, buffer, rt::min((DWORD)sizeof(buffer)-1,dwRead), &dwRead, NULL) && dwRead )
				{	pThis->_HookedOutput(buffer,dwRead);
					continue;
				}
			}
		}

		Sleep(100);
		if(pThis->hChildStdoutRdDup==INVALID_HANDLE_VALUE)return 0;
		if(pThis->_hProcess != INVALID_HANDLE_VALUE)
			GetExitCodeProcess(pThis->_hProcess,&exitcode);
		else return 0;
	} 

	// check for words before death of the process
	do
	{	dwRead = 0;
		if(PeekNamedPipe(pThis->hChildStdoutRdDup,NULL,0,NULL,&dwRead,0))
		{	if( dwRead )
			{	if( ReadFile( pThis->hChildStdoutRdDup, buffer, rt::min((DWORD)sizeof(buffer)-1,dwRead), &dwRead, NULL) && dwRead )
					pThis->_HookedOutput(buffer,dwRead);
			}
		}
	}while(dwRead);

	pThis->_ExitCode = exitcode;
	return 0;
}

bool os::LaunchProcess::SendToStdin(LPCVOID str, UINT len)
{
	DWORD wlen = 0;
	return WriteFile(hChildStdinWrDup, str, len, &wlen, NULL) && wlen == len;
}

void os::LaunchProcess::_RemoveCarriageReturn(rt::String& output, const rt::String_Ref& add)
{
	if(add.IsEmpty())return;

	LPCSTR p = add.Begin();
	LPCSTR end = add.End();
	LPCSTR last_linend = NULL;
	LPCSTR last_copied = p;

	if(!output.IsEmpty() && output.Last() == '\r')
	{
		if(*p == '\n')
		{	p++;
			output += '\n';
			last_copied = last_linend = p;
		}
		else
		{	int i = (int)output.FindCharacterReverse('\n');
			if(i<0){ output.Empty(); }
			else{ output.SetLength(i+1); }
			last_linend = p;
		}
	}
	else
	{	LPCSTR s = p;
		while(*s != '\r' && *s != '\n' && s < end)s++;
		if(*s == '\r')
		{
			if(s+1 < end)
			{	
				if(s[1] == '\n')
				{	last_linend = s+2;
				}
				else
				{	int i = (int)output.FindCharacterReverse('\n');
					if(i<0){ output.Empty(); }
					else{ output.SetLength(i + 1); }
					p = last_copied = last_linend = s + 1;
				}
			}
			else
			{	output += rt::String_Ref(p,s+1);
				return;
			}	
		}
		else if(s == end)
		{
			output += add;
			return;
		}
		else
		{	ASSERT(*s == '\n');
			last_linend = p = s+1;
		}
	}

	for(;p < end;p++)
	{

		if(*p == '\n'){ last_linend = p+1; }
		else if(*p == '\r')
		{
			if(p+1<end)
			{
				if(p[1] == '\n')
				{	last_linend = p + 2;
					p++;
				}
				else
				{	ASSERT(last_linend >= last_copied);
					if(last_linend != last_copied)
					{	output += rt::String_Ref(last_copied,last_linend); 
					}
					last_copied = last_linend = p + 1;
				}
			}
			else
			{
				output += rt::String_Ref(last_copied, p+1);
				return;
			}
		}
	}

	if(last_copied < end)
		output += rt::String_Ref(last_copied, end);
}


void os::LaunchProcess::SetOutputCallback(FUNC_HOOKEDOUTPUT func, LPVOID cookie)
{
	_Callback = func;
	_Callback_Cookie = cookie;
}

bool os::LaunchProcess::SendInput(LPCSTR p, UINT len)
{
	if(hChildStdinWrDup)
	{	DWORD w;
		return WriteFile(hChildStdinWrDup, p, len, &w, NULL) && w == len;
	}
	return false;
}


#else


#endif



/////////////////////////////////////////////////
// CParallelFileWriter

ParallelFileWriter::ParallelFileWriter()
{
	_FilenameByLocalTime = false;
	_WriteDownInterval = 0;

	rt::Zero(_WriteBuffers,sizeof(_WriteBuffers));
	_FrontBuf = _WriteBuffers;
	_BackBuf = _FrontBuf + 1;

	//rt::Zero(_InMemoryCopy,sizeof(_InMemoryCopy));
	//_InMemoryCopy_FrontBuf = _InMemoryCopy;
	//_InMemoryCopy_BackBuf = _InMemoryCopy_FrontBuf + 1;

	stat_BufferUsagePeek = 0;
	stat_BufferUsageAvg = 0;
	stat_FileIOUsageAvg = 0;
	stat_FileIOUsagePeek = 0;

	SetWriteDownInterval(1000);

	_pFileWriterCB = NULL;

	//_DesiredFileLength = 0;
	//SetTruncationBoundaryTag('\n',1);
}

ParallelFileWriter::~ParallelFileWriter()
{
	Close();
}

void ParallelFileWriter::SwitchTo(LPCSTR filename, bool no_wait)
{
	ASSERT(_File.IsOpen() && _WriteThread.IsRunning());

	EnterCSBlock(_FilenameTemplateCS);
	_FilenameTemplate = filename;
	_FilenameChanged = true;

	if(!no_wait)
	{	while(_FilenameChanged)
			os::Sleep(100);
	}
}

void ParallelFileWriter::SetWriteDownInterval(UINT sync_interval)
{
	_WriteDownInterval = rt::max<UINT>(100,sync_interval);
	_AvgAttenuation = pow(0.64, sync_interval/1000.0);		// Attenuate to 0.01 after 10 sec
	ASSERT_FLOAT(_AvgAttenuation);
}

void ParallelFileWriter::RealizeFilename(const rt::String& file_template, const os::Timestamp::Fields& f, rt::String& filename)
{
	if(file_template.FindCharacter('%')>=0)
	{
		file_template.ReplaceTo("%YEAR%", rt::tos::Number(f.Year), filename);
		filename.Replace("%MONTH%", rt::tos::Number(f.Month).RightAlign(2,'0'));
		filename.Replace("%DAY%", rt::tos::Number(f.Day).RightAlign(2,'0'));
		filename.Replace("%HOUR%", rt::tos::Number(f.Hour).RightAlign(2,'0'));
		filename.Replace("%HALFHOUR%", rt::tos::Number(f.Minute/30*30).RightAlign(2,'0'));
		filename.Replace("%QUARTER%", rt::tos::Number(f.Minute/15*15).RightAlign(2,'0'));
		filename.Replace("%MINUTE%", rt::tos::Number(f.Minute).RightAlign(2,'0'));
	}
	else
	{	filename = file_template;
	}

}

void ParallelFileWriter::_RealizeFilename(rt::String& filename)
{
	os::Timestamp::Fields f;
	if(_FilenameByLocalTime)
		os::Timestamp::Get().GetLocalDateTime(f);
	else
		os::Timestamp::Get().GetDateTime(f);

	RealizeFilename(_FilenameTemplate, f, filename);
}

bool ParallelFileWriter::Open(LPCSTR filename, bool append_existing, UINT buffer_size, bool filetime_by_localtime)
{
	EnterCSBlock(_FilenameTemplateCS);
	ASSERT(!_File.IsOpen() && !_WriteThread.IsRunning());

	_FilenameByLocalTime = filetime_by_localtime;
	_FilenameTemplate = filename;
	_FilenameChanged = false;

	stat_ClaimPerSecond = -1;
	stat_BufferUsagePeek = 0;
	stat_BufferUsageAvg = 0;
	stat_FileError = 0;
	stat_UnfinalizedChunk = 0;
	stat_ClaimFailure = 0;
	stat_FinalizeFailure = 0;
	stat_FileIOUsageAvg = 0;			// percentage
	stat_FileIOUsagePeek = 0;			// percentage
	stat_TotalClaimed = 0;

	_FileOpenFlag = append_existing?os::File::Normal_Append:os::File::Normal_Write;

	_WriteBufferSize = (buffer_size + 3) & 0xffffffffc;

	rt::String fn;
	_RealizeFilename(fn);

	if(	_File.Open(fn,_FileOpenFlag, true) &&
		_WritePool.SetSize(2*_WriteBufferSize)
	)
	{	_File.SeekToEnd();

		_FrontBuf->pBuf = _WritePool;
		_FrontBuf->Used = 0;
		_BackBuf->pBuf = &_WritePool[_WriteBufferSize];
		_BackBuf->Used = 0;

		struct _call
		{	static DWORD _func(LPVOID pThis)
			{	((ParallelFileWriter*)pThis)->_WriteRoute();
				return 0;
			}
		};

		_WriteThread.Create(_call::_func,this);

		return true;
	}
	
	if(_File.IsOpen())
		_File.Close();

	_WritePool.SetSize(0);
	return false;
}

void ParallelFileWriter::PreClose()
{
	if(_WriteThread.IsRunning())
	{
		_WriteThread.WantExit() = true;
	}
}

void ParallelFileWriter::Close()
{
	stat_ClaimPerSecond = -1;

	if(_WriteThread.IsRunning())
	{
		_WriteThread.WantExit() = true;
		_WriteThread.WaitForEnding(1000, true);
	}
	
	if(_File.IsOpen())
		_File.Close();

	//_WritePool.SetSize(0);
}

void ParallelFileWriter::SetFileWritingCallback(FUNC_FileWriter cb, LPVOID cookie)
{
	_pFileWriterCB = cb;
	_FileWriterCookie = cookie;
}

void ParallelFileWriter::_WriteDownBackBuffer()
{
	// dump to file only
	_Chunk* p = (_Chunk*)_BackBuf->pBuf;
	int used = _BackBuf->Used;
	_Chunk* pend = (_Chunk*)(_BackBuf->pBuf + rt::min<int>(used,(int)(_WriteBufferSize - _ChunkHeaderSize)));
	
	for(;p<pend;p = p->GetNext())
	{	
		if(p->size)
		{
			if(p->length==INFINITE)
			{	stat_UnfinalizedChunk++;
				continue;
			}

			if(p->length>0)
			{	if(_pFileWriterCB)
				{	if(!_pFileWriterCB(p->data,p->length,_File,_FileWriterCookie))
					{	stat_FileError++;
					}
				}
				else
				{	if(_File.Write(p->data,p->length) != p->length)
					{	stat_FileError++;
					}
				}
			}
		}
		else break;
	}
	_File.Flush();

	// Use Rate
	{	int fr;
		if(_BackBuf->Used >= (int)_WriteBufferSize)
		{	fr = 100;	}
		else
		{	fr = 100*(_BackBuf->Used/1024)/(_WriteBufferSize/1024);	}

		if(stat_BufferUsagePeek)
		{
			if(fr > stat_BufferUsagePeek)stat_BufferUsagePeek = fr;
			stat_BufferUsageAvg = (int)(fr + _AvgAttenuation*(stat_BufferUsageAvg - fr) + 0.5);
		}
		else
		{	stat_BufferUsagePeek = fr;
			stat_BufferUsageAvg = fr;
		}
	}

	_BackBuf->Used = 0;
	rt::Zero(_BackBuf->pBuf,_WriteBufferSize);
}

void ParallelFileWriter::LogAlert()
{
	static const rt::SS precentage("% / ");
	_LOG_WARNING("ParallelFileWriter Alert"<<
					"\n - Total Claim      : "<<stat_TotalClaimed<<
					"\n - Claim Failure    : "<<stat_ClaimFailure<<
					"\n - Finalize Failure : "<<stat_FinalizeFailure<<
					"\n - Unfinalized      : "<<stat_UnfinalizedChunk<<
					"\n - File I/O Failure : "<<stat_FileError<<
					"\n - Buffer Load      : "<<stat_BufferUsageAvg<<precentage<<stat_BufferUsagePeek<<
					"%\n - File I/O Load    : "<<stat_FileIOUsageAvg<<precentage<<stat_FileIOUsagePeek<<'%'
	);
}

void ParallelFileWriter::GetCurrentLogFilename(rt::String& filename)
{
	EnterCSBlock(_FilenameTemplateCS);
	filename = _File.GetFilename();
}

void ParallelFileWriter::_WriteRoute()
{
	int time_to_sleep = _WriteDownInterval;
	os::Timestamp tm;
	rt::String fn;

	LONGLONG minute_last_checked = -1;

	int _last_claim = 0;
	os::Timestamp _last_cps;
	_last_cps.LoadCurrentTime();

	for(;;)
	{
		while(time_to_sleep > 300)
		{	os::Sleep(300);
			time_to_sleep -= 300;
			if(_WriteThread.WantExit())
				goto CLOSING_FILE;
		}

		ASSERT(time_to_sleep > 0);
		os::Sleep(time_to_sleep);

		tm.LoadCurrentTime();
		LONGLONG minute = tm/(60*1000);
		
		if(_FilenameChanged || minute != minute_last_checked)
		{
			EnterCSBlock(_FilenameTemplateCS);

			_RealizeFilename(fn);
			if(_File.GetFilename() != fn)
			{	os::File	_log;
				if(_log.Open(fn,_FileOpenFlag))
				{	_log.SeekToEnd();
					rt::Swap(_log,_File);
				}
				else
				{	stat_FileError++;
				}	
			}

			_FilenameChanged = false;
			minute_last_checked = minute;
		}

		_WriteDownBackBuffer();
		rt::Swap(_BackBuf,_FrontBuf);

		{	float cps = (stat_TotalClaimed - _last_claim)*1000/(float)(_last_cps.TimeLapse(tm));
			if(stat_ClaimPerSecond<0)
				stat_ClaimPerSecond = cps;
			else stat_ClaimPerSecond = (stat_ClaimPerSecond*9 + cps)/10;

			_last_claim = stat_TotalClaimed;
			_last_cps = tm;
		}

		if(stat_ClaimFailure || stat_FinalizeFailure || stat_FileError || stat_UnfinalizedChunk)
		{
			LogAlert();
		}

		int time_used = (int)tm.TimeLapse();

		{	int IOUage;
			if(	time_used < (int)_WriteDownInterval )
				IOUage = 100*time_used/_WriteDownInterval;
			else
				IOUage = 100;

			if( stat_FileIOUsagePeek )
			{
				if(IOUage>stat_FileIOUsagePeek)stat_FileIOUsagePeek = IOUage;
				stat_FileIOUsageAvg = (int)(IOUage + _AvgAttenuation*(stat_FileIOUsageAvg - IOUage) + 0.5);
			}
			else
			{
				stat_FileIOUsagePeek = IOUage;
				stat_FileIOUsageAvg = IOUage;
			}
		}

		time_to_sleep = rt::min<int>(_WriteDownInterval, rt::max<int>(100, _WriteDownInterval - time_used));
	}	

CLOSING_FILE:

	_WriteDownBackBuffer();
	rt::Swap(_BackBuf,_FrontBuf);

	_WriteDownBackBuffer();

	_File.Close();
}

} // namespace os


