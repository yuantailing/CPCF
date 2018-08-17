#pragma once

//////////////////////////////////////////////////////////////////////
// Cross-Platform Foundation (CPF)
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

#include "../../os/kernel.h"


#ifdef PLATFORM_INTEL_IPP_SUPPORT
#include "../ipp/ipp_core.h"
#include "../ipp/inc/ippcp.h"
#endif

#pragma warning(disable:4101)

#if defined(PLATFORM_WIN)
	#ifdef PLATFORM_64BIT
		#include "./platforms/win_x64/botan_all.h"
	#else
		#include "./platforms/win_x86/botan_all.h"
	#endif
#elif defined(PLATFORM_ANDROID)
	#if defined (__mips__)
		#include "./platforms/botan_ndk_mips.h"
	#elif defined (__arm__)
		#include "./platforms/botan_ndk_arm.h"
		// treatment for NDK, because it is not supporting C++ exception
		#define try
		#define catch(x)	if(0)
	#elif defined (__i386__)
		#include "./platforms/botan_ndk_x86.h"
	#else
		#error unknown driod platform
	#endif
#elif defined(PLATFORM_MAC)
	#ifdef PLATFORM_64BIT
		#include "./platforms/mac_x64/botan_all.h"
	#else
		#include "./platforms/mac_x64/botan_all.h"
	#endif
#elif defined(PLATFORM_IOS)
    #ifdef PLATFORM_64BIT
        #include "./platforms/ios_64b/botan_all.h"
    #else
        #include "./platforms/ios_32b/botan_all.h"
    #endif
#elif defined(PLATFORM_LINUX)
    #ifdef PLATFORM_64BIT
        #include "./platforms/linux_x64/botan_all.h"
    #else
        #include "./platforms/linux_x32/botan_all.h"
    #endif
#else
	#error unknown platform
#endif

#ifdef PLATFORM_ANDROID
#define _LOG_EXPCEPTION(x)	{ do { (void)(x); } while (0) }
#else
#define _LOG_EXPCEPTION(x)	_LOG(x)
#endif


namespace sec
{

enum _tagHashMethod
{	HASH_CRC32 = 0,
	HASH_MD5,
	HASH_SHA1,	// aka SHA160
	HASH_SHA224,
	HASH_SHA256,
	HASH_SHA384,
	HASH_SHA512,
	HASH_MAX,
};

namespace _details
{
	template<int HASH_METHOD>
	struct _HashSize;
		template<> struct _HashSize<HASH_CRC32>
		{	static const int size = 4;
		};
		template<> struct _HashSize<HASH_MD5>
		{	static const int size = 16; 
		};
		template<> struct _HashSize<HASH_SHA1>
		{	static const int size = 20; 
		};
		template<> struct _HashSize<HASH_SHA224>
		{	static const int size = 28; 
		};
		template<> struct _HashSize<HASH_SHA256>
		{	static const int size = 32; 
		};
		template<> struct _HashSize<HASH_SHA384>
		{	static const int size = 48; 
		};
		template<> struct _HashSize<HASH_SHA512>
		{	static const int size = 64; 
		};
} // namespace _details


//////////////////////////////////////////////////////
// Symmetric Cryptography AES
enum _tagCryptoMethod
{
	CIPHER_AES128 = 1,
	CIPHER_AES256
};

namespace _details
{

template<UINT _METHOD>
struct	_AES_DataAlign;
	template<> struct _AES_DataAlign<CIPHER_AES128>{ static const int Result = 128/8-1; };
	template<> struct _AES_DataAlign<CIPHER_AES256>{ static const int Result = 256/8-1; };

} // namespace _details
} // namespace sec

#ifdef PLATFORM_INTEL_IPP_SUPPORT
namespace sec
{
namespace _details
{
	template<UINT _METHOD>
	struct _get_hash_context_size;
		template<> struct _get_hash_context_size<HASH_MD5>{ static const UINT Result = 103; };
		template<> struct _get_hash_context_size<HASH_SHA1>{ static const UINT Result = 111; };
		template<> struct _get_hash_context_size<HASH_SHA224>{ static const UINT Result = 119; };
		template<> struct _get_hash_context_size<HASH_SHA256>{ static const UINT Result = 119; };
		template<> struct _get_hash_context_size<HASH_SHA384>{ static const UINT Result = 239; };
		template<> struct _get_hash_context_size<HASH_SHA512>{ static const UINT Result = 239; };

	template<UINT _METHOD>
	struct _get_hmachash_context_size;
		template<> struct _get_hmachash_context_size<HASH_MD5>{ static const UINT Result = 103; };
		template<> struct _get_hmachash_context_size<HASH_SHA1>{ static const UINT Result = 163; };
		template<> struct _get_hmachash_context_size<HASH_SHA224>{ static const UINT Result = 119; };
		template<> struct _get_hmachash_context_size<HASH_SHA256>{ static const UINT Result = 119; };
		template<> struct _get_hmachash_context_size<HASH_SHA384>{ static const UINT Result = 239; };
		template<> struct _get_hmachash_context_size<HASH_SHA512>{ static const UINT Result = 239; };
} // namespace _details
} // namespace sec

namespace sec
{

template<UINT _METHOD = HASH_MD5> class Hash;
template<UINT _METHOD = HASH_MD5> class HmacHash;

#define HASH_IPP(tag)	template<> class Hash<HASH_##tag> \
						{	public: \
							static const int HASHSIZE = _details::_HashSize<HASH_##tag>::size; \
							protected:	BYTE	m_Context[_details::_get_hash_context_size<HASH_##tag>::Result]; \
							public:		Hash(){ int sz; IPPCALL(ipps##tag##GetSize)(&sz); ASSERT(sz <= sizeof(m_Context)); Reset(); } \
										void Reset(){ IPPCALL(ipps##tag##Init)((Ipps##tag##State*)m_Context); } \
										template<typename T> void Update(const T& x){ Update(&x, sizeof(x)); } \
										void Update(LPCVOID data, UINT size){ IPPCALL(ipps##tag##Update)((LPCBYTE)data,size,(Ipps##tag##State*)m_Context); } \
										void Finalize(LPVOID HashValue){ IPPCALL(ipps##tag##Final)((LPBYTE)HashValue,(Ipps##tag##State*)m_Context); } \
										void Calculate(LPCVOID data, UINT size, LPVOID HashValue){ Reset(); Update(data, size); Finalize(HashValue); } \
						}; \
						template<> class HmacHash<HASH_##tag> \
						{	public: \
							static const int HASHSIZE = _details::_HashSize<HASH_##tag>::size; \
							protected:	BYTE	m_Context[_details::_get_hmachash_context_size<HASH_##tag>::Result]; \
							public:		HmacHash(LPCVOID key, UINT key_len){ int sz; IPPCALL(ippsHMAC##tag##GetSize)(&sz); ASSERT(sz <= sizeof(m_Context)); Reset(key, key_len); } \
										void Reset(LPCVOID key, UINT key_len){ IPPCALL(ippsHMAC##tag##Init)((LPCBYTE)key, key_len, (IppsHMAC##tag##State*)m_Context); } \
										void Update(const rt::String_Ref& x){ Update(x.Begin(), x.GetLength()); } \
										void Update(LPCVOID data, UINT size){ IPPCALL(ippsHMAC##tag##Update)((LPCBYTE)data,size,(IppsHMAC##tag##State*)m_Context); } \
										void Finalize(LPVOID HashValue){ IPPCALL(ippsHMAC##tag##Final)((LPBYTE)HashValue,_details::_HashSize<HASH_##tag>::size,(IppsHMAC##tag##State*)m_Context); } \
										void Calculate(LPCVOID message, UINT message_len, LPVOID HashValue){ Update(message, message_len); Finalize(HashValue); } \
						}; \

		HASH_IPP(MD5)
		HASH_IPP(SHA1)
		HASH_IPP(SHA224)
		HASH_IPP(SHA256)
		HASH_IPP(SHA384)
		HASH_IPP(SHA512)

#undef HASH_IPP

template<> class Hash<HASH_CRC32>
{	
protected:
	Ipp32u	m_Context;
public:
	Hash(){ Reset(); }
	void Reset(){ m_Context = 0; }//~((DWORD)0); }
	template<typename T> void Update(const T& x){ Update(&x, sizeof(x)); }
	void Update(LPCVOID data, UINT size){ IPPCALL(ippsCRC32_8u)((LPCBYTE)data, size, &m_Context); }
	void Finalize(LPVOID HashValue){ *((Ipp32u*)HashValue) = m_Context; IPPCALL(ippsSwapBytes_32u_I)((Ipp32u*)HashValue, 1); }
	void Calculate(LPCVOID data, UINT size, LPVOID HashValue){ Reset(); Update(data, size); Finalize(HashValue); }
	DWORD Calculate(LPCVOID data, UINT size){ DWORD a; Calculate(data,size,&a); return a; }
};
} // namespace sec

namespace sec
{
namespace _details
{

template<UINT _METHOD>
struct	_cipher_spec;
	template<> struct _cipher_spec<CIPHER_AES128>
	{	static const int ContextSize = 583;
		static const int KEY_HASHER = HASH_MD5;
		static const int BlockSize = 128/8;
	};		
	template<> struct _cipher_spec<CIPHER_AES256>
	{	static const int ContextSize = 1031;
		static const int KEY_HASHER = HASH_SHA256; 
		static const int BlockSize = 256/8;
	};

} // namespace _details
} // namespace sec


namespace sec
{

namespace _details
{	template<int _LEN>
	struct CipherInitVec
	{	DWORD _Vec[_LEN/4];
		CipherInitVec(UINT random_seed){ Init(random_seed); }
		DWORD Init(DWORD s){ return ~(_Vec[_LEN/4-1] = s*((CipherInitVec<_LEN-4>&)*this).Init(s)); }
		operator LPCBYTE() const { return (LPCBYTE)_Vec; }
	};
	template<> struct CipherInitVec<0>
	{	DWORD Init(DWORD s){ return s; }
	};
};

template<UINT _METHOD = CIPHER_AES128>
class Cipher;

#define DEF_AES_CIPHER(_METHOD, MethodName) \
template<> class Cipher<_METHOD> \
{	protected: BYTE _Context[_details::_cipher_spec<_METHOD>::ContextSize]; \
public: static const UINT DataBlockSize = _details::_cipher_spec<_METHOD>::BlockSize; \
		static const UINT NativeKeySize = _details::_HashSize< _details::_cipher_spec<_METHOD>::KEY_HASHER>::size; \
	INLFUNC Cipher(){ int len=0; ASSERT(ippStsNoErr == IPPCALL(ipps##MethodName##GetSize)(&len) && len <= sizeof(_Context)); } \
	INLFUNC ~Cipher(){ rt::Zero(_Context); } \
	INLFUNC static void ComputeKey(LPVOID key, LPCVOID data, UINT size){ Hash<_details::_cipher_spec<_METHOD>::KEY_HASHER>().Calculate(data, size, key); } \
	INLFUNC void SetKey(LPCVOID key, UINT len) \
	{	BYTE hash[NativeKeySize]; \
		if(len != NativeKeySize){ ComputeKey(hash, key, len); key = hash; } \
		IPPCALL(ipps##MethodName##Init)((LPCBYTE)key, (IppsRijndaelKeyLength)(sizeof(hash)*8), (Ipps##MethodName##Spec*)_Context); \
	} \
	INLFUNC void Encrypt(LPCVOID pPlain, LPVOID pCrypt, UINT Len){ IPPCALL(ipps##MethodName##EncryptECB)((LPCBYTE)pPlain,(LPBYTE)pCrypt,(int)Len,(Ipps##MethodName##Spec*)_Context,IppsCPPaddingNONE); } \
	INLFUNC void Decrypt(LPCVOID pCrypt, LPVOID pPlain, UINT Len){ IPPCALL(ipps##MethodName##DecryptECB)((LPCBYTE)pCrypt,(LPBYTE)pPlain,(int)Len,(Ipps##MethodName##Spec*)_Context,IppsCPPaddingNONE); } \
	INLFUNC void EncryptBlockChained(LPCVOID pPlain, LPVOID pCrypt, UINT Len, UINT nonce) \
	{	_details::CipherInitVec<DataBlockSize> IV(nonce); \
		IPPCALL(ipps##MethodName##EncryptCBC)((LPCBYTE)pPlain,(LPBYTE)pCrypt,(int)Len,(Ipps##MethodName##Spec*)_Context,IV,IppsCPPaddingNONE); \
	} \
	INLFUNC void DecryptBlockChained(LPCVOID pCrypt, LPVOID pPlain, UINT Len, UINT nonce) \
	{	_details::CipherInitVec<DataBlockSize> IV(nonce); \
		IPPCALL(ipps##MethodName##DecryptCBC)((LPCBYTE)pCrypt,(LPBYTE)pPlain,(int)Len,(Ipps##MethodName##Spec*)_Context,IV,IppsCPPaddingNONE); \
	} \
};


DEF_AES_CIPHER(CIPHER_AES128, Rijndael128)
DEF_AES_CIPHER(CIPHER_AES256, Rijndael256)

#undef DEF_AES_CIPHER


class BigNum
{
	static const int	_BigNumSizeMax = 2*1024;
	// https://software.intel.com/en-us/ipp-crypto-reference-bignuminit (see security notes)
	IppsBigNumState *	_Ctx;
	UINT				_Len;
	UINT				_ActualBytes;
	void	_UpdateActualBytes()
			{	LPBYTE bin = NULL;
				int len;	IppsBigNumSGN sign;
				IPPVERIFY(ippsRef_BN(&sign, &len, (Ipp32u**)&bin, _Ctx));
				bin += len - 1;
				_ActualBytes = _Len;
				while(_ActualBytes>0 && *bin == 0){ bin--; _ActualBytes--; }
			}
	bool	_SetLength(UINT len)
			{
				ASSERT((len&0x3) == 0);
				if(len && len<=_Len)return true;
				_SafeFree32AL(_Ctx);
				_Len = 0;
		
				if(len>_Len)
				{	int sz; IPPVERIFY(ippsBigNumGetSize((len+3)/4, &sz));
					_Ctx = (IppsBigNumState *)rt::mem32AL::Malloc32AL(sz, true);
					if(_Ctx == NULL)return false;
					_Len = len;
					return ippStsNoErr == ippsBigNumInit((len+3)/4, _Ctx);
				}
				return true;
			}
	bool	_Set(LPCVOID data, UINT len, IppsBigNumSGN sign = ippBigNumPOS)
			{	
				if(_SetLength(len) &&
				   ippStsNoErr == ippsSet_BN(sign, len/4, (const Ipp32u*)data, _Ctx)
				)
				{	_UpdateActualBytes();
					return true;
				}
				else return false;
			}
public:
	BigNum(){ _Ctx = NULL; _Len = 0; }
	BigNum(int size){ _Ctx = NULL; _Len = 0; VERIFY(_SetLength((size+3)&0x7ffffffc)); }
	BigNum(const BigNum& x){ _Ctx = NULL; _Len = 0;	Set(x); }
	BigNum(const rt::String_Ref& str){ _Ctx = NULL; _Len = 0; Set(str); }
	~BigNum(){ _SafeFree32AL(_Ctx); }

	operator const IppsBigNumState *() const { return _Ctx; }
	operator IppsBigNumState *(){ return _Ctx; }

	bool	IsEmpty() const { return _Ctx == NULL || _Len == 0 || _ActualBytes == 0; }
	void	Empty(){ _SafeFree32AL(_Ctx); _Len = 0; _ActualBytes = 0; }
	bool	Set(const BigNum& x, bool negative = false)
			{
				if(!x.IsEmpty())
				{	LPBYTE bin = NULL;
					int len;	IppsBigNumSGN sign;
					IPPVERIFY(ippsRef_BN(&sign, &len, (Ipp32u**)&bin, x._Ctx));
					return _Set(bin, ((len+7)/8+3)&0x7ffffffc, (IppsBigNumSGN)(negative?(sign+1)&1:sign));
				}else Empty();
				return true;
			}
	bool	NegativeValue(const BigNum& x){ return Set(x, true); }
	bool	Set(const rt::String_Ref& str)
			{	
				LPCSTR p = str.Begin();
				UINT s = str.GetLength();
				if(s<2)return false;

				IppsBigNumSGN sign = ippBigNumPOS;
				if(p[0] == '-'){	p++;	s--;	sign = ippBigNumNEG; }
				else if(p[0] == '+'){	p++;	s--;	}

				if(p[0] != '0' || (p[1] != 'x' && p[1] != 'X'))return false;
				p+=2;	s-=2;
				if(s>_BigNumSizeMax || (s&1))return false;

				int buf_s = rt::max(4, (int)((s/2+3)&0x7ffffffc));
				LPBYTE bin = (LPBYTE)_alloca(buf_s);
				ASSERT(bin);
				*(DWORD*)bin = 0;

				if(!os::Base16Decode(bin + (buf_s - s/2), s/2, p, s))return false;
				for(int i=0; i<buf_s/2; i++)
					rt::Swap(bin[i], bin[buf_s-1-i]);

				return _Set(bin, buf_s, sign);
			}
	void	ToString(rt::String& out) const
			{	if(_Ctx && _Len)
				{
					IppsBigNumSGN sign;
					LPBYTE bin = NULL;
					int len;
					IPPVERIFY(ippsRef_BN(&sign, &len, (Ipp32u**)&bin, _Ctx));
					if(len<0){ out = "NaN"; return; }
					len = (len+7)/8;

					while(*bin == 0 && len>0){ bin++; len--; }

					LPSTR str;
					if(len && sign == ippBigNumNEG)
					{	out.SetLength(3 + len*2); out[0] = '-'; out[1] = '0'; out[2] = 'x';
						str = out.Begin() + 3;
					}
					else
					{	out.SetLength(2 + len*2); out[0] = '0'; out[1] = 'x';
						str = out.Begin() + 2;
					}

					for(SIZE_T i=0;i<len;i++)
					{
						int idx = len-1-i;
						int c1 = bin[idx]>>4;
						int c2 = bin[idx]&0xf;
						str[2*i+0] = (c1>9)?('a'+c1-10):('0'+c1);
						str[2*i+1] = (c2>9)?('a'+c2-10):('0'+c2);
					}
				}
				else{ out.SetLength(2); out[0] = '0'; out[1] = 'x'; }
			}
	bool	Add(const BigNum&a, const BigNum&b) // a + b
			{	
				if(a.IsEmpty() || b.IsEmpty())
				{	Empty();
					if(!a.IsEmpty())return Set(a);
					if(!b.IsEmpty())return Set(b);
					return true;
				}
				if(	_SetLength((rt::max(a._ActualBytes, b._ActualBytes)+4)&0x7ffffffc) &&
					ippStsNoErr == ippsAdd_BN(a._Ctx, b._Ctx, _Ctx)
				)
				{	_UpdateActualBytes();
					return true;
				}
				else return false;
			}
	bool	Subtract(const BigNum&a, const BigNum&b) // a - b
			{	
				if(a.IsEmpty() || b.IsEmpty())
				{	Empty();
					if(!a.IsEmpty())return Set(a);
					if(!b.IsEmpty())return Set(b, true);
					return true;
				}
				if(	_SetLength((rt::max(a._ActualBytes, b._ActualBytes)+4)&0x7ffffffc) &&
					ippStsNoErr == ippsSub_BN(a._Ctx, b._Ctx, _Ctx)
				)
				{	_UpdateActualBytes();
					return true;
				}
				else return false;
			}
	template<class t_Ostream> friend
	t_Ostream& operator<<(t_Ostream& Ostream, const BigNum& x){	rt::String out;	x.ToString(out); return Ostream<<out; }
};



enum GFp
{
	GFp_secp256k1 = 0,
	GFp_secp256r1
	// http://www.secg.org/sec2-v2.pdf
};

class ECGFp
{
	struct ECGFpParam
	{
		int			Bits;
		LPCSTR		Prime;
		LPCSTR		A, B;
		LPCSTR		Gx, Gy;
		LPCSTR		Order;
		int			Cofactor;
	};

	IppsECCPState*	_GFp;
	int				_PointCtxSize;
public:
	ECGFp(GFp x){ _GFp = NULL; SetParam(GetStandardParam(x)); }
	ECGFp(const ECGFpParam& param){ _GFp = NULL; SetParam(param); }
	~ECGFp(){ _SafeFree32AL(_GFp); }

	static const ECGFpParam&	
				GetStandardParam(GFp f);
	bool		SetParam(const ECGFpParam& param);
	bool		DerivePublicKey(const BigNum& sk);
};


} //  namespace sec

////////////////////////////////////////////////////////////
// Platform without IPP
#else
namespace sec
{
namespace _details
{
	template<int HASH_METHOD>
	struct _HashTrait;
		template<> struct _HashTrait<HASH_CRC32>{	typedef Botan::CRC32 type;	};
		template<> struct _HashTrait<HASH_MD5>{		typedef Botan::MD5 type;	};
		template<> struct _HashTrait<HASH_SHA1>{	typedef Botan::SHA_160 type;};
		template<> struct _HashTrait<HASH_SHA224>{	typedef Botan::SHA_224 type;};
		template<> struct _HashTrait<HASH_SHA256>{	typedef Botan::SHA_256 type;};
		template<> struct _HashTrait<HASH_SHA384>{	typedef Botan::SHA_384 type;};
		template<> struct _HashTrait<HASH_SHA512>{	typedef Botan::SHA_512 type;};
}
} // namespace sec

namespace sec
{

template<int HASH_METHOD = HASH_MD5>
class Hash
{
public:
	static const int HASHSIZE = _details::_HashSize<HASH_METHOD>::size;
	typedef typename _details::_HashTrait<HASH_METHOD>::type HashType;
protected:
	HashType*	Hasher;
public:
	INLFUNC Hash()
	{	Hasher = NULL;
		try
		{	Hasher = new HashType;
			ASSERT(Hasher);
		}catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }
	}
	INLFUNC ~Hash()
	{	try{ _SafeDel(Hasher); return; }
		catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }
	}
	INLFUNC void Reset()
	{	try{ Hasher->clear(); return; }
		catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }
	}
	INLFUNC void Update(LPCVOID data, SIZE_T size)
	{	try{ Hasher->update((LPCBYTE)data, size); return; }
		catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }
	}
	INLFUNC void Finalize(LPVOID HashValue)
	{	try{ Hasher->final((LPBYTE)HashValue); return; }
		catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }			
	}
	INLFUNC void Calculate(LPCVOID data, SIZE_T size, LPVOID HashValue)
	{	try
		{	Hasher->clear();
			Hasher->update((LPCBYTE)data, size);
			Hasher->final((LPBYTE)HashValue);
		}
		catch(std::exception& e){ _LOG_EXPCEPTION(e.what()); }
	}
	INLFUNC void Calculate(LPCVOID data, SIZE_T size, rt::String& hash_hex)
	{
		BYTE hash[HASHSIZE];
		Calculate(data,size,hash);

		hash_hex.SetLength(HASHSIZE*2);
		os::Base16Encode(hash_hex.Begin(), hash, HASHSIZE);
	}
};


} // namespace sec

#endif // #ifdef PLATFORM_INTEL_IPP_SUPPORT


namespace sec
{

class TLS: protected Botan::TLS::Callbacks
{
	rt::BufferEx<BYTE>		__RecvBuf;
	UINT					__RecvAteSize;
	
protected:
	virtual void tls_emit_data(const uint8_t data[], size_t size);
	virtual void tls_record_received(Botan::u64bit seq_no, const uint8_t data[], size_t size);
	virtual void tls_alert(Botan::TLS::Alert alert){}
	virtual bool tls_session_established(const Botan::TLS::Session& session);
	virtual void tls_session_activated(){};
	virtual void tls_verify_cert_chain(	const std::vector<Botan::X509_Certificate>& cert_chain,
										const std::vector<std::shared_ptr<const Botan::OCSP::Response>>& ocsp_responses,
										const std::vector<Botan::Certificate_Store*>& trusted_roots,
										Botan::Usage_Type usage,
										const std::string& hostname,
										const Botan::TLS::Policy& policy);

	rt::Buffer<BYTE>		_ExpectedPubKey;

public:
	typedef bool (*TLS_SocketRecv_CB)(LPVOID buf, UINT buf_size, UINT&read, LPVOID cookie);
	typedef bool (*TLS_SocketSend_CB)(LPCVOID buf, UINT buf_size, LPVOID cookie);

protected:
	bool	_NoErrorOccurred;
	bool	_CertificateError;
	bool	_Init;
	BYTE	_BotanRngObject[sizeof(Botan::AutoSeeded_RNG)];
	BYTE	_BotanTLSObject[sizeof(Botan::TLS::Client)];

	TLS_SocketRecv_CB	_RecvCB;
	TLS_SocketSend_CB	_SendCB;
	LPVOID				_CB_Cookie;

public:
	TLS(TLS_SocketRecv_CB recv_cb, TLS_SocketSend_CB send_cb, LPVOID cookie = NULL);
	~TLS();
	bool Recv(LPVOID buf, UINT buf_size, UINT&read);
	bool Send(LPCVOID buf, UINT buf_size);
	bool Create();
	void Destroy();
	bool IsCreate() const { return _Init; }
	void SetExpectedServerPublicKey(LPCVOID data = 0, UINT data_size = 0);
	bool HasCertificateError() const { return _CertificateError; }
};

INLFUNC void Randomize(LPVOID p, UINT len)
{
	Botan::System_RNG().randomize((LPBYTE)p, len);
}

} // namespace sec
