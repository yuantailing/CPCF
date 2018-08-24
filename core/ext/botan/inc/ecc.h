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


#include "botan_inc.h"
#include "datablock.h"


namespace sec
{

enum _tagECCMethods
{
	ECDSA_SECP256K1 = 1

//      case 23:
//         return "secp256r1";
//      case 24:
//         return "secp384r1";
//      case 25:
//         return "secp521r1";
//      case 26:
//         return "brainpool256r1";
//      case 27:
//         return "brainpool384r1";
//      case 28:
//         return "brainpool512r1";
//
//#if defined(BOTAN_HAS_CURVE_25519)
//      case 29:
//         return "x25519";
//
};

enum _tagSignatureType
{
	EMSA1_SHA_160	= 0x1000 + 20,
	EMSA1_SHA_256	= 0x2000 + 32,
	EMSA1_SHA_384	= 0x3000 + 48,
	EMSA1_SHA_512	= 0x4000 + 64,
};

template<int CRYTOMETHOD>
struct ECC;
	template<> struct ECC<ECDSA_SECP256K1>
	{
		static const int KeySize = 32;
		typedef Botan::ECDSA_PrivateKey		PrivateKey;
		typedef Botan::ECDSA_PublicKey		PublicKey;
		static LPCSTR	ECGroupName(){ return "secp256k1"; }
	};

template<int CRYTOMETHOD>
class Keypair: public DataBlock<ECC<CRYTOMETHOD>::KeySize*2, true>
{
	typedef DataBlock<ECC<CRYTOMETHOD>::KeySize*2, true> _SC;
public:
	static const int KeySize = ECC<CRYTOMETHOD>::KeySize;
	Keypair() = default;
	Keypair(const DataBlock<KeySize*2>& x){ _SC::From(x); }
	Keypair(const rt::String_Ref& str){ _SC::FromBase32(str); }
	const DataBlock<KeySize, false>& PK() const { return *(DataBlock<KeySize, false>*)&_SC::Bytes[KeySize]; }
	const DataBlock<KeySize, true>& SK() const { return *(DataBlock<KeySize, true>*)&_SC::Bytes[0]; }
	bool  Generate(Botan::RandomNumberGenerator* pRNG = NULL)
	{	try{
			rt::ObjectPlaceHolder<Botan::System_RNG>	SysRNG;
			if(pRNG == NULL){ pRNG = SysRNG; }
			typename ECC<CRYTOMETHOD>::PrivateKey sk(*pRNG, Botan::EC_Group(ECC<CRYTOMETHOD>::ECGroupName()));
			memcpy(_SC::Bytes, sk.private_value().get_word_vector().data(), KeySize);
			memcpy(_SC::Bytes + KeySize, sk.public_point().get_affine_x().get_word_vector().data(), KeySize);
			return true;
		}catch(...){ return false; }
	}
	bool  SetPrivateKey(const DataBlock<KeySize>& k)
	{	
		try{
			static Botan::AutoSeeded_RNG rng;
			typename ECC<CRYTOMETHOD>::PrivateKey sk(rng, Botan::EC_Group(ECC<CRYTOMETHOD>::ECGroupName()), sec::BigInt(k));
			memcpy(_SC::Bytes, sk.private_value().get_word_vector().data(), KeySize);
			memcpy(_SC::Bytes + KeySize, sk.public_point().get_affine_x().get_word_vector().data(), KeySize);
			return true;
		}catch(...){ return false; }
	}

	//void ToString(rt::String& str) const
	//{
	//	str = rt::SS("OID = ") + _SK.algorithm_identifier().oid.as_string().c_str();
	//	str += rt::SS("OID-PARAM = ") + rt::tos::StdPodVec(_SK.algorithm_identifier().parameters);
	//	str += rt::SS("\nPK-x = ") + rt::tos::BigInt(_SK.public_point().get_affine_x());
	//	str += rt::SS("\nPK-y = ") + rt::tos::BigInt(_SK.public_point().get_affine_y());

	//	str += rt::SS("\nPK = ") + rt::tos::StdPodVec(_SK.x509_subject_public_key());

	//	str += rt::SS("\nSK = ") + rt::tos::BigInt(_SK.private_value());
	//}
};

template<int CRYTOMETHOD, int SIG_TYPE = EMSA1_SHA_256>
class Signer
{
	static const int KeySize = ECC<CRYTOMETHOD>::KeySize;
	static const int SignatureSize = SIG_TYPE&0xfff;

	struct _State
	{
		typename ECC<CRYTOMETHOD>::PrivateKey _SK;
		bool				_SK_init;
		Botan::PK_Signer	_Signer;
		bool				_Signer_init;
	};
	_State	_S;
	Botan::RandomNumberGenerator*	_pRNG;
	bool							_bExternalRNG;
	void							_ClearState()
	{	if(_S._SK_init)_S._SK.~PrivateKey();
		if(_S._Signer_init)_S._Signer.~PK_Signer();
	}
public:
	Signer(Botan::RandomNumberGenerator* rng = NULL)
	{	if(rng){ _pRNG = rng; _bExternalRNG = true; }
		else{ rng = new Botan::System_RNG(); _bExternalRNG = false; }
		_S._SK_init = false;
		_S._Signer_init = false;
	}
	~Signer(){ _ClearState(); if(!_bExternalRNG)_SafeDel(_pRNG); }
	bool SetPrivateKey(const DataBlock<KeySize>& sk)
	{	_ClearState();
		try{
			_S._SK_init = true;
			new (&_S._SK) typename ECC<CRYTOMETHOD>::PrivateKey(_pRNG, Botan::EC_Group(ECC<CRYTOMETHOD>::ECGroupName()), sec::BigInt(sk));
		}catch(...){ return false; }
		try{
			_S._Signer_init = true;
			new (&_S._Signer) Botan::PK_Signer(_S._SK, *_pRNG, rt::SS("EMSA1(SHA-") + SignatureSize*8 + ')');
		}catch(...){ return false; }
		return true;
	}
	void Sign(LPCVOID data, UINT size, DataBlock<SignatureSize, false>& signature_out)
	{	ASSERT(_S._Signer_init);
		try{
			auto sig = _S._Signer.sign_message(data, size, *_pRNG);
			ASSERT(sig.size() == SignatureSize);
			signature_out = *(DataBlock<SignatureSize, false>*)sig.data();
		}catch(...){ ASSERT(0); }
	}
};

template<int CRYTOMETHOD, int SIG_TYPE = EMSA1_SHA_256>
class SignatureVerifier
{
	static const int KeySize = ECC<CRYTOMETHOD>::KeySize;
	static const int SignatureSize = SIG_TYPE&0xfff;

	struct _State
	{
		typename ECC<CRYTOMETHOD>::PrivateKey _SK;
		bool				_SK_init;
		Botan::PK_Verifier	_Verifier;
		bool				_Verifier_init;
	};
	_State	_S;
	void							_ClearState()
	{	if(_S._SK_init)_S._SK.~PrivateKey();
		if(_S._Verifier_init)_S._Verifier.~PK_Verifier();
	}
public:
	SignatureVerifier()
	{	_S._SK_init = false;
		_S._Verifier_init = false;
	}
	~SignatureVerifier(){ _ClearState(); }
}; 


} // namespace sec
