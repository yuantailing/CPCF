#include "botan.h"


#pragma warning(disable:4267)

#if defined(PLATFORM_WIN)
	#ifdef PLATFORM_64BIT
		#include "./platforms/win_x64/botan_all.cpp"
		#include "./platforms/win_x64/botan_all_aesni.cpp"
		#include "./platforms/win_x64/botan_all_ssse3.cpp"
	#else
		#include "./platforms/win_x86/botan_all.cpp"
		#include "./platforms/win_x86/botan_all_aesni.cpp"
		#include "./platforms/win_x86/botan_all_sse2.cpp"
		#include "./platforms/win_x86/botan_all_ssse3.cpp"
	#endif
#elif defined(PLATFORM_ANDROID)
	#if defined (__mips__)
		#include "./platforms/botan_ndk_mips.cpp"
	#elif defined (__arm__)
		#include "./platforms/botan_ndk_arm.cpp"
	#elif defined (__i386__)
		#include "./platforms/botan_ndk_x86.cpp"
	#else
		#error unknown andriod platform
	#endif
#elif defined(PLATFORM_MAC)
	#ifdef PLATFORM_64BIT
        #include "./platforms/mac_x64/botan_all.cpp"
        #include "./platforms/mac_x64/botan_all_ssse3.cpp"
	#else
        #include "./platforms/mac_x86/botan_all.cpp"
        #include "./platforms/mac_x86/botan_all_sse2.cpp"
        #include "./platforms/mac_x86/botan_all_ssse3.cpp"
    #endif
#elif defined(PLATFORM_IOS)
    #ifdef PLATFORM_64BIT
        #include "./platforms/ios_64b/botan_all.cpp"
    #else
        #include "./platforms/ios_32b/botan_all.cpp"
    #endif
#elif defined(PLATFORM_LINUX)	
	#ifdef PLATFORM_64BIT
		#include "./platforms/linux_x64/botan_all.cpp"
		#include "./platforms/linux_x64/botan_all_aesni.cpp"
		#include "./platforms/linux_x64/botan_all_ssse3.cpp"
	#else
		#include "./platforms/linux_x86/botan_all.cpp"
		#include "./platforms/linux_x86/botan_all_aesni.cpp"
		#include "./platforms/linux_x86/botan_all_sse2.cpp"
		#include "./platforms/linux_x86/botan_all_ssse3.cpp"
	#endif
#else
	#error unknown platform
#endif

namespace sec
{

const ECGFp::ECGFpParam& ECGFp::GetStandardParam(GFp f)
{
	static const ECGFpParam standards[] = 
	{
		// GFp_secp256k1
		{	256,
			"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEFFFFFC2F",
			"0x", 
			"0x07",
			// Gxy
			"0x79BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798",
			"0x483ADA7726A3C4655DA4FBFC0E1108A8FD17B448A68554199C47D08FFB10D4B8",
			"0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFEBAAEDCE6AF48A03BBFD25E8CD0364141",			1
		},
		// GFp_secp256r1
		{	256,
			"0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFF",
			"0xFFFFFFFF00000001000000000000000000000000FFFFFFFFFFFFFFFFFFFFFFFC",
			"0x5AC635D8AA3A93E7B3EBBD55769886BC651D06B0CC53B0F63BCE3C3E27D2604B",
			// Gxy, S = C49D3608 86E70493 6A6678E1 139D26B7 819F7E90
			"0x6B17D1F2E12C4247F8BCE6E563A440F277037D812DEB33A0F4A13945D898C296",
			"0x4FE342E2FE1A7F9B8EE7EB4A7C0F9E162BCE33576B315ECECBB6406837BF51F5",
			"0xFFFFFFFF00000000FFFFFFFFFFFFFFFFBCE6FAADA7179E84F3B9CAC2FC632551",
			1
		}
	};

	ASSERT(f>=0 && f<sizeofArray(standards));
	return standards[f];
}

bool ECGFp::SetParam(const ECGFp::ECGFpParam& param)
{
	_SafeFree32AL(_GFp);

	int sz;
	ippsECCPGetSize(param.Bits, &sz);
	ippsECCPPointGetSize(param.Bits, &_PointCtxSize);

	_GFp = (IppsECCPState*)rt::mem32AL::Malloc32AL(sz, true); 

	IPPVERIFY(ippsECCPInit(param.Bits, _GFp));

	BigNum p,a,b,gx,gy,order;

	if(	p.Set(param.Prime) &&
		a.Set(param.A) && b.Set(param.B) &&
		gx.Set(param.Gx) && gy.Set(param.Gy) &&
		order.Set(param.Order)
	){}
	else return false;

	return ippStsNoErr == IPPCALL(ippsECCPSet)(p, a, b, gx, gy, order, param.Cofactor, _GFp);
}

bool ECGFp::DerivePublicKey(const BigNum& sk)
{
	IppsECCPPointState* ps = (IppsECCPPointState*)_alloca(_PointCtxSize);
	
	BigNum x(32),y(32);

	IppStatus r,s;

	if(	ippStsNoErr == (r = ippsECCPPublicKey(sk, ps, _GFp)) &&
		ippStsNoErr == (s = ippsECCPGetPoint(x, y, ps, _GFp))
	)
	{
		_LOG(x<<','<<y);
	}

	_LOG(x<<','<<y);

	return true;
}


} // namespace sec


namespace sec
{

struct HttpsPolicy: public Botan::TLS::Policy
{
	std::vector<std::string> allowed_key_exchange_methods() const override
	{
		std::vector<std::string> ke = Botan::TLS::Policy::allowed_key_exchange_methods();
		ke.push_back("RSA");
		return ke;
	}
};

Botan::TLS::Session_Manager_Noop	g_TlsSessionManager;
Botan::Credentials_Manager			g_TlsCredentialsManager;
HttpsPolicy							g_TlsPolicy;

TLS::TLS(TLS_SocketRecv_CB recv_cb, TLS_SocketSend_CB send_cb, LPVOID cookie)
{
	_RecvCB = recv_cb;
	_SendCB = send_cb;
	_CB_Cookie = cookie;
	_Init = false;
	_NoErrorOccurred = true;
	_CertificateError = false;
	__RecvAteSize = 0;
}

TLS::~TLS()
{
	Destroy();
}

void TLS::tls_emit_data(const uint8_t data[], size_t size)
{
	if(_NoErrorOccurred)
		_NoErrorOccurred = _SendCB(data, (UINT)size, _CB_Cookie);
}

void TLS::tls_record_received(Botan::u64bit seq_no, const uint8_t data[], size_t size)
{
	if(_NoErrorOccurred)
		__RecvBuf.push_back(data, size);
}

bool TLS::tls_session_established(const Botan::TLS::Session& session)
{
	return false;
}

void TLS::tls_verify_cert_chain(	const std::vector<Botan::X509_Certificate>& cert_chain,
									const std::vector<std::shared_ptr<const Botan::OCSP::Response>>& ocsp_responses,
									const std::vector<Botan::Certificate_Store*>& trusted_roots,
									Botan::Usage_Type usage,
									const std::string& hostname,
									const Botan::TLS::Policy& policy
)
{	
	if(_ExpectedPubKey.GetSize())
	{
		rt::Buffer_Ref<BYTE> pk(cert_chain[cert_chain.size()-1].subject_public_key_bits().data(), cert_chain[cert_chain.size()-1].subject_public_key_bits().size());
		if(_ExpectedPubKey != pk)
		{	_NoErrorOccurred = false;
			_CertificateError = true;
		}
	}
}


bool TLS::Recv(LPVOID buf, UINT buf_size, UINT&read)
{
	ASSERT(_Init);

	if(__RecvBuf.GetSize())
	{
		read = rt::min(buf_size, (UINT)(__RecvBuf.GetSize() - __RecvAteSize));
		memcpy(buf, &__RecvBuf[__RecvAteSize], read);
		__RecvAteSize += read;
		if(__RecvAteSize == __RecvBuf.GetSize())
		{	__RecvBuf.ChangeSize(0);
			__RecvAteSize = 0;
		}
		return true;
	}
	
	BYTE recv_buf[2048];

	while(__RecvBuf.GetSize() == 0)
	{
		UINT recv_size = 0;
		if(!_RecvCB(recv_buf, sizeof(recv_buf), recv_size, _CB_Cookie))
			return _NoErrorOccurred = false;

		try
		{	((Botan::TLS::Client*)_BotanTLSObject)->received_data((Botan::byte*)recv_buf, recv_size);
		}
		catch(...){ return false; }
	}

	VERIFY(Recv(buf, buf_size, read));
	return true;
}


bool TLS::Send(LPCVOID buf, UINT buf_size)
{
	ASSERT(_Init);
	if(_NoErrorOccurred)
	{	try
		{	((Botan::TLS::Client*)_BotanTLSObject)->send((const Botan::byte*)buf, buf_size);
		}
		catch(...){ return false; }
	}
	return _NoErrorOccurred;
}

void TLS::SetExpectedServerPublicKey(LPCVOID data, UINT data_size)
{
	_ExpectedPubKey.SetSize(data_size);
	if(data_size)
		_ExpectedPubKey.CopyFrom((LPCBYTE)data);
}

bool TLS::Create()
{
	ASSERT(_Init == false);

	_NoErrorOccurred = true;
	_CertificateError = false;

	try
	{
		new (_BotanRngObject)Botan::AutoSeeded_RNG();
		new (_BotanTLSObject)Botan::TLS::Client(*this, 
												g_TlsSessionManager, 
												g_TlsCredentialsManager, 
												g_TlsPolicy, 
												*((Botan::AutoSeeded_RNG*)_BotanRngObject));
		
		_Init = true;

		// Start hand-shaking
		{	BYTE recv_buf[2048];
			UINT recv_size = 0;

			try
			{	
				for(;;)
				{
					if(!_RecvCB(recv_buf, sizeof(recv_buf), recv_size, _CB_Cookie))
						return _NoErrorOccurred = false;

					if(	((Botan::TLS::Client*)_BotanTLSObject)->received_data((Botan::byte*)recv_buf, recv_size) == 0 &&
						((Botan::TLS::Client*)_BotanTLSObject)->is_active()
					)break;
				}
			}
			catch(...){ return false; }
		}

		return true;
	}
	catch(Botan::Stream_IO_Error &e)
	{
		if (strstr(e.what(), "Server certificate is not valid") != NULL)
			_CertificateError = true;
	}
	catch(...){}

	return false;
}

void TLS::Destroy()
{
	if(_Init)
	{	try
		{	((Botan::TLS::Client*)_BotanTLSObject)->~Client();
			((Botan::AutoSeeded_RNG*)_BotanRngObject)->~AutoSeeded_RNG();
		}
		catch(...){}
		_Init = false;
		__RecvBuf.ChangeSize(0);
		__RecvAteSize = 0;
	}
}


} // namespace sec
