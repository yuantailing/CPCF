#pragma once

#include "../../os/kernel.h"
#include "rocksdb_conf.h"

#include "./include/db.h"
#include "./include/slice_transform.h"
#include "./include/merge_operator.h"

namespace rocksdb
{

class SliceDyn: public Slice
{
public:
	INLFUNC SliceDyn(){ data_ = NULL; }
	INLFUNC ~SliceDyn(){ _SafeFree32AL(data_); }
	template<typename StrExp>
	INLFUNC SliceDyn(StrExp& se){ data_ = NULL; *this = se; }
	template<typename StrExp>
	INLFUNC StrExp& operator = (StrExp& se)
	{	_SafeFree32AL(data_);
		size_ = se.GetLength();
		if((data_ = _Malloc32AL(char, size_)))
		{	se.CopyTo((LPSTR)data_);
		}else{ size_ = 0; }
		return se;
	}
};

namespace _details
{
template<bool is_pod>
struct SliceType
{
	template<typename T> static LPCSTR ptr(const T& v){ return (LPCSTR)&v; }
	template<typename T> static size_t size(const T& v){ return sizeof(v); }
};
template<>
struct SliceType<false>
{	
	static LPCSTR ptr(const rt::String_Ref& i){ return i.Begin(); }
	static size_t size(const rt::String_Ref& i){ return i.GetLength(); }

	static LPCSTR ptr(const rt::String& i){ return i.Begin(); }
	static size_t size(const rt::String& i){ return i.GetLength(); }

	static LPCSTR ptr(const std::string& i){ return i.data(); }
	static size_t size(const std::string& i){ return i.size(); }
};
} // namespace _details


class SliceValue: public Slice
{
protected:
	char	_embedded[8];
public:
	INLFUNC SliceValue():Slice(NULL, 0){}
	INLFUNC SliceValue(LPCVOID p, SIZE_T sz):Slice((char*)p, sz){}

	INLFUNC SliceValue(int i):Slice(_embedded, sizeof(i)){ *((int*)_embedded) = i; }
	INLFUNC SliceValue(BYTE i):Slice(_embedded, sizeof(i)){ *((BYTE*)_embedded) = i; }
	INLFUNC SliceValue(WORD i):Slice(_embedded, sizeof(i)){ *((WORD*)_embedded) = i; }
	INLFUNC SliceValue(DWORD i):Slice(_embedded, sizeof(i)){ *((DWORD*)_embedded) = i; }
	INLFUNC SliceValue(ULONGLONG i):Slice(_embedded, sizeof(i)){ *((ULONGLONG*)_embedded) = i; }
	INLFUNC SliceValue(LONGLONG i):Slice(_embedded, sizeof(i)){ *((LONGLONG*)_embedded) = i; }
	INLFUNC SliceValue(float i):Slice(_embedded, sizeof(i)){ *((float*)_embedded) = i; }
	INLFUNC SliceValue(double i):Slice(_embedded, sizeof(i)){ *((double*)_embedded) = i; }

	INLFUNC SliceValue(LPSTR str):Slice(str, str?strlen(str):0){}
	INLFUNC SliceValue(LPCSTR str):Slice(str, str?strlen(str):0){}
	
	INLFUNC SliceValue(const SliceDyn& i):Slice(i){}
	INLFUNC SliceValue(const Slice& i):Slice(i){}

	template<typename T>
	INLFUNC SliceValue(const T& x)
		:Slice(_details::SliceType<rt::TypeTraits<T>::IsPOD>::ptr(x),
			   _details::SliceType<rt::TypeTraits<T>::IsPOD>::size(x)
		)
	{}


	//template<size_t LEN>
	//INLFUNC SliceValue(char str[LEN]):Slice(str, LEN-1){}
	//template<size_t LEN>
	//INLFUNC SliceValue(const char str[LEN]):Slice(str, LEN-1){}


	INLFUNC SIZE_T GetSize() const { return size(); }

	INLFUNC rt::String_Ref ToString(SIZE_T off = 0) const { ASSERT(size_>=off); return rt::String_Ref(data_ + off, size_ - off); }
	template<typename T>
	const T& To(SIZE_T off = 0) const
	{	ASSERT(size_ >= off + sizeof(T));
		return *((T*)(data_ + off));
	}
};

#define SliceValueNull		::rocksdb::Slice()
#define SliceValueSS(x)		::rocksdb::Slice(x, sizeof(x)-1)

class RocksCursor
{
	Iterator* iter;
	friend class RocksDB;

public:
	template<typename T>
	INLFUNC const T&			Value() const { return *(T*)iter->value().data(); }
	template<typename T>
	INLFUNC const T&			Key() const { return *(T*)iter->key().data(); }

	INLFUNC const SliceValue	Key() const { return (const SliceValue&)iter->key(); }
	INLFUNC const SliceValue	Value() const { return (const SliceValue&)iter->value(); }
	INLFUNC SIZE_T				KeyLength() const { return iter->key().size(); }
	INLFUNC SIZE_T				ValueLength() const { return iter->value().size(); }
	INLFUNC						RocksCursor(){ iter = NULL; }
	INLFUNC						RocksCursor(Iterator* i):iter(i){}
	INLFUNC						~RocksCursor(){ _SafeDel(iter); }
	INLFUNC Iterator*			operator = (Iterator* it){ _SafeDel(iter); return iter = it; }
	INLFUNC bool				IsValid() const { return iter && iter->Valid(); }
	INLFUNC void				Next(){ iter->Next(); }
	INLFUNC void				Prev(){ iter->Prev(); }
	INLFUNC Iterator*			Detach(){ Iterator* ret = iter; iter = NULL; return ret; }
	INLFUNC bool				IsEmpty() const { return iter == NULL; }
};

class RocksDB
{
protected:
	ReadOptions		__DefaultReadOpt;
	WriteOptions	__DefaultWriteOpt;
	DB*		_pDB;
public:
public:
	INLFUNC RocksDB(){ _pDB = NULL; }
	INLFUNC ~RocksDB(){ Close(); }
	INLFUNC bool Open(LPCSTR db_filename, const Options* opt = NULL)
	{	ASSERT(_pDB == NULL);
		if(opt == NULL)
		{	Options* my;
			opt = my = _StackNew(Options)();
			my->create_if_missing = true;
		}
		DB* p;
		if(DB::Open(*opt, db_filename, &p).ok())
		{	_pDB = p;	return true; }
		else return false;
	}
	INLFUNC bool IsOpen() const { return _pDB!=NULL; }
	INLFUNC void Close(){ _SafeDel(_pDB); }
	INLFUNC bool Set(const SliceValue& k, const SliceValue& val, WriteOptions* opt = NULL){ ASSERT(_pDB); return _pDB->Put(opt?*opt:__DefaultWriteOpt, k, val).ok(); }
	INLFUNC bool Merge(const SliceValue& k, const SliceValue& val, WriteOptions* opt = NULL){ ASSERT(_pDB); return _pDB->Merge(opt?*opt:__DefaultWriteOpt, k, val).ok(); }
	INLFUNC bool Get(const SliceValue& k, std::string& str, ReadOptions* opt = NULL){ ASSERT(_pDB); return _pDB->Get(opt?*opt:__DefaultReadOpt, k, &str).ok(); }
	INLFUNC bool Has(const SliceValue& k, ReadOptions* opt = NULL){ thread_local std::string t; return Get(k, t, opt); }
	template<typename t_POD>
	INLFUNC bool Get(const SliceValue& k, t_POD* valout, ReadOptions* opt = NULL) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		if(_pDB->Get(opt?*opt:__DefaultReadOpt, k, &temp).ok() && temp.length() == sizeof(t_POD))
		{	memcpy(valout, temp.data(), sizeof(t_POD));
			return true;
		}else return false;
	}
	template<typename t_NUM>
	INLFUNC t_NUM Get(const SliceValue& k, t_NUM default_val = 0, ReadOptions* opt = NULL) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		return (_pDB->Get(opt?*opt:__DefaultReadOpt, k, &temp).ok() && temp.length() == sizeof(t_NUM))?
			   *((t_NUM*)temp.data()):default_val;
	}
	template<typename t_Type>
	INLFUNC const t_Type* Fetch(const SliceValue& k, SIZE_T* len_out = NULL, ReadOptions* opt = NULL) const // Get a inplace referred buffer, will be invalid after next Fetch
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		if(_pDB->Get(opt?*opt:__DefaultReadOpt, k, &temp).ok() && temp.length() >= sizeof(t_Type))
		{	if(len_out)*len_out = temp.length();
			return (t_Type*)temp.data();
		}
		else
		{	if(len_out)*len_out = 0;
			return NULL;
		}
	}
	INLFUNC rt::String_Ref Fetch(const SliceValue& k, ReadOptions* opt = NULL) const
	{	ASSERT_NONRECURSIVE;
		thread_local std::string temp;
		ASSERT(_pDB);
		return (_pDB->Get(opt?*opt:__DefaultReadOpt, k, &temp).ok())?
				rt::String_Ref(temp.data(), temp.length()):rt::String_Ref();
	}
	INLFUNC Iterator* Find(const SliceValue& begin, ReadOptions* opt = NULL)
	{	Iterator* it = _pDB->NewIterator(opt?*opt:__DefaultReadOpt);
		ASSERT(it);
		it->Seek(begin);
		return it;
	}
	INLFUNC Iterator* First(ReadOptions* opt = NULL)
	{	Iterator* it = _pDB->NewIterator(opt?*opt:__DefaultReadOpt);
		ASSERT(it);
		it->SeekToFirst();
		return it;
	}
	INLFUNC Iterator* Last(ReadOptions* opt = NULL)
	{	Iterator* it = _pDB->NewIterator(opt?*opt:__DefaultReadOpt);
		ASSERT(it);
		it->SeekToLast();
		return it;
	}
	INLFUNC bool Delete(const SliceValue& k, WriteOptions* opt = NULL){ ASSERT(_pDB); return _pDB->Delete(opt?*opt:__DefaultWriteOpt, k).ok(); }
	template<typename func_visit>
	INLFUNC SIZE_T ScanBackward(const func_visit& v, const SliceValue& begin, ReadOptions* opt = NULL) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(opt?*opt:__DefaultReadOpt);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->Seek(begin); it.IsValid(); it.Prev())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
	template<typename func_visit>
	INLFUNC SIZE_T ScanBackward(const func_visit& v, ReadOptions* opt = NULL) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(opt?*opt:__DefaultReadOpt);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->SeekToLast(); it.IsValid(); it.Prev())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
	template<typename func_visit>
	INLFUNC SIZE_T Scan(const func_visit& v, const SliceValue& begin, ReadOptions* opt = NULL) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(opt?*opt:__DefaultReadOpt);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->Seek(begin); it.IsValid(); it.Next())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
	template<typename func_visit>
	INLFUNC SIZE_T Scan(const func_visit& v, ReadOptions* opt = NULL) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(opt?*opt:__DefaultReadOpt);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->SeekToFirst(); it.IsValid(); it.Next())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
	template<typename func_visit>
	INLFUNC SIZE_T ScanPrefix(const func_visit& v, const SliceValue& prefix, ReadOptions* opt = NULL) const
	{	ASSERT(_pDB);
		RocksCursor it = _pDB->NewIterator(opt?*opt:__DefaultReadOpt);
		ASSERT(!it.IsEmpty());
		SIZE_T ret = 0;
		for(it.iter->Seek(prefix); it.IsValid() && it.Key().starts_with(prefix); it.Next())
		{	ret++;
			if(!rt::_details::_CallLambda<bool, decltype(v(it))>(true, v, it).retval)
				break;
		}
		return ret;
	}
};

INLFUNC bool RemoveDatabase(LPCSTR path)
{	if(os::File::IsExist(path))
		return os::File::RemovePath(path); 
	return true;
}

template<char separator = ':'>
class SeparatorPrefixTransform : public SliceTransform 
{
public:
	explicit SeparatorPrefixTransform() = default;
    virtual const char* Name() const override { return "SeparatorPrefixTransform"; }
	virtual Slice Transform(const Slice& src) const override {
		const char* p = src.data();
		const char* sep = strchr(p, separator);
		if(sep)
		{	return Slice(src.data(), (int)(sep - p));
		}else return src;
	}
	virtual bool InDomain(const Slice& src) const override {
		return true;
	}
	virtual bool InRange(const Slice& dst) const override {
		return dst[dst.size()-1] == ':' || strchr(dst.data(), ':') == NULL;
	}
	virtual bool SameResultWhenAppended(const Slice& prefix) const override {
		return strchr(prefix.data(), ':') != NULL;
	}
};


} // namespace rocksdb

