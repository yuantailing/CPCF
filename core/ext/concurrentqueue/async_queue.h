#pragma once
#include "../../../../CPCF/essentials.h"
#include "blockingconcurrentqueue.h"
#include "readerwriterqueue.h"

namespace ext
{
	
namespace _details
{

template<UINT PL_MAX>
struct AsyncData
{
	union
	{	UINT	Size24;	// 16M max
	struct{
		BYTE	__padding[3];
		BYTE	Type;
	};};
	BYTE		Data[PL_MAX];

	INLFUNC void SetSize(UINT size, UINT type)
	{	ASSERT(size <= PL_MAX);
		ASSERT(type <= 0xff);
		Size24 = size;
		Type = type;
	}
	INLFUNC UINT GetSize() const { return Size24&0xffffff; }
	template<typename T>
	T& Val(){ return *(T*)Data; }
};

template<UINT block_size>
struct CQ_Traits: public moodycamel::ConcurrentQueueDefaultTraits
{	static const size_t BLOCK_SIZE = block_size;
};

template<typename T, bool blocking, bool singleReaderWriter, UINT block_size = 32>
struct _ConcurrentQueue;
	template<typename T, UINT block_size> 
	struct _ConcurrentQueue<T, true, false, block_size>
		:public moodycamel::BlockingConcurrentQueue<T, CQ_Traits<block_size>>
    {   typedef moodycamel::BlockingConcurrentQueue<T, CQ_Traits<block_size>> _SC;
        INLFUNC _ConcurrentQueue(UINT reserved_size)
            :moodycamel::BlockingConcurrentQueue<T, CQ_Traits<block_size>>(reserved_size){ ASSERT(_SC::is_lock_free()); }
		INLFUNC bool Pop(T& t, UINT timeout = 0)
        {	return timeout?_SC::wait_dequeue_timed(t, timeout*1000LL):_SC::try_dequeue(t);
		}
	};
	template<typename T, UINT block_size>
	struct _ConcurrentQueue<T, false, false, block_size>
		:public moodycamel::ConcurrentQueue<T, CQ_Traits<block_size>>
    {   typedef moodycamel::ConcurrentQueue<T, CQ_Traits<block_size>> _SC;
        INLFUNC _ConcurrentQueue(UINT reserved_size)
            :moodycamel::ConcurrentQueue<T, CQ_Traits<block_size>>(reserved_size){ ASSERT(_SC::is_lock_free()); }
		INLFUNC bool Pop(T& t, UINT timeout = 0)
		{	ASSERT(timeout==0);
            return _SC::try_dequeue(t);
		}
	};
	template<typename T, UINT block_size>
	struct _ConcurrentQueue<T, true, true, block_size>
		:public moodycamel::BlockingReaderWriterQueue<T, block_size>
    {   typedef moodycamel::BlockingReaderWriterQueue<T, block_size> _SC;
        INLFUNC _ConcurrentQueue(UINT reserved_size):moodycamel::BlockingReaderWriterQueue<T, block_size>(reserved_size){}
		INLFUNC bool Pop(T& t, UINT timeout = 0)
        {	return timeout?_SC::wait_dequeue_timed(t, timeout*1000LL): _SC::try_dequeue(t);
		}
	};
	template<typename T, UINT block_size>
	struct _ConcurrentQueue<T, false, true, block_size>
		:public moodycamel::ReaderWriterQueue<T, block_size>
	{	INLFUNC _ConcurrentQueue(UINT reserved_size):moodycamel::ReaderWriterQueue<T, block_size>(reserved_size){}
		INLFUNC bool Pop(T& t, UINT timeout = 0)
		{	ASSERT(timeout==0);
			return try_dequeue(t);
		}
	};
} // _details

template<typename T, UINT BLOCK_SIZE, bool BLOCKING = true, bool SINGLE_READER_WRITER = false>
// the reserved size in the ctor will be round to multiple of BLOCK_SIZE
class AsyncDataQueueInfinite
{
protected:
	_details::_ConcurrentQueue<T, BLOCKING, SINGLE_READER_WRITER, BLOCK_SIZE>	_Q;

public:
	AsyncDataQueueInfinite(INT ReservedSize = 32):_Q(ReservedSize){}
	INLFUNC void Push(const T& t){ VERIFY(_Q.enqueue(t)); }
	INLFUNC bool Pop(T& t, UINT timeout = 0){ return _Q.Pop(t,timeout); }
};

template<typename T, UINT BLOCK_SIZE, bool BLOCKING = true, bool SINGLE_READER_WRITER = false>
// the reserved size in the ctor will be round to multiple of BLOCK_SIZE
class AsyncDataQueue: public AsyncDataQueueInfinite<T, BLOCK_SIZE, BLOCKING, SINGLE_READER_WRITER>
{
	typedef AsyncDataQueueInfinite<T, BLOCK_SIZE, BLOCKING, SINGLE_READER_WRITER> _SC;
	volatile INT	__Size;
	INT				__SizeMax;
public:
	AsyncDataQueue(INT MaxSize = 0x7fffffff, INT ReservedSize = 32):_SC(ReservedSize){ __Size = 0; __SizeMax = MaxSize; }
	INLFUNC bool Push(const T& t, bool force_grow = false)
	{	if(force_grow || __Size<__SizeMax)
		{	
			os::AtomicIncrement(&__Size); 
			VERIFY(_SC::_Q.enqueue(t));
			return true; 
		}
		else return false;
	}
	INLFUNC bool Pop(T& t, UINT timeout = 0)
	{	if(_SC::_Q.Pop(t,timeout))
		{	os::AtomicDecrement(&__Size);
			return true;
		}
		else return false;
	}
	INLFUNC INT GetSize() const { return __Size; }
};

} // ext
