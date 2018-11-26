#pragma once

//////////////////////////////////////////////////////////////////////
// Cross-Platform Core Foundation (CPCF)
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of CPF.  nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//////////////////////////////////////////////////////////////////////

#include "runtime_base.h"
#include "type_traits.h"
#include "string_type.h"


namespace rt
{

namespace _details
{
	template<bool is_not_pod, typename t_Val>
	struct _xtor
	{	static FORCEINL void ctor(t_Val*p){ new (p) t_Val(); }
		static FORCEINL void ctor(t_Val*p, t_Val*end){ for(;p<end;p++) new (p) t_Val(); }
		static FORCEINL void ctor(t_Val*p, const t_Val& x){ new (p) t_Val(x); }
		static FORCEINL void dtor(t_Val& x){ x.~t_Val(); }
		static FORCEINL void dtor(t_Val*p, t_Val*end){ for(;p<end;p++) p->~t_Val(); }
	};
	template<typename t_Val>
	struct _xtor<false, t_Val>
	{	static FORCEINL void ctor(t_Val*p){}
		static FORCEINL void ctor(t_Val*p, t_Val*end){}
		static FORCEINL void ctor(t_Val*p, const t_Val& x){ rt::CopyByteTo(x, *p); }
		static FORCEINL void dtor(t_Val& x){}
		static FORCEINL void dtor(t_Val*p, t_Val*end){}
	};
};

template<typename t_Val, typename t_Index = SIZE_T>
class Buffer_Ref
{
	typedef int (* _PtFuncCompare)(const void *, const void *);
	typedef typename rt::TypeTraits<t_Index>::t_Signed	t_SignedIndex;
protected:
	t_Val*	_p;
	t_Index	_len;
public:
	typedef t_Val						ItemType;
	typedef ItemType*					LPItemType;
	typedef const ItemType*				LPCItemType;

	INLFUNC LPCItemType Begin()const { return &_p[0]; }
	INLFUNC LPItemType  Begin(){ return &_p[0]; }
	INLFUNC LPCItemType End()const { return &_p[GetSize()]; }
	INLFUNC LPItemType  End(){ return &_p[GetSize()]; }

	INLFUNC t_Val&			First(){ return _p[0]; }
	INLFUNC const t_Val&	First() const { return _p[0]; }
	INLFUNC t_Val&			Last(){ return _p[_len-1]; }
	INLFUNC const t_Val&	Last() const { return _p[_len-1]; }

	INLFUNC operator LPItemType(){ return &_p[0]; }
	INLFUNC operator LPCItemType()const { return &_p[0]; }
	INLFUNC operator LPVOID(){ return &_p[0]; }
	INLFUNC operator LPCVOID()const { return &_p[0]; }

	t_Index GetSize() const { return _len; }

public:
	INLFUNC Buffer_Ref(){ _p=NULL; _len=0; }
	INLFUNC Buffer_Ref(const Buffer_Ref<t_Val>& x){ _p = x._p; _len = x._len; }
	INLFUNC Buffer_Ref(const t_Val*	p, t_Index len){ _p = (t_Val*)p; _len = len; }
	INLFUNC Buffer_Ref GetSub(t_Index start, t_Index size){ return Buffer_Ref(_p+start, size); }
	template<typename T> INLFUNC void Set(const T& x)
	{	t_Val*	end = _p + _len;	
		for(t_Val* p = _p; p < end; p++)*p = x;
	}
	template<typename T>
	t_Val& operator [](T i){ ASSERT(i>=0 && i<=(T)_len); return _p[i]; }
	template<typename T>
	const t_Val& operator [](T i) const { ASSERT(i>=0 && i<=(T)_len); return _p[i]; }
	template<typename T> INLFUNC bool operator ==(const Buffer_Ref<T>& x) const
	{	if(x.GetSize() != GetSize())return false;
		for(t_Index i=0;i<GetSize();i++)
		{	if(_p[i] == x[i]){}
			else return false;
		}
		return true;
	}
	template<typename T> INLFUNC bool operator !=(const Buffer_Ref<T>& x) const
	{	if(x.GetSize() != GetSize())return true;
		for(t_Index i=0;i<GetSize();i++)
			if(_p[i] != x[i])return true;
		return false;
	}
	template<typename T> INLFUNC t_SignedIndex SearchItem(const T& x) const
	{	for(t_Index i=0;i<GetSize();i++)
			if(_p[i] == x)return i;
		return -1;
	}
	INLFUNC t_SignedIndex SearchSortedItem(const t_Val& x) const // binary search
	{	return std::find(Begin(), End(), x) - Begin();
	}
	INLFUNC t_SignedIndex SearchLowerbound(const t_Val& x) const // binary search
	{	return std::lower_bound(Begin(), End(), x) - Begin();
	}
	INLFUNC void Zero(){ memset((LPVOID)_p, 0, _len*sizeof(t_Val)); }

	template<typename T>
	INLFUNC void CopyFrom(const Buffer_Ref<T>& x)
	{	ASSERT(GetSize() == x.GetSize());
		for(t_Index i=0;i<GetSize();i++)
			_p[i] = x[i];
	}
	template<typename T>
	INLFUNC void CopyFrom(const T* x)
	{	for(t_Index i=0;i<GetSize();i++)
			_p[i] = x[i];
	}
	template<typename T>
	INLFUNC void CopyTo(T* x) const
	{	for(t_Index i=0;i<GetSize();i++)
			x[i] = _p[i];
	}
	INLFUNC void Sort()
	{	struct _comp
		{	static int compare(const t_Val * a, const t_Val * b)
			{	if(*a < *b)return -1;
				if(*b < *a)return 1;
				return 0;
		}	};
		Sort<_comp>();
	}
	INLFUNC void SortDesc()
	{	struct _comp
		{	static int compare(const t_Val * a, const t_Val * b)
			{	if(*a < *b)return 1;
				if(*b < *a)return -1;
				return 0;
		}	};
		Sort<_comp>();
	}
	template<class T>
	INLFUNC void Sort()
	{
		qsort(Begin(), GetSize(), sizeof(t_Val), (_PtFuncCompare)T::compare);
	}
	INLFUNC void Shuffle(DWORD seed)
	{	if(GetSize())
		{	Randomizer rng(seed);	
			for(t_Index i=0;i<GetSize()-1;i++)
				rt::Swap((*this)[i], (*this)[i+ rng%(GetSize()-i)]);
		}
	}
	INLFUNC void RandomBits(DWORD seed = rand())
	{	Randomizer rng(seed);
		t_Index int_size = GetSize()*sizeof(t_Val)/4;
		t_Index i=0;
		for(; i<int_size; i++)
			((DWORD*)_p)[i] = rng;
		i*=4;
		if(GetSize()*sizeof(t_Val) > i)
		{	
			DWORD last = rng;
			int b=0;
			for(;i<GetSize()*sizeof(t_Val);i++, b++)
				((BYTE*)_p)[i] = ((BYTE*)&last)[b];
		}
	}
	template<typename visitor>
	INLFUNC SIZE_T ForEach(const visitor& v)  // v(obj, idx, tot)
	{	SIZE_T i=0;
		while(i<GetSize())
		{	if(!rt::_details::_CallLambda<bool, decltype(v(_p[i],i,GetSize()))>(true, v, _p[i],i,GetSize()).retval)
				return i+1;
			i++;
		}
		return i;
	}
	template<typename visitor>
	INLFUNC SIZE_T ForEach(const visitor& v) const  // v(const obj, idx, tot)
	{	SIZE_T i=0;
		while(i<GetSize())
		{	if(!rt::_details::_CallLambda<bool, decltype(v(_p[i],i,GetSize()))>(true, v, _p[i],i,GetSize()).retval)
				return i+1;
			i++;
		}
		return i;
	}
	template<typename T>
	INLFUNC SSIZE_T Find(const T& v) const
	{	for(SIZE_T i=0; i<GetSize(); i++)
			if(_p[i] == v)return i;
		return -1;
	}
};


template<typename t_Val>
class Buffer:public Buffer_Ref<t_Val>
{	typedef Buffer_Ref<t_Val> _SC;
protected:
	static const bool IsElementNotPOD = !rt::TypeTraits<t_Val>::IsPOD;
	typedef _details::_xtor<!rt::TypeTraits<t_Val>::IsPOD, t_Val>	_xt;
	INLFUNC void __SafeFree()
	{	if(!_SC::_p)return;
		_xt::dtor(_SC::_p, _SC::_p+_SC::_len);
		_SafeFree32AL(((LPVOID&)_SC::_p));
	}
public:
	// allow for(iterator : Buffer) syntax (C++ 11)
	INLFUNC t_Val*			begin(){ return _p; }
	INLFUNC const t_Val*	begin() const { return _p; }
	INLFUNC t_Val*			end(){ return _p + GetSize(); }
	INLFUNC const t_Val*	end() const { return _p + GetSize(); }

	INLFUNC Buffer(){}
	INLFUNC Buffer(const t_Val* p, SIZE_T len){ *this = Buffer_Ref<t_Val>(p,len); }
	INLFUNC explicit Buffer(const Buffer_Ref<t_Val> &x){ *this=x; }	//copy ctor should be avoided, use reference for function parameters
	INLFUNC explicit Buffer(const Buffer<t_Val> &x){ *this=x; }	//copy ctor should be avoided, use reference for function parameters
	INLFUNC const Buffer_Ref<t_Val>& operator = (const Buffer<t_Val> &x){ *this = (Buffer_Ref<t_Val>&)x; return *this; }
	INLFUNC const Buffer_Ref<t_Val>& operator = (const Buffer_Ref<t_Val> &x)
    {	for(SIZE_T i=0;i<_SC::_len;i++)
			_xt::dtor(_SC::_p[i]);
        if(_SC::_len >= x.GetSize()){ _SC::_len = x.GetSize(); }
		else
		{	_SafeFree32AL(((LPVOID&)_SC::_p));
            _SC::_p = _Malloc32AL(t_Val, _SC::_len = x.GetSize());
			ASSERT(_SC::_p);
		}
		for(SIZE_T i=0;i<_SC::_len;i++)
			_xt::ctor(&_SC::_p[i], x[i]);
		return x;
	}
	INLFUNC ~Buffer(){ __SafeFree(); }
	INLFUNC bool SetSize(SIZE_T co=0) //zero for clear
	{	
		if(co == _SC::_len){ return true; }
		else
		{	__SafeFree();
			_SC::_len = co;
			if(co)
			{	_SC::_p = _Malloc32AL(t_Val,co);
				if( _SC::_p )
				{	
					_xt::ctor(_SC::_p, _SC::_p+_SC::_len);
				}
				else
				{	_SC::_len = 0; return false; }
			}
		}
		return true;
	}
	INLFUNC bool ChangeSize(SIZE_T new_size) //Original data at front is preserved
	{	
		if( new_size == _SC::_len )return true;
		if( new_size<_SC::_len )
		{	
			for(SIZE_T i=new_size;i<_SC::_len;i++)_xt::dtor(_SC::_p[i]);	//call dtor for unwanted instances at back
			_SC::_len = new_size;
			return true;
		}
		else	//expand buffer
		{	t_Val* new_p = _Malloc32AL(t_Val,new_size);
			if(new_p)
			{	memcpy(new_p,_SC::_p,_SC::_len*sizeof(t_Val));
				_SafeFree32AL(_SC::_p);
				_SC::_p = new_p;
			
				for(SIZE_T i=_SC::_len;i<new_size;i++)_xt::ctor(&_SC::_p[i]); //call ctor for additional instances at back
				_SC::_len = new_size;
				return true;
			}
		}
		
		return false;
	}
	INLFUNC t_Val* Detach(){ auto* p = _SC::_p; _SC::_p = NULL; _SC::_len = 0; return p; }
};

template<typename t_Val>
class BufferEx: public Buffer<t_Val>
{
	typedef Buffer<t_Val> _SC;
	INLFUNC bool _add_entry(SIZE_T co = 1) // the added entry's ctor is not called !!
	{	if(Buffer<t_Val>::_len+co <= _len_reserved){} // expand elements only
		else // expand buffer
		{	SIZE_T new_buf_resv = rt::max(rt::max((SIZE_T)4,Buffer<t_Val>::_len+co),_len_reserved*2);
			t_Val* pNewBuffer = _Malloc32AL(t_Val,new_buf_resv);
			if(pNewBuffer){}else
			{	new_buf_resv = Buffer<t_Val>::_len+co;
				pNewBuffer = (t_Val*)_Malloc32AL(t_Val,new_buf_resv);
				if(pNewBuffer){}else{ return false; }
			}
			// copy old elements
			memcpy(pNewBuffer,Buffer<t_Val>::_p,sizeof(t_Val)*Buffer<t_Val>::_len);
			_SafeFree32AL(Buffer<t_Val>::_p);
			Buffer<t_Val>::_p = (t_Val*)pNewBuffer;
			_len_reserved = new_buf_resv;
		}
		Buffer<t_Val>::_len+=co;
		return true;
	}
protected:
	SIZE_T	_len_reserved;
public:
	INLFUNC BufferEx(const BufferEx &x){ _len_reserved = 0; *this = x; }
	INLFUNC const BufferEx& operator = (const BufferEx &x)
	{	ChangeSize(0);
		_SC::_len = x.GetSize();
		if(_SC::_len <= _len_reserved){}
		else
		{	_SafeFree32AL(Buffer<t_Val>::_p);
			Buffer<t_Val>::_p = _Malloc32AL(t_Val,_SC::_len);
			ASSERT(Buffer<t_Val>::_p);
			_len_reserved = _SC::_len;
		}
		for(SIZE_T i=0; i<_SC::_len; i++)
			Buffer<t_Val>::_xt::ctor(&_SC::_p[i], x[i]);
		return x;
	}
	INLFUNC BufferEx(){ _len_reserved=0; }
	INLFUNC bool SetSize(SIZE_T co=0) //zero for clear
	{	if(Buffer<t_Val>::SetSize(co)){ _len_reserved=Buffer<t_Val>::_len; return true; }
		else{ _len_reserved=0; return false; }
	}
	INLFUNC SIZE_T GetSize() const { return Buffer<t_Val>::GetSize(); } // make Visual Studio happy
	INLFUNC bool Clear(){ return SetSize(0); }
	INLFUNC bool ChangeSize(SIZE_T new_size) // Original data at front is preserved
	{	if( new_size == Buffer<t_Val>::_len )return true;
		if( new_size<Buffer<t_Val>::_len ){ return Buffer<t_Val>::ChangeSize(new_size); }
		else 
		{	if(new_size <= _len_reserved){} // expand elements only
			else // expand buffer
			{	_len_reserved = rt::max(rt::max((SIZE_T)16,new_size),_len_reserved*2);
				LPBYTE pNewBuffer = (LPBYTE)_Malloc32AL(t_Val,_len_reserved);
				if(pNewBuffer){}else
				{	_len_reserved = new_size;
					pNewBuffer = (LPBYTE)_Malloc32AL(t_Val,_len_reserved);
					if(pNewBuffer){}else{ return false; }
				}
				// copy old elements
				memcpy(pNewBuffer,Buffer<t_Val>::_p,sizeof(t_Val)*Buffer<t_Val>::_len);
				_SafeFree32AL(Buffer<t_Val>::_p);
				Buffer<t_Val>::_p = (t_Val*)pNewBuffer;
			}
			for(SIZE_T i=Buffer<t_Val>::_len;i<new_size;i++)Buffer<t_Val>::_xt::ctor(&Buffer<t_Val>::_p[i]); // call ctor for additional instances at back
			Buffer<t_Val>::_len = new_size;
			return true;
		}
	}
	INLFUNC SIZE_T GetReservedSize() const { return _len_reserved; }
	INLFUNC t_Val& push_front()
	{	VERIFY(_add_entry());
		memmove(&Buffer<t_Val>::_p[1],&Buffer<t_Val>::_p[0],sizeof(t_Val)*(Buffer<t_Val>::_len-1));
		Buffer<t_Val>::_xt::ctor(&Buffer<t_Val>::_p[0]);
		return Buffer<t_Val>::_p[0];
	}
	template<typename T>
	INLFUNC t_Val& push_front(const T& x)
	{	VERIFY(_add_entry());
		memmove(&Buffer<t_Val>::_p[1],&Buffer<t_Val>::_p[0],sizeof(t_Val)*(Buffer<t_Val>::_len-1));
		Buffer<t_Val>::_xt::ctor(&Buffer<t_Val>::_p[0],x);
		return Buffer<t_Val>::_p[0];
	}
	INLFUNC t_Val& push_back()
	{	VERIFY(_add_entry());
		Buffer<t_Val>::_xt::ctor(&Buffer<t_Val>::_p[Buffer<t_Val>::_len-1]);
		return Buffer<t_Val>::_p[Buffer<t_Val>::_len-1];
	}
	INLFUNC t_Val& push_back(const t_Val& x)
	{	VERIFY(_add_entry());
		Buffer<t_Val>::_xt::ctor(&Buffer<t_Val>::_p[Buffer<t_Val>::_len-1],x);
		return Buffer<t_Val>::_p[Buffer<t_Val>::_len-1];
	}
	template<typename T>
	INLFUNC void push_back(const T* x, SIZE_T count)
	{	
		SIZE_T sz = _SC::GetSize();
		VERIFY(_add_entry(count));
		for(;sz<Buffer<t_Val>::_len;sz++){ Buffer<t_Val>::_xt::ctor(&Buffer<t_Val>::_p[sz], *x++); }
	}
	INLFUNC t_Val* push_back_n(SIZE_T count)
	{	
		SIZE_T sz = _SC::GetSize();
		return ChangeSize(sz + count)?&Buffer<t_Val>::_p[sz]:NULL;
	}
	INLFUNC void erase(const t_Val* p)
	{	ASSERT(p < _SC::End());
		ASSERT(p >= _SC::Begin());
		erase(p - _SC::Begin());
	}
	INLFUNC void erase(const t_Val* begin, const t_Val* end) // *end will not be erased
	{	erase(begin - _SC::Begin(), end - _SC::Begin());
	}
	INLFUNC void erase(SIZE_T index)
	{	ASSERT(index<Buffer<t_Val>::_len);
		// call dtor for removed items
		Buffer<t_Val>::_xt::dtor(Buffer<t_Val>::_p[index]);
		Buffer<t_Val>::_len--;
		memmove(&Buffer<t_Val>::_p[index],&Buffer<t_Val>::_p[index+1],sizeof(t_Val)*(Buffer<t_Val>::_len-index));
	}
	INLFUNC void erase(SIZE_T index_begin, SIZE_T index_end)
	{	ASSERT(index_begin<=index_end);
		ASSERT(index_end<Buffer<t_Val>::_len);
		index_end++;
		// call dtor for removed items
		for(SIZE_T i=index_begin;i<index_end;i++)Buffer<t_Val>::_xt::dtor(Buffer<t_Val>::_p[i]);
		memmove(&Buffer<t_Val>::_p[index_begin],&Buffer<t_Val>::_p[index_end],sizeof(t_Val)*(Buffer<t_Val>::_len-index_end));
		Buffer<t_Val>::_len-=(index_end-index_begin);
	}
	INLFUNC void pop_back()
	{	ASSERT(Buffer<t_Val>::_len); 	Buffer<t_Val>::_len--;
		Buffer<t_Val>::_xt::dtor(Buffer<t_Val>::_p[Buffer<t_Val>::_len]);
	}
	INLFUNC void pop_front()
	{	ASSERT(Buffer<t_Val>::_len); 	Buffer<t_Val>::_len--;
		Buffer<t_Val>::_xt::dtor(Buffer<t_Val>::_p[0]);
		memmove(&Buffer<t_Val>::_p[0],&Buffer<t_Val>::_p[1],sizeof(t_Val)*Buffer<t_Val>::_len);
	}
	INLFUNC void compact_memory()
	{	if(Buffer<t_Val>::_len < _len_reserved)
		{	LPBYTE pNew = (LPBYTE)_Malloc32AL(t_Val,Buffer<t_Val>::_len);
			if(pNew)
			{	memcpy(pNew,Buffer<t_Val>::_p,sizeof(t_Val)*Buffer<t_Val>::_len);
				_SafeFree32AL(Buffer<t_Val>::_p);
				Buffer<t_Val>::_p = pNew;
				_len_reserved = Buffer<t_Val>::_len;
			}
		}
	};
	INLFUNC bool reserve(SIZE_T co)
	{	if(co>_len_reserved)
		{	LPBYTE pNew = (LPBYTE)_Malloc32AL(t_Val,co);
			if(pNew)
			{	memcpy(pNew,Buffer<t_Val>::_p,sizeof(t_Val)*Buffer<t_Val>::_len);
				_SafeFree32AL(Buffer<t_Val>::_p);
				Buffer<t_Val>::_p = (t_Val*)pNew;
				_len_reserved = co;
			}
			return (bool)(pNew!=NULL);
		}
		return true;
	}

	INLFUNC t_Val& first(){ ASSERT(Buffer<t_Val>::_len); return Buffer<t_Val>::_p[0]; }
	INLFUNC const t_Val& first()const{  ASSERT(Buffer<t_Val>::_len); return Buffer<t_Val>::_p[0]; }
	INLFUNC t_Val& last(){  ASSERT(Buffer<t_Val>::_len); return Buffer<t_Val>::_p[Buffer<t_Val>::_len-1]; }
	INLFUNC const t_Val& last()const{  ASSERT(Buffer<t_Val>::_len); return Buffer<t_Val>::_p[Buffer<t_Val>::_len-1]; }

	INLFUNC t_Val& insert(SIZE_T index)
	{	VERIFY(_add_entry());
		if(index<Buffer<t_Val>::_len-1)	
		{	memmove(&Buffer<t_Val>::_p[index+1],&Buffer<t_Val>::_p[index],(Buffer<t_Val>::_len-index-1)*sizeof(t_Val));
			Buffer<t_Val>::_xt::ctor(&Buffer<t_Val>::_p[index]);
		}
		return Buffer<t_Val>::_p[index];
	}
	INLFUNC bool insert(SIZE_T index,SIZE_T count)
	{
		if(reserve(Buffer<t_Val>::GetSize() + count))
		{	memmove(&Buffer<t_Val>::_p[index+count], &Buffer<t_Val>::_p[index], (Buffer<t_Val>::GetSize() - index)*sizeof(t_Val));
			for(SIZE_T i = index;i<index+count;i++)Buffer<t_Val>::_xt::ctor(&Buffer<t_Val>::_p[i]);
			Buffer<t_Val>::_len += count;
			return true;
		}	
		return false;
	}
	INLFUNC bool insert(SIZE_T index,SIZE_T count,const t_Val& x)
	{
		if(reserve(Buffer<t_Val>::GetSize() + count))
		{	memmove(&Buffer<t_Val>::_p[index+count], &Buffer<t_Val>::_p[index], (Buffer<t_Val>::GetSize() - index)*sizeof(t_Val));
			for(SIZE_T i = index;i<index+count;i++)Buffer<t_Val>::_xt::ctor(&Buffer<t_Val>::_p[i], x);
			Buffer<t_Val>::_len += count;
			return true;
		}	
		return false;
	}
	INLFUNC SSIZE_T SortedPush(const t_Val& x)
	{	if(GetSize() == 0 || x<first()){ push_front(x); return GetSize()-1; }
		if(reserve(Buffer<t_Val>::GetSize() + 1))
		{
			Buffer<t_Val>::_len++;
			auto* p = &_SC::Last();
			for(p--;;p--)
			{	if(*p < x){ p[1] = *p; }
				else break;
			}
			_SC::_xt::ctor(p, x);
			return p - _SC::Begin();
		}else return -1;
	}
	INLFUNC bool Include(const t_Val& x) // newly included item is at the end
	{	if(_SC::Find(x) == -1){	push_back(x); return true; }
		return false;
	}
};

template<typename T>
class TypeTraits<Buffer<T>>
{
public:	
	typedef Buffer<T> t_Val;
	typedef T t_Element;
	typedef Buffer<T> t_Signed;
	typedef Buffer<T> t_Unsigned;
	static const int Typeid = _typeid_buffer;
	static const bool IsPOD = false;
	static const bool IsNumeric = false;
};

template<typename T>
class TypeTraits<BufferEx<T>>
{
public:	
	typedef BufferEx<T> t_Val;
	typedef T t_Element;
	typedef BufferEx<T> t_Signed;
	typedef BufferEx<T> t_Unsigned;
	static const int Typeid = _typeid_buffer;
	static const bool IsPOD = false;
	static const bool IsNumeric = false;
};

template<class t_Ostream, typename t_Ele>
t_Ostream& operator<<(t_Ostream& Ostream, const _details::_LOG_FULLDATA<rt::Buffer_Ref<t_Ele>> & vec)
{	Ostream<<'{';
	if(rt::TypeTraits<typename rt::TypeTraits<t_Ele>::t_Element>::Typeid == rt::_typeid_8s)
	{
		for(SIZE_T i=0;i<vec.GetSize();i++)
		{	if(i)
				Ostream<<','<<'"'<<vec[i]<<'"';
			else
				Ostream<<'"'<<vec[i]<<'"';
		}
	}
	else
	{
		for(SIZE_T i=0;i<vec.GetSize();i++)
		{	if(i)
				Ostream<<','<<vec[i];
			else
				Ostream<<vec[i];
		}
	}
	Ostream<<'}';
	return Ostream;
}

template<class t_Ostream, typename t_Ele>
t_Ostream& operator<<(t_Ostream& Ostream, const rt::Buffer_Ref<t_Ele> & vec)
{
	if(rt::TypeTraits<typename rt::TypeTraits<t_Ele>::t_Element>::Typeid == rt::_typeid_8s)
	{
		if(vec.GetSize()>5)
		{	
			Ostream<<'{';
			for(SIZE_T i=0;i<3;i++)
				Ostream<<'"'<<vec[i]<<'"'<<',';
			Ostream<<rt::SS(" ... ,\"");
			Ostream<<vec.End()[-1]<<"\"} // size="<<vec.GetSize();
			return Ostream;
		}
	}
	else
	{
		if(vec.GetSize()>10)
		{	
			Ostream<<'{';
			for(SIZE_T i=0;i<8;i++)
				Ostream<<vec[i]<<',';
			Ostream<<rt::SS(" ... ,");
			Ostream<<vec.End()[-1]<<"} // size="<<vec.GetSize();
			return Ostream;
		}
	}

	return Ostream << rt::LOG_FULLDATA(vec);
}


} // namespace rt



namespace rt
{
namespace _details
{
template<class t_Storage>
class StreamT: protected t_Storage
{	typedef t_Storage _SC;
protected:
	UINT	m_Pos;
	UINT	m_Used;
public:
	INLFUNC StreamT(){ m_Pos = 0; m_Used = 0; }
	INLFUNC StreamT(LPCBYTE p, UINT len, UINT used_len):t_Storage(p,len){ m_Pos = 0; m_Used = used_len; }
	INLFUNC SIZE_T GetLength() const { return m_Used; }
	INLFUNC LPBYTE GetInternalBuffer(){ return (LPBYTE)_SC::_p; }
	INLFUNC LPCBYTE GetInternalBuffer() const { return (LPCBYTE)_SC::_p; }
	INLFUNC void Rewind(){ m_Pos = 0; }
	INLFUNC LONGLONG Seek(SSIZE_T offset, int nFrom = rt::_File::Seek_Begin)
	{	SSIZE_T newp = 0;
		switch(nFrom)
		{	case rt::_File::Seek_Begin:	newp = offset; break;
			case rt::_File::Seek_Current: newp = m_Pos + offset; break;
			case rt::_File::Seek_End:		newp = m_Used + offset; break;
			default: ASSERT(0);
		}
		if(newp >=0 && newp <= (SSIZE_T)_SC::_len)m_Pos = (UINT)newp;
		return m_Pos;
	}
};

template<class t_Storage>
class OStreamT: public StreamT<t_Storage>
{	typedef StreamT<t_Storage> _SC;
public:
	INLFUNC OStreamT(){}
	INLFUNC OStreamT(LPCBYTE p, UINT len):StreamT<t_Storage>(p,len,0){}
	INLFUNC UINT Write(LPCVOID pBuf, UINT co)
	{	co = rt::min((UINT)(_SC::_len - _SC::m_Pos), co);
		if(co)
		{	memcpy(_SC::_p + _SC::m_Pos,pBuf,co);
			_SC::m_Pos += co;
			if(_SC::m_Used < _SC::m_Pos)_SC::m_Used = _SC::m_Pos;
		}
		return co;
	}
};

template<class t_Storage>
class IStreamT: public StreamT<t_Storage>
{	typedef StreamT<t_Storage> _SC;
public:
	INLFUNC IStreamT(LPCBYTE p, UINT len):StreamT<t_Storage>(p,len,len){}
	INLFUNC UINT Read(LPVOID pBuf, UINT co)
	{	ASSERT(_SC::m_Pos <= _SC::_len);
		co = rt::min(co, (UINT)(_SC::_len - _SC::m_Pos));
		memcpy(pBuf,_SC::_p + _SC::m_Pos,co);
		_SC::m_Pos += co;
		return co;
	}
};

} // namespace _details

class OStream: public _details::OStreamT< Buffer<BYTE> >
{	
public:
	INLFUNC UINT Write(LPCVOID pBuf, UINT co)
	{	if(this->ChangeSize(rt::max(_len,(SIZE_T)(m_Pos+co))))
		{	memcpy(_p + m_Pos,pBuf,co);
			m_Pos += co;
			return co;
		}
		else return 0;
	}
	INLFUNC LONGLONG Seek(SSIZE_T offset, int nFrom = rt::_File::Seek_Begin)
	{	SSIZE_T newp = 0;
		switch(nFrom)
		{	case rt::_File::Seek_Begin:	newp = offset; break;
			case rt::_File::Seek_Current: newp = m_Pos+offset; break;
			case rt::_File::Seek_End:		newp = _len + offset; break;
			default: ASSERT(0);
		}
		if(newp >=0 && this->ChangeSize(rt::max(_len,(SIZE_T)newp)))
			m_Pos = (UINT)newp;
		return m_Pos;
	}
	INLFUNC bool SetLength(UINT sz){ m_Pos = rt::min(m_Pos,sz); m_Used = sz; return ChangeSize((UINT)sz); }
};

typedef _details::OStreamT<Buffer_Ref<BYTE> > OStream_Ref;
typedef _details::IStreamT<Buffer_Ref<BYTE> > IStream_Ref;

template<UINT LEN>
class OStreamFixed: public OStream_Ref
{
	BYTE	_Buffer[LEN];
public:
	OStreamFixed():OStream_Ref(_Buffer,LEN){}
	INLFUNC bool SetLength(UINT sz){ if(sz <= LEN){ m_Pos = rt::min(m_Pos,sz); m_Used = sz; return true; } return false; }
};

class CircularBuffer // not thread-safe
{
protected:
#pragma pack(1)
	struct _BlockHeader
	{	SIZE_T	Next;
		SIZE_T	PayloadSize;
		SIZE_T	PayloadLength;
	};
	struct _Block: public _BlockHeader
	{	BYTE	Payload[1];
	};
public:
	struct Block
	{	SIZE_T	Length;
		BYTE	Data[1];
	};
#pragma pack()
protected:
	LPBYTE			_Buffer;
	SIZE_T			_BufferSize;
	SIZE_T			Back;
	SIZE_T			NewBlock;
	_Block*			pLastBlock;	
public:
	CircularBuffer()
	{	_Buffer = NULL;
		SetSize(0);
	}
	~CircularBuffer(){ _SafeFree32AL(_Buffer); }
	void   SetSize(SIZE_T sz)
	{	_SafeFree32AL(_Buffer);
		if(sz)
		{	_Buffer = (LPBYTE)_Malloc32AL(BYTE,sz);
			_BufferSize = sz;
			ASSERT(_Buffer);
		}
		else
		{	_Buffer = NULL;
			_BufferSize = 0;
		}
		Back = NewBlock = 0;
		pLastBlock = NULL;
	}
	LPBYTE Push(SIZE_T size_max)
	{	size_max = rt::EnlargeTo32AL(size_max);
		SIZE_T	block_size = size_max + sizeof(_BlockHeader);
		_Block*	p;
		if(	(block_size + NewBlock < _BufferSize && NewBlock >= Back) ||
			(block_size + NewBlock < Back && NewBlock < Back)
		)
		{	p = (_Block*)(_Buffer + NewBlock);
			NewBlock += block_size;
		}
		else if(NewBlock >= Back && Back > block_size)
		{	p = (_Block*)_Buffer;
			NewBlock = block_size;
		}
		else return NULL;

		p->PayloadSize = size_max;
		p->Next = INFINITE;
		p->PayloadLength = 0;
		
		if(pLastBlock)pLastBlock->Next = ((LPBYTE)p) - _Buffer;
		pLastBlock = p;
		return p->Payload;
	}
	void Pop()
	{
		if(Back != NewBlock)
		{
			_Block* p = (_Block*)(_Buffer + Back);
			if(p == pLastBlock)
			{	ASSERT(p->Next == INFINITE);
				pLastBlock = NULL;
				NewBlock = Back = 0;
			}
			else
			{	ASSERT(p->Next != INFINITE);
				Back = p->Next;
			}
		}
	}
	void SetBlockSize(LPCVOID pushed, SIZE_T finalized)	// call after Push()
	{
		ASSERT(finalized);
		_Block* block = (_Block*)(((LPBYTE)pushed) - sizeof(_BlockHeader));
		ASSERT(block->PayloadSize >= finalized);
		block->PayloadLength = finalized;
	}
	const Block* Peek() const
	{
		Block* p = (Back != NewBlock)?(Block*)(_Buffer + Back + sizeof(_BlockHeader) - sizeof(SIZE_T)):NULL;
		if(p && p->Length>0)
		{	return p;
		}
		else
			return NULL;
	}
};


template<typename T, bool ASC = false>
class SortedArray // not thread-safe
{
	rt::BufferEx<T>	_q;
	SIZE_T			_used;
public:
	void Clear(){ _used = 0; }
	SortedArray(SIZE_T capacity = 0){ Clear(); SetCapacity(capacity); }
	bool		SetCapacity(SIZE_T sz){ Clear(); return _q.ChangeSize(sz); }
	SIZE_T		GetSize() const { return _used; }
	bool		WillBeDropped(const T& x) const 
				{	if(_used < _q.GetSize())return false;
					if((ASC && x >= _q.last()) || (!ASC && x <= _q.last()))return true;
					return false;
				}
	INLFUNC bool Add(const T&x)
	{
		if(ASC)
		{
			if(_used==0 || _q[_used-1]<x)
			{	if(_used == _q.GetSize())
				{	return false;
				}
				else
				{	_q[_used++] = x;
					return true;
				}
			}
			if(_used<_q.GetSize())_used++;
			for(SIZE_T i = _used-1; i>0; i--)
			{
				if(x<_q[i-1])
				{	_q[i] = _q[i-1];
				}
				else
				{	_q[i] = x;
					return true;
				}
			}
		}
		else
		{
			if(_used==0 || x<_q[_used-1])
			{	if(_used == _q.GetSize())
				{	return false;
				}
				else
				{	_q[_used++] = x;
					return true;
				}
			}
			if(_used<_q.GetSize())_used++;
			for(SIZE_T i = _used-1; i>0; i--)
			{
				if(_q[i-1]<x)
				{	_q[i] = _q[i-1];
				}
				else
				{	_q[i] = x;
					return true;
				}
			}
		}

		_q[0] = x;
		return true;
	}
	const T& operator[](SIZE_T idx) const { return _q[idx]; }
	T& operator[](SIZE_T idx) { return _q[idx]; } // don't modify members while sorting is in-progress
	template<typename t_Accum>
	t_Accum GetAverage() const
	{	t_Accum ret = 0;
		for(UINT i=0;i<_used;i++)
			ret += _q[i];
		return ret/_used;
	}
};


template<typename T, UINT TOP_K=1, bool store_last_val = false>
class TopFrequentValues
{
	template<typename _T, UINT _TOP_K, bool s> friend class TopFrequentValues;
	struct _val
	{	T	Val;
		int	Count;
	};
public:
	static const int UNMATCHED = 0;
	static const int MATCHED = 1;
	static const int MATCHED_WITH_TOP = 2;
protected:
	_val	_TopValues[TOP_K];
	FORCEINL int _Match(const T& val)	// 0: not match, 1: matched and no promote, 2: matched
	{
		if(_TopValues[0].Val == val)
		{	_TopValues[0].Count++;
			if(store_last_val)_TopValues[0].Val = val;
			return MATCHED_WITH_TOP;
		}
		if(_TopValues[0].Count == 0)
		{	_TopValues[0].Count = 1;
			_TopValues[0].Val = val;
			return MATCHED_WITH_TOP;
		}
		int ret = ((TopFrequentValues<T,TOP_K-1>*)&_TopValues[1])->_Match(val);
		if(ret == MATCHED_WITH_TOP)
		{	if(_TopValues[1].Count > _TopValues[0].Count)
			{	rt::Swap(_TopValues[1], _TopValues[0]);
				return MATCHED_WITH_TOP;
			}else return MATCHED;
		}
		return ret;
	}
public:
	static FORCEINL UINT GetSize(){ return TOP_K; }
	FORCEINL TopFrequentValues(){ Reset(); }
	FORCEINL void Reset(){ rt::Zero(*this); }
	FORCEINL int Sample(const T& val)		// UNMATCHED / MATCHED / MATCH_WITH_TOP, 0: no match, 1: matched but not the top one no promote, 2: matched with top one
	{	int ret = _Match(val);
		if(ret == UNMATCHED)
		{	if((--_TopValues[TOP_K-1].Count) < 0)
			{	_TopValues[TOP_K-1].Val = val;
				_TopValues[TOP_K-1].Count = 1;
				return MATCHED;
			}
		}
		return ret;
	}
	FORCEINL bool		IsEmpty() const { return GetFrequency() <= 0; }
	FORCEINL const int	GetFrequency() const { return _TopValues[0].Count; }
	FORCEINL const int	GetFrequency(UINT i) const { return _TopValues[i].Count; }
	FORCEINL const T&	Get() const { return _TopValues[0].Val; }
	FORCEINL const T&	Get(UINT i) const { return _TopValues[i].Val; }
	FORCEINL const bool Get(UINT i, T* val, int* count) const
	{	if(_TopValues[i].Count>0)
		{	*count = _TopValues[i].Count;
			*val = _TopValues[i].Val;
			return true; 
		}else return false;
	}
	FORCEINL const T& operator[](UINT i) const { return Get(i); }
	FORCEINL void		Remove(UINT i)
	{	if(((int)TOP_K) - (int)i - 1 > 0)memmove(&_TopValues[i], &_TopValues[i+1], sizeof(_val)*(TOP_K - i - 1));
		_TopValues[TOP_K-1].Count = 0;
	}
};
	template<typename T, bool s>
	class TopFrequentValues<T,0,s>
	{	template<typename _T, UINT _TOP_K, bool _s> friend class TopFrequentValues;
		protected:	FORCEINL int  _Match(const T& val){ return 0; }
	};

template<class t_Ostream, typename t_Ele, int TOP, bool S>
t_Ostream& operator<<(t_Ostream& Ostream, const TopFrequentValues<t_Ele, TOP, S> & vec)
{	Ostream<<'{';
	if(rt::TypeTraits<typename rt::TypeTraits<t_Ele>::t_Element>::Typeid == rt::_typeid_8s)
	{
		for(UINT i=0;i<vec.GetSize();i++)
		{	if(i)
				Ostream<<','<<'"'<<vec[i]<<"\"="<<vec.GetFrequency(i);
			else
				Ostream<<'"'<<vec[i]<<"\"="<<vec.GetFrequency(i);
		}
	}
	else
	{
		for(UINT i=0;i<vec.GetSize();i++)
		{	if(i)
				Ostream<<','<<vec[i]<<'='<<vec.GetFrequency(i);
			else
				Ostream<<vec[i]<<'='<<vec.GetFrequency(i);
		}
	}
	Ostream<<'}';
	return Ostream;
}

} // namespace rt


