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

namespace os
{
namespace _details
{
thread_local bool g_IsMemoryExceptionEnabled = false;
} // namespace _details

bool IsMemoryExceptionEnabledInThread()
{
	return _details::g_IsMemoryExceptionEnabled;
}

void EnableMemoryExceptionInThread(bool y)
{
	_details::g_IsMemoryExceptionEnabled = y;
}
} // namespace os


namespace os
{
namespace _details
{
///////////////////////////////////////////////////////////////////////////////////////////////////
//// 32byte Aligned Memory allocation layout
//// additional 32+1+4 bytes (or 32+1+8 bytes in Win64) is added to every memory block
//// 
////                 |----  Offset  ----|
//// /-- LEN-BYTE --\/-- Prefix bytes -\#/---------- User block -----------\/-- Suffix bytes --\
//// UUUUUUUUUUUUUUUUU ......... UUUUUUUUUUUUUUUUUUUUUUUUUU .... UUUUUUUUUUUUUUUUUUUUUUUUUUUUUUU
//// ^                                  ^^
//// |Original                          ||
//// \ allocated           Saved Offset /\ First aligned address after LEN-BYTE, the return 
////
//// 1. LEN-BYTE (size_t) indicates the size of User block
//// 
//// 2. If the p is returned by malloc and subtracted by 8, then &p[32-p%32] is the output address
////    and the offset=32-p%32 is saved at p[offset-1] as one byte the legal range of offset is from
////    1 to 32, and this value will be checked when free and used to calculate the original address
//// 
//// 3. The Prefix bytes (size=offset-1) and Suffix bytes (size=33-offset) will be check to ensure 
////    no boundary memory written occurred (buffer overflow)
/////////////////////////////////////////////////////////////////////////////////////////////////////

LPVOID Malloc32AL(size_t size, bool allow_fail)   //size in byte
{
#pragma warning(disable:4244 4127)	// conversion from 'int' to 'BYTE', possible loss of data
									// conditional expression is constant
	LPBYTE p;
	//32 for alignment, 1 for suffix, 4(or 8) for LEN-BYTE
	if(g_IsMemoryExceptionEnabled)
	{
		if(size >= size+32+1+sizeof(size_t))
			throw std::bad_alloc();

		p = new BYTE[size+32+1+sizeof(size_t)]; 
	}
	else
	{
		if(size >= size+32+1+sizeof(size_t))
			return NULL;

		p = new(std::nothrow) BYTE[size+32+1+sizeof(size_t)];
	}

#if defined CPF_MEMORY_LEAK_ADDRESS
	if( p == (LPBYTE)CPF_MEMORY_LEAK_ADDRESS )
	{	ASSERT(0);
		//Leaked memory block is located, check [Call Stack] for high-level code
	}
#endif

	if(p)
	{	//Record user block size for determining the high boundary of the memory block
		*((size_t*)p) = size; //size of User block
		p += sizeof(size_t);
		int offset;
		offset = 32 - (int)( ((size_t)p) & 0x1f);	// align to 32
		p[offset-1] = offset;	//Store offset

#if defined CPF_MEMORY_LEAK_ADDRESS
	if( &p[offset] == (LPBYTE)CPF_MEMORY_LEAK_ADDRESS )
	{	ASSERT(0);
		//abnormal used memory block is located, check [Call Stack] for high-level code
	}
#endif
		//Additional guard bytes is set for detecting broundry-overwriting, 
		//which will be checked when memory free. 
		int i;
		for(i=0;i<offset-1;i++)p[i] = 0x61+i;				//Set prefix bytes to 0x61+i = "abcde..."
		for(i=0;i<33-offset;i++)p[offset+size+i] = 0x41+i;	//Set suffix bytes to 0x41+i = "ABCDE..."

#ifdef CPF_REPORT_MEMORY_ACTIVITY
		_LOG("User block [0x"<<((LPVOID)&p[offset])<<"] allocated.\n");
#endif

		return &p[offset];
	}
	else  //Allocation failed
	{	
#ifdef CPF_REPORT_MEMORY_ACTIVITY
		_LOG("\n\nOut of memory: Failed to allocate "<<((size+41)/1024)<<"KB memory block!\7\n");
#endif
		ASSERT(0);
		return NULL;
	}
}

void Free32AL(LPCVOID ptr_in)
{
	if( ptr_in ) //NULL pointer is allowed but take no effect
	{	
#ifdef CPF_REPORT_MEMORY_ACTIVITY
		_LOG(_T("User block [0x")<<ptr_in<<_T("] try to free.\n"));
#endif
		LPCBYTE ptr = reinterpret_cast<LPCBYTE>(ptr_in);

		bool Prefix_Err_Detected = false; 
		bool Suffix_Err_Detected = false;
		size_t user_block_size = ((size_t)(-1));
		int offset = ptr[-1];
		if( offset <=32 && offset>0 )
		{	//Detect buffer overrun
			user_block_size = *((size_t*)&ptr[-offset-sizeof(size_t)]);

			int i; 
			for(i=0;i<offset-1;i++)
			{// check prefix bytes
				if( ptr[-(offset-i)] == 0x61+i ){}
				else{ Prefix_Err_Detected = true; break; }
			}
			for(i=0;i<33-offset;i++)
			{// check suffix bytes
				if( ptr[user_block_size+i] == 0x41+i ){}
				else{ Suffix_Err_Detected = true; break; }
			}

			if( !Prefix_Err_Detected && !Suffix_Err_Detected )
			{	
				//ensure heap correctness 
				//_CheckHeap;
			
#ifdef CPF_SECURE_MEMORY_RELEASE
				rt::Zero((LPVOID)ptr_in, user_block_size);	// security enforcement
#endif
				delete [] (&ptr[-offset-sizeof(size_t)]);
				return;
			}
		}
		else{ Prefix_Err_Detected = true; }

		LPCSTR emsg = NULL;
		if(Prefix_Err_Detected && Suffix_Err_Detected)
		{	emsg = ("Both low and high");	}
		else if( Prefix_Err_Detected )
		{	emsg = ("Low");	}
		else if( Suffix_Err_Detected )
		{	emsg = ("High");	}
		else{ ASSERT(0); }

		_LOG("Abnormal block at 0x"<<((LPVOID)ptr)<<", "<<emsg<<" boundry was overwritten.");

		ASSERT(0);
	}
}
#pragma warning(default:4244 4127)

}} // namespace os::_details


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
	if(_TMA_Exit || p == NULL)return p;

	EnterCSBlock(_GetTMA()._CS);
	ASSERT(_GetTMA()._TrackedMemory.find((SIZE_T&)p) == _GetTMA()._TrackedMemory.end());

	// make sure zero memory footprint on heap
	LPCSTR s1 = no_ctor?"Malloc ":"New ";
	LPCSTR s2, s3;

	rt::String_Ref type_trim = rt::String_Ref(type).TrimAfter('(');  // remove ctor arguments

	if(co>1)
	{	auto x = type_trim + '[' + co + ']';
		s2 = ALLOCA_C_STRING(x);
	}
	else
	{	s2 = ALLOCA_C_STRING(type_trim);
	}

	{	auto x = rt::SS(" in ") + rt::String_Ref(fn).GetFilename() + rt::SS(":L") + line + "\n    by " + func + "()";
		s3 = ALLOCA_C_STRING(x);
	}

	auto s = rt::SS() + s1 + s2 + s3;

	_TMA::MemBlock* n;
	if(g_IsMemoryExceptionEnabled)
		n = (_TMA::MemBlock*) new BYTE[s.GetLength() + sizeof(SIZE_T) + 1];
	else
		n = (_TMA::MemBlock*) new (std::nothrow) BYTE[s.GetLength() + sizeof(SIZE_T) + 1];

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

os::Thread::Thread()
{
	__MFPTR_Obj = NULL;
	__CB_Func = NULL;
	_hThread = NULL;
	_ExitCode = INFINITE;
}

os::Thread::~Thread()
{
	if(_hThread)
		__release_handle(_hThread);
}

bool os::Thread::Create(LPVOID obj, const THISCALL_MFPTR& on_run, LPVOID param, UINT stack_size)
{
	ASSERT(_hThread == NULL);

	__CB_Func = NULL;
	__MFPTR_Obj = obj;
	__MFPTR_Func = on_run;
	__CB_Param = param;

	return _Create(stack_size);
}

bool os::Thread::Create(FUNC_THREAD_ROUTE x, LPVOID thread_cookie, UINT stack_size)
{
	ASSERT(_hThread == NULL);

	__CB_Func = x;
	__CB_Param = thread_cookie;
	__MFPTR_Obj = NULL;
	__MFPTR_Func.Zero();

	return _Create(stack_size);
}

DWORD os::Thread::_Run()
{
#if defined(PLATFORM_ANDROID)
	struct _thread_call
	{	static void thread_exit_handler(int sig)
		{ 	pthread_exit(0);
	}	};

	struct sigaction actions;
	memset(&actions, 0, sizeof(actions)); 
	sigemptyset(&actions.sa_mask);
	actions.sa_flags = 0; 
	actions.sa_handler = thread_exit_handler;
	sigaction(SIGUSR2,&actions,NULL);
#endif

	while(!_hThread)os::Sleep(10);
	os::Sleep(10);

	DWORD ret;
	if(__MFPTR_Obj)
	{
		ret = THISCALL_POLYMORPHISM_INVOKE(OnRun, __MFPTR_Obj, __MFPTR_Func, __CB_Param);
	}
	else
	{	
		ASSERT(__CB_Func);
		ret = __CB_Func(__CB_Param);
	}

	if(ret != THREAD_OBJECT_DELETED_ON_RETURN)
	{
		_ExitCode = ret;
		__release_handle(_hThread);
		_hThread = NULL;
	} // else never touch this again.

	return ret;
}

//////////////////////////////////////////////////////////
// All Windows implementations
#if defined(PLATFORM_WIN)

void os::Thread::__release_handle(HANDLE hThread)
{
	::CloseHandle(hThread);
}

bool os::Thread::_Create(UINT stack_size)
{
	ASSERT(_hThread == NULL);
	_bWantExit = false;

	struct _call
	{	static DWORD _func(LPVOID p)
		{	return ((Thread*)p)->_Run();
	}	};

	_hThread = ::CreateThread(NULL, stack_size, _call::_func, this, 0, &_ThreadId);
	return _hThread != NULL;
}

SIZE_T os::Thread::GetId()
{
	return _ThreadId;
}

void os::Thread::Suspend()
{
	ASSERT(_hThread != NULL);
	::SuspendThread(_hThread);
}

void os::Thread::Resume()
{
	ASSERT(_hThread != NULL);
	::ResumeThread(_hThread);
}

SIZE_T os::Thread::GetCurrentId()
{
	return (SIZE_T)::GetCurrentThreadId();
}

void os::Thread::SetPriority(UINT p)
{
	SetThreadPriority(_hThread, p);
}

void os::Thread::TerminateForcely()
{
	if(_hThread)
	{
		::TerminateThread(_hThread, -1);
		__release_handle(_hThread);
		_hThread = NULL;
	}
}

bool os::Thread::SetAffinityMask(SIZE_T x)
{
	return 0 != SetThreadAffinityMask(_hThread, ((DWORD_PTR&)x));
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

bool os::Thread::_Create(UINT stack_size)
{
	ASSERT(_hThread == NULL);
	_bWantExit = false;

	pthread_attr_t attr;
	pthread_attr_t* set_attr = NULL;

	if(stack_size)
	{	pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, rt::max((int)PTHREAD_STACK_MIN,(int)stack_size));
		set_attr = &attr;
	}

	struct _call
	{	static LPVOID _func(LPVOID p)
		{	return (LPVOID)((Thread*)p)->_Run();
	}	};

	if(0 == pthread_create((pthread_t*)&_hThread, set_attr, _call::_func, this))
		return true;

	_hThread = NULL;
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
	return (SIZE_T)pthread_mach_thread_np(*(pthread_t*)&_hThread);
#else
	return *(SIZE_T*)&_hThread;
#endif
}

void os::Thread::SetPriority(UINT p)
{
	struct sched_param sp;
	sp.sched_priority = p;
	pthread_setschedparam(*(pthread_t*)&_hThread, SCHED_OTHER, &sp);
}

void os::Thread::Suspend()
{
	ASSERT(_hThread == NULL);
	ASSERT(0); // not implemented
}

void os::Thread::Resume()
{
	ASSERT(_hThread == NULL);
	ASSERT(0); // not implemented
}


void os::Thread::TerminateForcely()
{
	if(_hThread)
	{
#if defined(PLATFORM_ANDROID)
		pthread_kill(*(pthread_t*)&_hThread, SIGUSR2);
#else
		pthread_cancel(*(pthread_t*)&_hThread);
#endif
		_hThread = NULL;
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


