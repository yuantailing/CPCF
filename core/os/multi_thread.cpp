#include "multi_thread.h"
#include "../rt/buffer_type.h"
#include "kernel.h"

/////////////////////////////////////////////////////////////////
// Platform independent implementations
//////////////////////////////////////////////////
// GarbageCollection
namespace os
{
namespace _details
{
	struct _GarbagItem
	{	LPVOID			pObj;
		os::GarbageCollection::LPFUNC_DELETION	pDeletionFunction;
		int				TTL;
		void			Delete(){ pDeletionFunction(pObj); }
		bool			Tick() // return true when the item should be NOT deleted
		{	if(TTL)TTL--;
			return TTL;
		}
	};

	struct _GarbagBin
	{
		rt::BufferEx<_GarbagItem>		_PendingGarbag;
		os::CriticalSection				_PendingGarbagCCS;
		os::Thread						_GarbagDeletionThread;
		~_GarbagBin(){ Exit(); }
		void Exit()
		{
			if(_GarbagDeletionThread.IsRunning())
			{
				_GarbagDeletionThread.WantExit() = true;
				_GarbagDeletionThread.WaitForEnding();
			}
		}
	};
	_GarbagBin				g_GCB;
}
} // namespace os

void os::GarbageCollection::Exit()
{
	_details::g_GCB.Exit();
}

DWORD os::GarbageCollection::_DeletionThread(LPVOID)
{
	for(;;)
	{
		for(UINT i=0;i<5;i++)
		{
			os::Sleep(200);
			if(_details::g_GCB._GarbagDeletionThread.WantExit())goto DELETION_EXITING;
		}

		{	EnterCSBlock(_details::g_GCB._PendingGarbagCCS);
			UINT open = 0;
			for(UINT i=0;i<_details::g_GCB._PendingGarbag.GetSize();i++)
			{
				if(_details::g_GCB._PendingGarbag[i].Tick())
				{
					_details::g_GCB._PendingGarbag[open] = _details::g_GCB._PendingGarbag[i];
					open++;
				}
				else
				{
					_details::g_GCB._PendingGarbag[i].Delete();
				}
			}

			_details::g_GCB._PendingGarbag.ChangeSize(open);
		}
	}

DELETION_EXITING:
	{	EnterCSBlock(_details::g_GCB._PendingGarbagCCS);
		for(UINT i=0;i<_details::g_GCB._PendingGarbag.GetSize();i++)
			_details::g_GCB._PendingGarbag[i].Delete();

		_details::g_GCB._PendingGarbag.SetSize();
	}

	return 0;
}


void os::GarbageCollection::DeleteObject(LPVOID x, DWORD TTL_msec, os::GarbageCollection::LPFUNC_DELETION delete_func)
{
	ASSERT(delete_func);

	if(x == NULL)return;
	if(TTL_msec == 0)
	{	delete_func(x);
		return;
	}

	{	EnterCSBlock(_details::g_GCB._PendingGarbagCCS);
		if(_details::g_GCB._GarbagDeletionThread.IsRunning()){}
		else
		{	_details::g_GCB._GarbagDeletionThread.Create(_DeletionThread,NULL);
		}

#ifdef PLATFORM_DEBUG_BUILD
		// check if the object is added for collection already (delete twice)
		for(UINT i=0;i<_details::g_GCB._PendingGarbag.GetSize();i++)
			ASSERT(_details::g_GCB._PendingGarbag[i].pObj != x);
#endif

		_details::_GarbagItem& n = _details::g_GCB._PendingGarbag.push_back();
		n.pObj = x;
		n.pDeletionFunction = delete_func;
		n.TTL = (TTL_msec+999)/1000;
	}
}

bool os::Thread::WaitForEnding(UINT time_wait_ms, bool terminate_if_timeout)
{
	Sleep(0);
	while(time_wait_ms > (UINT)100)
	{	
		if(!IsRunning())return true;
		Sleep(100);
		if(time_wait_ms!=INFINITE)time_wait_ms -= 100;
	}

#if !defined(PLATFORM_DEBUG_BUILD)
	if(terminate_if_timeout)TerminateForcely();
#endif
	return false;
}

#ifdef PLATFORM_DEBUG_BUILD
namespace os
{
namespace _details
{

bool _TMA_Exit = false;

struct _TMA
{
	struct MemBlock
	{
		SIZE_T		Size;
		char		Memo[1];
	};

	os::CriticalSection _CS;
	rt::hash_map<SIZE_T, MemBlock*>	_TrackedMemory;
	~_TMA(){ _TMA_Exit = true; }
};

_TMA& _GetTMA()
{
	static _TMA _;
	return _;
}

LPVOID TrackMemoryAllocation(LPVOID p, SIZE_T sz, bool no_ctor, LPCSTR type, UINT co, LPCSTR fn, LPCSTR func, UINT line)
{
	if(_TMA_Exit)return p;

	EnterCSBlock(_GetTMA()._CS);
	ASSERT(_GetTMA()._TrackedMemory.find((SIZE_T&)p) == _GetTMA()._TrackedMemory.end());

	// make sure zero memory footprint on heap
	LPCSTR s1 = no_ctor?"Malloc ":"New ";
	LPCSTR s2, s3;

	if(co>1)
	{	auto x = rt::String_Ref(type) + '[' + co + ']';
		s2 = ALLOCA_C_STRING(x);
	}
	else
	{	s2 = type;
	}

	{	auto x = rt::SS(" in ") + rt::String_Ref(fn).GetFilename() + rt::SS(":L") + line + "\n    by " + func + "()";
		s3 = ALLOCA_C_STRING(x);
	}

	auto s = rt::SS() + s1 + s2 + s3;
	_TMA::MemBlock* n = (_TMA::MemBlock*) new (std::nothrow) BYTE[s.GetLength() + sizeof(SIZE_T) + 1];
	ASSERT(n);

	n->Size = sz;
	n->Memo[s.CopyTo(n->Memo)] = 0;

	_GetTMA()._TrackedMemory[(SIZE_T&)p] = n;
	return p;
}

void UntrackMemoryAllocation(LPCVOID p)
{
	if(_TMA_Exit)return;

	if(p)
	{
		EnterCSBlock(_GetTMA()._CS);
		auto it = _GetTMA()._TrackedMemory.find((SIZE_T&)p);
		ASSERT(it != _GetTMA()._TrackedMemory.end());

		delete [] (LPBYTE)it->second;
		_GetTMA()._TrackedMemory.erase(it);
	}
}

void DumpTrackedMemoryAllocation(bool verbose)
{
	if(_TMA_Exit)return;

	if(verbose)_LOG(" ");

	EnterCSBlock(_GetTMA()._CS);
	SIZE_T tot = 0;
	if(_GetTMA()._TrackedMemory.size())
	{
		if(verbose)_LOG("Dump Tracked Memory Blocks ("<<_GetTMA()._TrackedMemory.size()<<"):");
		for(auto it = _GetTMA()._TrackedMemory.begin(); it != _GetTMA()._TrackedMemory.end(); it++)
		{
			_LOG("[0x"<<it->first<<"] "<<it->second->Size<<"B "<<it->second->Memo);
			if(verbose)
			{
				auto x = rt::String_Ref((LPCSTR)it->first, rt::min(72, (int)it->second->Size));
				LPSTR d = ALLOCA_C_STRING(x);
				for(int i=0; d[i]; i++)
					if(d[i]<' ')d[i] = ' ';
				_LOG("    = \""<<d<<'"');
			}
			tot += it->second->Size;
		}
		if(verbose)_LOG("Tracked memory blocks take "<<rt::tos::FileSize<>(tot));
	}
	else
	{	if(verbose)_LOG("No tracked memory blocks");
	}
}

}} // namespace os::_details
#endif

namespace os
{
namespace _details
{
	struct _thread_call
	{	
#if defined(PLATFORM_ANDROID)
		static void thread_exit_handler(int sig)
		{ 	pthread_exit(0);
		}
#endif
		struct _thread_class:public os::Thread
		{	void __clear_after_run(DWORD exitcode)
			{	ExitCode = exitcode;
                while(!hThread)os::Sleep(10); // creator thread is blocked
				__release_handle(hThread);
				hThread = NULL;
			}
		};
		FUNC_THREAD_ROUTE	x;
		LPVOID				thread_cookie;
		Thread*				pThis;
		INLFUNC	DWORD run()
		{	
#if defined(PLATFORM_ANDROID)
			struct sigaction actions;
			memset(&actions, 0, sizeof(actions)); 
			sigemptyset(&actions.sa_mask);
			actions.sa_flags = 0; 
			actions.sa_handler = thread_exit_handler;
			sigaction(SIGUSR2,&actions,NULL);
#endif
			DWORD ret = x(thread_cookie);
			if(ret != os::Thread::THREAD_OBJECT_DELETED_ON_RETURN)
				((_thread_class*)pThis)->__clear_after_run(ret);
			_SafeDel_Const(this);
			return ret;
		}
		_thread_call(FUNC_THREAD_ROUTE xi, LPVOID thread_cookiei, Thread* pThisi)
		{	x = xi;
			thread_cookie = thread_cookiei;
			pThis = pThisi;
		}
	};
}


} // namespace os

//////////////////////////////////////////////////////////
// All Windows implementations
#if defined(PLATFORM_WIN)

void os::Thread::__release_handle(HANDLE hThread)
{
	::CloseHandle(hThread);
}

bool os::Thread::Create(os::FUNC_THREAD_ROUTE x, LPVOID thread_cookie, UINT stack_size)
{
	ASSERT(hThread == NULL);
	bWantExit = false;

	struct _call
	{	static DWORD WINAPI _func(LPVOID p)
		{	return ((os::_details::_thread_call*)p)->run();
	}	};
	hThread = ::CreateThread(NULL, stack_size, _call::_func, _New(os::_details::_thread_call(x,thread_cookie,this)), 0, &ThreadId);
	return hThread != NULL;
}

SIZE_T os::Thread::GetId()
{
	return ThreadId;
}

void os::Thread::Suspend()
{
	ASSERT(hThread != NULL);
	::SuspendThread(hThread);
}

void os::Thread::Resume()
{
	ASSERT(hThread != NULL);
	::ResumeThread(hThread);
}


SIZE_T os::Thread::GetCurrentId()
{
	return (SIZE_T)::GetCurrentThreadId();
}

void os::Thread::SetPriority(UINT p)
{
	SetThreadPriority(hThread, p);
}

void os::Thread::TerminateForcely()
{
	if(hThread)
	{
		::TerminateThread(hThread, -1);
		__release_handle(hThread);
		hThread = NULL;
	}
}

bool os::Thread::SetAffinityMask(SIZE_T x)
{
	return 0 != SetThreadAffinityMask(hThread, ((DWORD_PTR&)x));
}


os::CriticalSection::CriticalSection()
{
	InitializeCriticalSection(&hCS);
	_OwnerTID = 0;
}

os::CriticalSection::~CriticalSection()
{
	ASSERT(_OwnerTID == 0);
	DeleteCriticalSection(&hCS);
}

os::Event::Event()
{
	VERIFY(hEvent = ::CreateEvent(NULL,true,false,NULL));
}

os::Event::~Event()
{
	::CloseHandle(hEvent);
}



#else
//////////////////////////////////////////////////////////
// All linux/BSD implementations
#include <pthread.h>

bool os::Thread::Create(os::FUNC_THREAD_ROUTE x, LPVOID thread_cookie, UINT stack_size)
{
	ASSERT(hThread == NULL);
	bWantExit = false;

	struct _call
	{	static LPVOID _func(LPVOID p)
		{	return (LPVOID)(SIZE_T)((os::_details::_thread_call*)p)->run();
	}	};

	pthread_attr_t attr;
	pthread_attr_t* set_attr = NULL;

	if(stack_size)
	{	pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, rt::max((int)PTHREAD_STACK_MIN,(int)stack_size));
		set_attr = &attr;
	}

	if(0 == pthread_create((pthread_t*)&hThread, set_attr, _call::_func, _New(os::_details::_thread_call(x,thread_cookie,this))))
		return true;
	hThread = NULL;
	return false;
}



SIZE_T os::Thread::GetCurrentId()
{
#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
	return (SIZE_T)pthread_mach_thread_np(pthread_self());
#else
	return pthread_self();
#endif
}

SIZE_T os::Thread::GetId()
{
#if defined(PLATFORM_IOS) || defined(PLATFORM_MAC)
	return (SIZE_T)pthread_mach_thread_np(*(pthread_t*)&hThread);
#else
	return *(SIZE_T*)&hThread;
#endif
}

void os::Thread::SetPriority(UINT p)
{
	struct sched_param sp;
	sp.sched_priority = p;
	pthread_setschedparam(*(pthread_t*)&hThread, SCHED_OTHER, &sp);
}

void os::Thread::Suspend()
{
	ASSERT(hThread == NULL);
	ASSERT(0); // not implemented
}

void os::Thread::Resume()
{
	ASSERT(hThread == NULL);
	ASSERT(0); // not implemented
}


void os::Thread::TerminateForcely()
{
	if(hThread)
	{
#if defined(PLATFORM_ANDROID)
		pthread_kill(*(pthread_t*)&hThread, SIGUSR2);
#else
		pthread_cancel(*(pthread_t*)&hThread);
#endif
		hThread = NULL;
	}
}

bool os::Thread::SetAffinityMask(SIZE_T x)
{
	ASSERT(0);
	return false;
}


void os::Thread::__release_handle(HANDLE hThread)
{
	pthread_detach((pthread_t&)hThread);
}

os::CriticalSection::CriticalSection()
{
	pthread_mutexattr_t attributes;
	pthread_mutexattr_init(&attributes);
	pthread_mutexattr_settype(&attributes, PTHREAD_MUTEX_RECURSIVE);
	
	VERIFY(0==pthread_mutex_init(&hCS, &attributes));
	
	pthread_mutexattr_destroy(&attributes);
	_OwnerTID = 0;
}

os::CriticalSection::~CriticalSection()
{
	pthread_mutex_destroy(&hCS);
	ASSERT(_OwnerTID == 0);
}

os::Event::Event()
{
	VERIFY(0 == pthread_cond_init(&hEvent,NULL)); 
	bSet = false;
}

os::Event::~Event()
{	
	pthread_cond_destroy(&hEvent);
}

#endif


