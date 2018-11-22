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

#include "string_type.h"


namespace rt
{

class _JObj
{
	rt::String		__Obj;
	rt::String_Ref	_Ref;
	bool			_AsBinary;
	char			_BinLen[10];
	UINT			_BinLenSize;
	void			_SetBinLen(){ _BinLenSize = rt::string_ops::itoa((UINT)_Ref.GetLength(), _BinLen); }
public:
	//FORCEINL _JObj(){}
	FORCEINL explicit _JObj(LPCSTR str, bool bin = false)
					{	_AsBinary = bin;
						if(bin){ _Ref = rt::String_Ref(str);			_SetBinLen(); }
						else _Ref = rt::String_Ref(str).TrimSpace();
					}
	FORCEINL explicit _JObj(const rt::String_Ref& str, bool bin = false)
					{	_AsBinary = bin;
						if(bin){ _Ref = str;							_SetBinLen(); }
						else _Ref = str.TrimSpace();
					}
	FORCEINL explicit _JObj(const rt::String& str, bool bin = false)
					{	_AsBinary = bin;
						if(bin){ _Ref = str;							_SetBinLen(); }
						else _Ref = str.TrimSpace();
					}
	FORCEINL explicit _JObj(LPSTR str, bool bin = false)
					{	_AsBinary = bin;
						if(bin){ _Ref = rt::String_Ref(str);			_SetBinLen(); }
						else _Ref = rt::String_Ref(str).TrimSpace();
					}
	template <template<class, class, class> class std_string, class _Traits, class _Alloc>
	FORCEINL explicit _JObj(const std_string<char, _Traits, _Alloc>& str, bool bin = false)
					{	_AsBinary = bin;
						if(bin){ _Ref = str;							_SetBinLen(); }
						else _Ref = rt::String_Ref(str).TrimSpace();
					}
	FORCEINL explicit _JObj(rt::String_Ref& str, bool bin = false)
					{	_AsBinary = bin;
						if(bin){ _Ref = str;							_SetBinLen(); }
						else _Ref = str.TrimSpace();
					}
	FORCEINL explicit _JObj(rt::String& str, bool bin = false)
					{	_AsBinary = bin;
						if(bin){ _Ref = str;							_SetBinLen(); }
						else _Ref = str.TrimSpace();
					}
	template<typename T>
	FORCEINL explicit _JObj(T& j, bool bin = false)
					{	_AsBinary = bin;
						j.ToString(__Obj);
						if(bin){ _Ref = __Obj;							_SetBinLen(); }
						else _Ref = __Obj.TrimSpace();
					}
	//template<typename T>
	//FORCEINL void operator = (const T& j){ j.ToString(__Obj); _Ref = __Obj; }
	//operator String_Ref() const 
	//{	static const rt::String_Ref nullstring("null", 4);
	//	return _Ref.IsEmpty()?nullstring:_Ref;
	//}
	FORCEINL UINT GetLength() const 
					{	if(_AsBinary)
						{	return 2 + _BinLenSize + (UINT)_Ref.GetLength() + 1;
						}
						else
						{	return _Ref.IsEmpty()?4:(UINT)_Ref.GetLength();
						}
					}
	FORCEINL UINT CopyTo(LPSTR p) const 
					{	if(_AsBinary)
						{	ASSERT(_Ref.GetLength() <= 0xffffff);
							*p = '<';
							memcpy(p+1, _BinLen, _BinLenSize);
							p += 2 + _BinLenSize;
							p[-1] = '/';
							_Ref.CopyTo(p);
							p[_Ref.GetLength()] = '>';
							return 2 + _BinLenSize + (UINT)_Ref.GetLength() + 1;
						}
						else
						{	if(_Ref.IsEmpty())
							{	*(DWORD*)p = 0x6c6c756e;
								return 4;
							}
							else return (UINT)_Ref.CopyTo(p);
						}
					}
};

namespace _details
{
	FORCEINL UINT _json_GetLengthPrev(LPVOID, bool no_commma = false){ return 0; }
	FORCEINL UINT _json_CopyPrev(LPVOID, LPSTR, bool no_commma = false){ return 0; }

	template<typename T>
	FORCEINL UINT _json_GetLengthPrev(T& n, bool no_commma = false){ return n.GetLength() + ((n.IsGhost()||no_commma)?0:2); }
	template<typename T>
	FORCEINL UINT _json_CopyPrev(T& n, LPSTR p, bool no_commma = false)
					{	UINT l = n.CopyTo(p);
						ASSERT(l == n.GetLength()); 
						if(n.IsGhost() || no_commma)return l;
						p[l]=','; p[l+1]='\n';
						return l+2;
					}
	FORCEINL UINT _json_StartClosure(LPVOID, LPSTR p){ p[0]='{'; p[1]='\n'; return 2; }
	FORCEINL UINT _json_EndClosure(LPVOID, LPSTR p){ p[0]='\n'; p[1]='}'; return 2; }
	FORCEINL UINT _json_GetClosureLength(LPVOID){ return 4; }

	FORCEINL UINT _json_StartClosure(int, LPSTR p){ return 0; }
	FORCEINL UINT _json_EndClosure(int, LPSTR p){ return 0; }
	FORCEINL UINT _json_GetClosureLength(int){ return 0; }

} // namespace _details


template<typename t_Prev, typename t_Val, typename void_is_end = LPVOID>
struct _JVar
{
	t_Prev			prev;
	rt::String_Ref	tagname;
	t_Val			value;
	DWORD			value_type;
	
	template<typename t_V>
	FORCEINL _JVar(const rt::String_Ref& name, t_V& v, DWORD vt):tagname(name), value(v), value_type(vt){}
	FORCEINL _JVar(){}
	template<typename Prev, typename t_V>
	FORCEINL _JVar(Prev& n, const rt::String_Ref& name,t_V& v, DWORD vt):tagname(name), value(v), value_type(vt), prev(n){}

	bool	IsGhost() const { return tagname.IsEmpty(); }

	UINT	GetLength() const
	{	if(IsGhost())return _details::_json_GetLengthPrev(prev, true) + _details::_json_GetClosureLength((void_is_end)0);
		return (UINT)(tagname.GetLength() + 5 +
						value.GetLength() + 
						_details::_json_GetLengthPrev(prev) +
						_details::_json_GetClosureLength((void_is_end)0)
				);
	}
	UINT	CopyTo(LPSTR p) const
	{	LPSTR org = p;
		p += _details::_json_StartClosure((void_is_end)0, p);
		if(!IsGhost())
		{
			p += _details::_json_CopyPrev(prev, p);
			*p++='"'; p += tagname.CopyTo(p); *p++='"';
			*p++=':';
			static const char sep[8] = { ' ', '"', ' ', ' ', ' ', '"', ' ', ' ' };
			*p++=sep[value_type];
			int len = (int)value.CopyTo(p);
			p += len;
			*p++=sep[value_type+4];
		}
		else
		{	p += _details::_json_CopyPrev(prev, p, true);
		}
		p += _details::_json_EndClosure((void_is_end)0, p);
		return (UINT)(p - org);
	}
	void ToString(rt::String& out) const
	{	VERIFY(out.SetLength(GetLength()));
		out.SetLength(CopyTo(out.Begin()));
	}
	// override operator , for connecting key-value pairs in declaring json object
	template<typename t_right>
	FORCEINL auto operator , (const _JVar<LPVOID, t_right>& right) const
	{	return _JVar< _JVar<t_Prev,t_Val,int>, t_right>((const _JVar<t_Prev,t_Val,int>&)*this, right.tagname, right.value, right.value_type);
	}
};

namespace _details
{
	template<typename T>
	struct _NeedQuote{ static const int need = 0; };
	template<>
	struct _NeedQuote<const rt::String_Ref*>{ static const int need = 1; };
}

template<int t_LEN = 2048>
class _JArray
{
	char	_buf[t_LEN];
	UINT	_len;
public:
	FORCEINL _JArray(){ _buf[0] = '['; _len = 1; }

#define JA_NUMERIC_ITEM(numeric_type)	FORCEINL void append(numeric_type x){ append(rt::tos::Number(x), false); }
	JA_NUMERIC_ITEM(bool)
	JA_NUMERIC_ITEM(short)
	JA_NUMERIC_ITEM(unsigned short)
	JA_NUMERIC_ITEM(int)
	JA_NUMERIC_ITEM(UINT)
#if defined(PLATFORM_WIN)
	JA_NUMERIC_ITEM(long)
	JA_NUMERIC_ITEM(unsigned long)
#endif
	JA_NUMERIC_ITEM(LONGLONG)
	JA_NUMERIC_ITEM(ULONGLONG)
	JA_NUMERIC_ITEM(float)
	JA_NUMERIC_ITEM(double)
#undef JA_NUMERIC_ITEM
		
	FORCEINL void append(LPCSTR x){ append(rt::String_Ref(x), true); }
	FORCEINL void append(LPSTR x){ append(rt::String_Ref(x), true); }
	FORCEINL void append(char x){ append(rt::String_Ref(&x, 1), true); }

	template<typename T>
	FORCEINL void	append(const T& obj, bool need_quote = (bool)rt::_details::_NeedQuote<const T*>::need)
	{	if(_len != 1)
		{	_buf[_len++] = ',';
			//_buf[_len++] = '\n';
		}
		if(need_quote)
		{
			_buf[_len++] = '"';
			_len += (UINT)obj.CopyTo(_buf+_len);
			_buf[_len++] = '"';
			ASSERT(_len < t_LEN);
		}
		else
		{
			_len += (UINT)obj.CopyTo(_buf+_len);
			ASSERT(_len < t_LEN);
		}
	}
	FORCEINL UINT	GetLength() const { return _len + 1; }
	FORCEINL UINT	CopyTo(LPSTR p) const 
	{	memcpy(p, _buf, _len);
		//p[_len] = '\n';
		p[_len] = ']';
		return _len+1;
	}
	
	template<typename T>
	FORCEINL rt::_JArray<t_LEN>& operator , (T&& right){ append(right); return *this; }
};

#define J_EXPR_CONNECT_OP(type, type_in, vt)	\
FORCEINL _JVar<LPVOID, type>					\
operator = (type_in p)							\
{ return tagname.IsEmpty() ? _JVar<LPVOID, type>(tagname, type(), vt) : _JVar<LPVOID, type>(tagname, p, vt); }

struct _JTag
{	
	template<typename t_Obj>
	struct _O
	{	const t_Obj* _Obj;
		FORCEINL UINT	GetLength() const { ASSERT(_Obj); return (UINT)_Obj->GetLength(); }
		FORCEINL UINT	CopyTo(LPSTR p) const { ASSERT(_Obj); return (UINT)_Obj->CopyTo(p); }
		FORCEINL		_O(const t_Obj& x):_Obj(&x){}
		FORCEINL		_O(){ _Obj = NULL; }
	};

	enum _tagVARTYPE
	{	VARTYPE_BUILTIN = 0,
		VARTYPE_STRING  = 1,
		VARTYPE_ARRAY   = 2,
		VARTYPE_OBJECT  = 3
	};
	rt::String_Ref	tagname;
	FORCEINL _JTag(const rt::String_Ref& name):tagname(name){}

	J_EXPR_CONNECT_OP(String_Ref,					const String_Ref&	, _JTag::VARTYPE_STRING)
	J_EXPR_CONNECT_OP(String_Ref,					const char*			, _JTag::VARTYPE_STRING)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,2)>,	char				, _JTag::VARTYPE_STRING)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,12)>,	int					, _JTag::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,7)>,	bool				, _JTag::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,12)>,	unsigned int		, _JTag::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,25)>,	LONGLONG			, _JTag::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,25)>,	ULONGLONG			, _JTag::VARTYPE_BUILTIN)
#if defined(PLATFORM_WIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,12)>,	long				, _JTag::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,12)>,	unsigned long		, _JTag::VARTYPE_BUILTIN)
#endif
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,32)>,	float				, _JTag::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(tos::S_<MARCO_CONCAT(1,64)>,	double				, _JTag::VARTYPE_BUILTIN)
	J_EXPR_CONNECT_OP(_O<_JObj>,	const _JObj& 		, _JTag::VARTYPE_OBJECT)

	template<typename prev, typename type>
	FORCEINL auto operator = (const _JVar<prev, type>& p)
	{	return _JVar<LPVOID, _O<const _JVar<prev, type>> >(tagname, p, _JTag::VARTYPE_OBJECT);
	}

	template<int t_LEN>
	FORCEINL auto operator = (const _JArray<t_LEN>& p)
	{	return _JVar<LPVOID, _O<const _JArray<t_LEN>> >(tagname, p, _JTag::VARTYPE_ARRAY);
	}

	template<typename t_Left, typename t_Right>
	FORCEINL auto operator = (const _SE<t_Left,t_Right>& p)
	{	return _JVar<LPVOID, _O<const _SE<t_Left,t_Right>> >(tagname, p, _JTag::VARTYPE_STRING);
	}
};

#undef J_EXPR_CONNECT_OP


class _JObject
{
	rt::String	__Temp;
	rt::String& _Save;
public:
	_JObject():_Save(__Temp){ __Temp = "{}"; }
	_JObject(rt::String& out):_Save(out)
	{	if(out.IsEmpty()){ out = "{}"; }
		_Save = _Save.TrimSpace();
		ASSERT(_Save.Last() == '}');
	}
	template<typename t_left, typename t_right>
	void operator << (const _JVar<t_left,t_right>& json)
	{
		if(_Save.GetLength() <= 2)
		{
			_Save = json;
		}
		else
		{	_Save._len--;
			SIZE_T e = _Save.GetLength();
			_Save += json;
			_Save.Begin()[e] = ',';
		}
	}
	const rt::String_Ref& GetString() const { return _Save; }
	operator const rt::String_Ref&() const { return _Save; }
};

} // namespace rt


#define J(x)					rt::_JTag(#x)
#define J_IF(cond, definition)	((cond) ? (definition) : decltype(definition)())
#define JA(...)					((rt::_JArray<>(), __VA_ARGS__))
#define JB(x)					(rt::_JObj((x), true))


namespace rt
{

typedef enum _tagJsonType
{
	JSON_STRING,
	JSON_NUMBER,
	JSON_BOOL,
	JSON_NULL,
	JSON_OBJECT,
	JSON_ARRAY,
	JSON_BINARY,
	JSON_CORRUPTED
} JsonType;


class JsonKeyValuePair
{
	friend class JsonObject;
	rt::String_Ref	_Key;
	rt::String_Ref	_Value;
public:
	static JsonType GetValueType(const rt::String_Ref& Value) 
	{	
		if(Value.IsEmpty())return JSON_NULL;
		if(Value[-1] == '"' || Value[-1] == '\'')return JSON_STRING;
		if(Value[-1] == '/')return JSON_BINARY;
		if(Value[0] == '{')return JSON_OBJECT;
		if(Value[0] == '[')return JSON_ARRAY;
		if(Value[0] == 't' || Value[0] == 'T' || Value[0] == 'f' || Value[0] == 'F')return JSON_BOOL;
		if((Value[0] >= '0' && Value[0] <= '9') || Value[0] == '-')return JSON_NUMBER;
		if(Value.GetLength() == 4 && *(DWORD*)Value.Begin() == 0x6c6c756e)return JSON_NULL;
		return JSON_CORRUPTED;
	}
public:
	const rt::String_Ref& GetKey() const { return _Key; }
	const rt::String_Ref& GetValue() const { return _Value; }
	rt::String_Ref GetValueRaw() const 
	{	if(_Value[-1] == '"' || _Value[-1] == '\'')return rt::String_Ref(_Value.Begin()-1, _Value.End()+1);
		if(_Value[-1] == '/')
		{	LPCSTR p = _Value.Begin() - 1;
			while(*p != '<'){ p--; ASSERT(p>_Key.End()); };
			return rt::String_Ref(p, _Value.End()+1);
		}
		return _Value;
	}
	JsonType GetValueType() const { return GetValueType(_Value); }

	template<typename T>
	INLFUNC T GetValueAs(T default_val) const
	{	rt::String_Ref s = GetValue();
		if(!s.IsEmpty())s.ToNumber(default_val); 
		return default_val;
	}
	template<typename T>
	INLFUNC T GetValueAs() const
	{	return GetValueAs<T>(0);
	}
	INLFUNC void Empty(){ _Key.Empty(); _Value.Empty(); }
	INLFUNC bool IsEmpty() const { return _Key.IsEmpty() || _Value.IsEmpty(); }
};
	
template<>
INLFUNC bool JsonKeyValuePair::GetValueAs<bool>(bool default_val) const
{	rt::String_Ref s = GetValue();
	if(!s.IsEmpty()){ return s[0] == 'T' || s[0] == 't'; }
	return default_val;
}


class JsonObject
{
	friend class JsonArray;
	rt::String_Ref	_Doc;
protected:
	static const INLFUNC LPCSTR _skip_whitespace(LPCSTR p, LPCSTR end){ while(p < end && *p<=' ' && *p>=0)p++; return p; }
	static const INLFUNC LPCSTR _scan_text(LPCSTR p, LPCSTR end){ while(p < end && _is_text(*p))p++; return p; }
	static const INLFUNC LPCSTR _seek_char(LPCSTR p, LPCSTR end, char c){ while(p < end && *p!=c)p++; return p; }
	static const INLFUNC void	_cook_raw_value(LPCSTR& p, LPCSTR& tail)
	{	if(*p == '"' || *p == '\''){ p++; tail--; } // string
		else if(*p == '<'){	p+=2; while(*p != '/'){ p++; ASSERT(p<tail); } p++;	tail--; }  // binary
	}
	static const INLFUNC bool   _is_text(int c)
	{	static const rt::CharacterSet sep(rt::SS("\x7\x9\xa\xb\xc\xd '\",:{}[]"));
		return !sep.Has(c); // c<0 || ( c>' ' && c!='\'' && c!='"' && c!=',' && c!=':' && c!='{' && c!='}'); 
	}
	static const INLFUNC LPCSTR _seek_char_escape(LPCSTR p, LPCSTR end, char c)
	{	for(;p < end;p++)
		{	if(*p==c)
			{	LPCSTR s = p-1;
				for(;*s == '\\';s--);
				if(((int)(s-p))&1)return p;
			}
		}
		return end;
	}
	static const INLFUNC int   _match_closure(int c)
	{	if(c == '"' || c == '\'')return c;
		else if(c == '[')return ']';
		else if(c == '{')return '}';
		else return 0;
	}
	static const LPCSTR _seek_json_object_closure(LPCSTR p, LPCSTR end)
	{	if(*p == '"' || *p == '\'') // string
		{	p = _seek_char_escape(p+1, end, *p);
			if(p != end)return p+1;
			return NULL;
		}
		else if(*p == '<') // binary
		{	UINT len;
			p++;
			p += rt::String_Ref(p, end).ToNumber(len);
			p += 1 + len;
			if(*p == '>')return p+1;
			return NULL;
		}
		else
		{	char c = _match_closure(*p);
			if(c)
			{	char depth_inc = *p;
				char depth_dec = c;
				int closure_depth = 1;
				p++;
				
				while(p<end)
				{
					if(*p == depth_inc)closure_depth++;
					else if(*p == depth_dec)
					{	closure_depth--;
						if(closure_depth == 0)return p+1;
					}
					else if(*p == '"' || *p == '\'')
					{	p = _seek_char_escape(p+1,end,*p);
						if(p == end)return NULL;
					}
					else if(*p == '<')
					{
						p = _seek_json_object_closure(p, end);
						if(!p)return NULL;
					}
					p++;
				}
				return NULL;
			}
			else return _scan_text(p, end);
		}
	}

	LPCSTR _LocateValue(const rt::String_Ref& xpath, bool bDoNotSplitDot = false);

public:
	INLFUNC JsonObject(){};
	INLFUNC JsonObject(LPCSTR str, UINT length){ Attach(str, length); }
	INLFUNC JsonObject(const rt::String_Ref& doc){ Attach(doc); }
	INLFUNC void Attach(LPCSTR str, UINT length){ Attach(rt::String_Ref(str, length)); }
	INLFUNC void Attach(const rt::String_Ref& doc)
	{	SSIZE_T s = doc.FindCharacter('{');
		if(s>=0)_Doc = doc.SubStr(s);
	}
	INLFUNC const rt::String_Ref GetValue(const rt::String_Ref& xpath, const rt::String_Ref& default_val = rt::String_Ref(), bool bDoNotSplitDot = false)	// xxx.yyy.zzz
	{	if(xpath.IsEmpty())return _Doc;
		LPCSTR p, tail;
		if((p = _LocateValue(xpath, bDoNotSplitDot)))
		{	if((tail = _seek_json_object_closure(p,_Doc.End())))
			{	_cook_raw_value(p, tail);
				return rt::String_Ref(p, tail);
			}
		}
		return default_val;
	}
	template<typename T>
	INLFUNC T GetValueAs(const rt::String_Ref& xpath, T default_val)
	{	rt::String_Ref s = GetValue(xpath);
		if(!s.IsEmpty())s.ToNumber(default_val);
		return default_val;
	}
	template<typename T>
	INLFUNC T GetValueAs(const rt::String_Ref& xpath)
	{	return GetValueAs<T>(xpath, 0);
	}

	INLFUNC bool IsEmpty() const { return _Doc.IsEmpty(); }
	INLFUNC bool IsEmptyObject() const { 
		if(IsEmpty())
			return true;

		JsonKeyValuePair kvp;
		if(GetNextKeyValuePair(kvp))
			return false;
		else
			return true;
	}

	INLFUNC const rt::String_Ref& GetString() const { return _Doc; }
	INLFUNC operator const rt::String_Ref () const { return _Doc; }
	INLFUNC static void UnescapeStringValue(const rt::String_Ref& in, rt::String& val_out)
	{
		if(in.IsEmpty()){ val_out.Empty(); return; }

		VERIFY(val_out.SetLength(in.GetLength()));

		static const rt::CharacterSet esc("bfrntv","\b\f\r\n\t\v");
		static const rt::CharacterSet hex("0123456789abcdefABCDEF","\x1\x2\x3\x4\x5\x6\x7\x8\x9\xa\xb\xc\xd\xe\xf\x10\xb\xc\xd\xe\xf\x10");
		
		LPSTR p = val_out.Begin();	LPCSTR s = in.Begin();	LPCSTR end = in.End();
		while(s<end-1)
		{	if(*s == '\\'){
				if(s[1] == 'u' && s+5<end)
				{	s+=2;
					int c =  ((hex.Mapped(s[0])-1)<<12) + ((hex.Mapped(s[1])-1)<<8) + ((hex.Mapped(s[2])-1)<<4) + (hex.Mapped(s[3])-1);
					if(c <= 0x7f){ *p = (char)c; p++; }
					else if(c > 0x7ff)	// 1110xxxx 	10xxxxxx 	10xxxxxx
					{	*((DWORD*)p) = 0x8080e0 | ((c>>12)&0xf) | ((c<<2)&0x3f00) | ((0x3f&c)<<16);
						p+=3;
					}
					else	// 110xxxxx 	10xxxxxx
					{	*((WORD*)p) = 0x80c0 | ((c>>6)&0x1f) | ((0x3f&c)<<8);
						p+=2;
					}
					s+=4;
				}
				else if(s[1] == 'x' && s+3<end)
				{	s+=2;
					*p++ = ((hex.Mapped(s[0])-1)<<4) + (hex.Mapped(s[1])-1);
					s+=2;
				}
				else
				{	*p++ = esc.Mapped(s[1]);
					s+=2;
				}
			}else{ *p++ = *s++; }
		}
		if(s<end){ *p++ = *s++; }
		val_out._len = p - val_out._p + 1;
	}
	INLFUNC void GetValueUnescaped(rt::String& val_out, const rt::String_Ref& xpath, const rt::String_Ref& default_val = rt::String_Ref(), bool bDoNotSplitDot = false)	// xxx.yyy.zzz
	{
		rt::String_Ref in = GetValue(xpath, default_val, bDoNotSplitDot);
		UnescapeStringValue(in, val_out);
	}
	bool GetNextKeyValuePair(JsonKeyValuePair& kvp) const
	{	
		if(_Doc.IsEmpty())return false;
		LPCSTR p = _Doc.Begin() + 1;
		LPCSTR end = _Doc.End();
		if(kvp._Key.Begin()>=p && kvp._Value.End()<=_Doc.End())p = kvp._Value.End();
		if(*p=='"' || *p=='\'' || *p=='>')p++;

		p = _skip_whitespace(p, end);
		if(*p == '}')return false;
		if(*p == ',')p = _skip_whitespace(p+1, end);

		LPCSTR name_end;
		if(*p=='"' || *p=='\''){ 
			name_end = _seek_char_escape(p+1, end, *p);
			p++;
		}
		else{ name_end = _scan_text(p, end); }
		if(end == name_end || p == name_end)return false;

		rt::String_Ref key(p, name_end);

		LPCSTR value = _seek_char(name_end, end, ':');
		if(value == end)return false; // ':' not found
		value = _skip_whitespace(value+1, end);
		LPCSTR tail = _seek_json_object_closure(value, end);
		if(tail == NULL)return false;

		_cook_raw_value(value, tail);

		kvp._Key =  key;
		kvp._Value = rt::String_Ref(value, tail);
		return !kvp._Value.IsEmpty() || *tail == '"';
	}
	void Derive(const rt::String_Ref& sub, rt::String& derived, bool append = false) const
	{	Override(_Doc, sub, derived, append);
	}
	void Derive(const rt::String_Ref& key, const rt::String_Ref& val_raw, rt::String& derived, bool append = false)
	{	Override(_Doc, key, val_raw, derived, append);
	}
	template<typename T1,typename T2,typename T3>
	void Derive(const rt::_JVar<T1,T2,T3>& sub, rt::String& derived, bool append = false) const
	{	Override(_Doc, sub, derived, append);
	}
	template<typename T1,typename T2,typename T3>
	static void Override(const rt::String_Ref& base, const rt::_JVar<T1,T2,T3>& sub, rt::String& derived, bool append = false)
	{	Override(base, rt::String(sub), derived, append);
	}
	static void Override(const rt::String_Ref& base, const rt::String_Ref& sub, rt::String& derived, bool append = false)
	{	typedef rt::hash_map<rt::String_Ref, rt::String_Ref, rt::String_Ref::hash_compare> t_ValList;
		t_ValList	vals;
		{	JsonKeyValuePair kv;
			JsonObject base_doc(base);
			while(base_doc.GetNextKeyValuePair(kv))
				vals[kv.GetKey()] = kv.GetValueRaw();
		}
		{	JsonKeyValuePair kv;
			JsonObject base_doc(sub);
			while(base_doc.GetNextKeyValuePair(kv))
				vals[kv.GetKey()] = kv.GetValueRaw();
		}
		if(!append)derived.Empty();
		t_ValList::const_iterator it = vals.begin();
		for(; it != vals.end(); it++)
		{	if(derived.IsEmpty())
				derived = rt::SS("{ \"") + it->first + rt::SS("\" : ") + it->second;
			else
				derived += rt::SS(",\n  \"") + it->first + rt::SS("\" : ") + it->second;
		}
		if(derived.IsEmpty())derived = rt::SS("{}");
		else derived += '}';
	}
	static void Override(const rt::String_Ref& base, const rt::String_Ref& key, const rt::String_Ref& val_raw, rt::String& derived, bool append = false)
	{	typedef rt::hash_map<rt::String_Ref, rt::String_Ref, rt::String_Ref::hash_compare> t_ValList;
		t_ValList	vals;
		{	JsonKeyValuePair kv;
			JsonObject base_doc(base);
			while(base_doc.GetNextKeyValuePair(kv))
				vals[kv.GetKey()] = kv.GetValueRaw();
		}
		vals[key] = val_raw;
		t_ValList::const_iterator it = vals.begin();
		if(!append)derived.Empty();
		for(; it != vals.end(); it++)
		{	if(derived.IsEmpty())
				derived = rt::SS("{ \"") + it->first + rt::SS("\" : ") + it->second;
			else
				derived += rt::SS(",\n  \"") + it->first + rt::SS("\" : ") + it->second;
		}
		if(derived.IsEmpty())derived = rt::SS("{}");
		else derived += '}';
	}
	static void RemoveKeys(const rt::String_Ref& source, const rt::String_Ref& keys_to_exclude, rt::String& removed)
	{	
		rt::String_Ref keys[256];
		UINT co = keys_to_exclude.Split(keys, sizeofArray(keys), ",;|");
		if(co == 0){ removed = source; return; }
		JsonKeyValuePair kv;
		JsonObject doc(source);
		while(doc.GetNextKeyValuePair(kv))
		{
			for(UINT i=0;i<co;i++)
				if(kv.GetKey() == keys[i])goto SKIP_THE_KEY;
			if(removed.IsEmpty()){ removed = rt::SS("{ "); }
			else{ removed += rt::SS(",  "); }
			removed += rt::SS("\n  \"") + kv.GetKey() + rt::SS("\": ") + kv.GetValueRaw();
SKIP_THE_KEY:
			continue;
		}
		if(removed.IsEmpty()){ removed = rt::SS("{ }"); }
		else{ removed += rt::SS("\n}"); }
	}
};
template<>
INLFUNC bool JsonObject::GetValueAs<bool>(const rt::String_Ref& xpath, bool default_val)
{	rt::String_Ref s = GetValue(xpath);
	if(!s.IsEmpty()){ return s[0] != 'F' && s[0] != 'f' && s[0] != '0'; }
	return default_val;
}

class JsonArray
{
	rt::String_Ref	_Doc;
public:
	INLFUNC bool IsEmpty() const { return _Doc.IsEmpty(); }
	INLFUNC JsonArray(const rt::String_Ref& doc)
	{	SSIZE_T s = doc.FindCharacter('[');
		if(s>=0)_Doc = doc.SubStr(s);
	}
	INLFUNC bool GetNextObjectRaw(JsonObject& obj) const { return GetNextObjectRaw(obj._Doc); }
	INLFUNC bool GetNextObjectRaw(rt::String_Ref& obj) const
	{
		if(_Doc.IsEmpty())return false;
		LPCSTR _CurObject = obj.End();
		if(_CurObject>=_Doc.Begin() && _CurObject<=_Doc.End())
		{	
			if(*_CurObject == '"' && obj.Begin()[-1] == '"')
				_CurObject++;
			_CurObject = JsonObject::_skip_whitespace(_CurObject, _Doc.End());
			if(*_CurObject == ',')
			{	_CurObject++;
			}else return false;
		}
		else
		{	_CurObject = _Doc.Begin() + 1;
		}

		LPCSTR p = JsonObject::_skip_whitespace(_CurObject, _Doc.End());
		if(*p == ']')return false;

		LPCSTR tail = JsonObject::_seek_json_object_closure(p,_Doc.End());
		if(tail)
		{	
			obj = rt::String_Ref(p, tail);
			return true;
		}
		return false;
	}
	INLFUNC bool GetNextObject(rt::String_Ref& obj) const
	{	
		if(GetNextObjectRaw(obj))
		{	
			LPCSTR p = obj.Begin();
			LPCSTR tail = obj.End();
			JsonObject::_cook_raw_value(p, tail);
			obj = rt::String_Ref(p, tail);

			return true;
		}
		else return false;
	}
	template<typename t_Num>
	INLFUNC UINT CopyNumbers(t_Num* pDest, SIZE_T DestLen) const // return actual number of numbers copied
	{	
		SIZE_T i = 0;
		rt::String_Ref item;
		for(; i<DestLen; i++)
		{	if(GetNextObject(item))
			{	item.ToNumber(pDest[i]);
			}else break;
		}
		return (UINT)i;
	}
	INLFUNC SIZE_T GetSize() const
	{	
		rt::String_Ref	obj;
		SIZE_T c = 0;
		while(GetNextObjectRaw(obj))
			c++;
		return c;
	}
};

INLFUNC LPCSTR JsonObject::_LocateValue(const rt::String_Ref& xpath, bool bDoNotSplitDot /* = false */)
{
	if(_Doc.IsEmpty())return NULL;

	rt::String_Ref	path_seg[256];

	int segco;
	if (bDoNotSplitDot)
		segco = xpath.Split<true>(path_seg, sizeofArray(path_seg), "[]");
	else
		segco = xpath.Split<true>(path_seg, sizeofArray(path_seg), ".[]");
	if(path_seg[segco-1].IsEmpty())segco--;

	LPCSTR p = _Doc.Begin();
	LPCSTR end = _Doc.End();

	if(*p != '{')return NULL;
	for(int i=0; i<segco; i++)
	{
		rt::String_Ref name = path_seg[i];

		if(name[-1] != '[')
		{
			for(;;)
			{	p = _skip_whitespace(p+1, end);
				if(*p == '}')return NULL; // name not found
				LPCSTR name_end;
				if(*p=='"' || *p=='\''){ 
					name_end = _seek_char_escape(p+1, end, *p);
					p++;
				}
				else{ name_end = _scan_text(p, end); }
				if(end == name_end || p == name_end)return NULL;

				LPCSTR value = _seek_char(name_end, end, ':');
				if(value == end)return NULL; // ':' not found
				value = _skip_whitespace(value+1, end);

				if(rt::String_Ref(p, name_end) == name)
				{	// found
					if(i == segco-1)return value;
					p = value;
					if(*p != '{' && *p != '[')return NULL; // an object expected
					break;
				}
				else
				{	p = _seek_json_object_closure(value, end); // p[-1] = '}' or ']' or '"' or last char of the value
					if(p==NULL)return NULL;
					p = _skip_whitespace(p, end);
					if(*p != ',')return NULL; // end of object reached, name not found
				}
			}
		}
		else
		{	if(*p != '[')return NULL; // not an array
			UINT co = 0;
			name.ToNumber(co);
			rt::JsonObject item;
			JsonArray  arr(rt::String_Ref(p, _Doc.End()));
			int s = -1;
			do
			{	if(!arr.GetNextObjectRaw(item))return NULL;
				s++;
			}while(s<(int)co);
			p = item._Doc.Begin();
			if(i == segco-1)
				return p;
		}
	}

	ASSERT(0);
	return NULL;
}

class JsonEscapeString: public rt::String
{
public:
	JsonEscapeString(const rt::String_Ref& c_string, bool add_quote = false)
	{	
		int open = 0;
		if(add_quote)(*this)[open++] = '"';

		SetLength(c_string.GetLength()*2 + 2);
		for(UINT i=0;i<c_string.GetLength();i++)
		{	static const rt::CharacterSet_Escape esc;
			int c = c_string[i];
			if(c)
			{
				if(!esc.Has(c))
				{	(*this)[open++] = c;
				}
				else
				{	(*this)[open++] = '\\';
					(*this)[open++] = esc.Mapped(c);
				}
			}
			else
			{	*(DWORD*)&(*this)[open] = 0x3030785c;
				open += 4;
			}
		}

		if(add_quote)(*this)[open++] = '"';
		SetLength(open);
	}
};

class JsonUnescapeString: public rt::String
{
public:
	JsonUnescapeString(const rt::String_Ref& json_string)
	{	
		rt::JsonObject::UnescapeStringValue(json_string, *this);
	}
};


} // namespace rt

