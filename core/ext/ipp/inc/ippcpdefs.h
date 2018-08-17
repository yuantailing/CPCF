/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2012 Intel Corporation. All Rights Reserved.
//
//              Intel(R) Integrated Performance Primitives
//                  Cryptographic Primitives (ippCP) definitions.
//
*/
#ifndef __IPPCPDEFS_H__
#define __IPPCPDEFS_H__

#ifdef __cplusplus
extern "C" {
#endif

#if !defined( _OWN_BLDPCS )

/*
// =========================================================
// Symmetric Ciphers
// =========================================================
*/
typedef enum {
   ippPaddingNONE  = 0, NONE  = 0, IppsCPPaddingNONE  = 0,
   ippPaddingPKCS7 = 1, PKCS7 = 1, IppsCPPaddingPKCS7 = 1,
   ippPaddingZEROS = 2, ZEROS = 2, IppsCPPaddingZEROS = 2
} IppsPadding, IppsCPPadding;

typedef struct _cpDES         IppsDESSpec;
typedef struct _cpRijndael128 IppsRijndael128Spec;
typedef struct _cpRijndael192 IppsRijndael192Spec;
typedef struct _cpRijndael256 IppsRijndael256Spec;
typedef struct _cpRC5_64      IppsARCFive64Spec;
typedef struct _cpRC5_128     IppsARCFive128Spec;

/* TDES */
#define  DES_BLOCKSIZE  (64)  /* cipher blocksize (bits) */
#define TDES_BLOCKSIZE  DES_BLOCKSIZE

#define  DES_KEYSIZE    (64) /*     cipher keysize (bits) */
#define TDES_KEYSIZE    DES_KEYSIZE

/* Rijndael */
#define IPP_RIJNDAEL128_BLOCK_BITSIZE (128)  /* cipher blocksizes (bits) */
#define IPP_RIJNDAEL192_BLOCK_BITSIZE (192)
#define IPP_RIJNDAEL256_BLOCK_BITSIZE (256)

#define RIJNDAEL128_BLOCKSIZE    IPP_RIJNDAEL128_BLOCK_BITSIZE /* obsolete */
#define RIJNDAEL192_BLOCKSIZE    IPP_RIJNDAEL192_BLOCK_BITSIZE
#define RIJNDAEL256_BLOCKSIZE    IPP_RIJNDAEL256_BLOCK_BITSIZE

typedef enum {
   ippRijndaelKey128 = 128, IppsRijndaelKey128 = 128, /* 128-bit key */
   ippRijndaelKey192 = 192, IppsRijndaelKey192 = 192, /* 192-bit key */
   ippRijndaelKey256 = 256, IppsRijndaelKey256 = 256  /* 256-bit key */
} IppsRijndaelKeyLength;

/* AES based  authentication & confidence */
typedef struct _cpRijndael128CCM IppsRijndael128CCMState;
typedef struct _cpRijndael128GCM IppsRijndael128GCMState;

/* AES-GCM available behaviour */
typedef enum {
   ippAESGCMdefault,
   ippAESGCMsafe,
   ippAESGCMtable2K
} IppsAESGCMbehaviour, IppAESGCMbehaviour;

/* 64--bit data block size RC5 */
#define IPP_ARCFIVE_64_BLOCK_BITSIZE   (64)  /* cipher blockizes (bits) */
#define IPP_ARCFIVE_64_KEYMAX_SIZE     (255) /* cipher key max len (bytes) */
#define IPP_ARCFIVE_64_RNDMAX_NUM      (255) /* max num of rounds */

#define ARCFIVE_64_BLOCKSIZE  IPP_ARCFIVE_64_BLOCK_BITSIZE  /* obsolete */
#define ARCFIVE_64_KEY_MAX    IPP_ARCFIVE_64_KEYMAX_SIZE
#define ARCFIVE_64_ROUNDS_MAX IPP_ARCFIVE_64_RNDMAX_NUM

/* 128-bit data block size RC5 */
#define IPP_ARCFIVE_128_BLOCK_BITSIZE  (128) /* cipher blocksize (bits) */
#define IPP_ARCFIVE_128_KEYMAX_SIZE    (255) /* cipher key max len (bytes) */
#define IPP_ARCFIVE_128_RNDMAX_NUM     (255) /* max num of rounrs */

#define ARCFIVE_128_BLOCKSIZE    IPP_ARCFIVE_128_BLOCK_BITSIZE /* obsolete */
#define ARCFIVE_128_KEY_MAX      IPP_ARCFIVE_128_KEYMAX_SIZE
#define ARCFIVE_128_ROUNDS_MAX   IPP_ARCFIVE_128_RNDMAX_NUM

/*
// =========================================================
// ARCFOUR Stream Cipher
// =========================================================
*/
typedef struct _cpARCfour  IppsARCFourState;

#define IPP_ARCFOUR_KEYMAX_SIZE  (256)  /* max key length (bytes) */
#define MAX_ARCFOUR_KEY_LEN   IPP_ARCFOUR_KEYMAX_SIZE /* obsolete */

/*
// =========================================================
// One-Way Hash Functions
// =========================================================
*/
typedef struct _cpSHA1     IppsSHA1State;
typedef struct _cpSHA224   IppsSHA224State;
typedef struct _cpSHA256   IppsSHA256State;
typedef struct _cpSHA384   IppsSHA384State;
typedef struct _cpSHA512   IppsSHA512State;
typedef struct _cpMD5      IppsMD5State;

/* MGF */
typedef IppStatus (__STDCALL *IppMGF)(const Ipp8u* pSeed, int seedLen, Ipp8u* pMask, int maskLen);
/* HASH function */
typedef IppStatus (__STDCALL *IppHASH)(const Ipp8u* pMsg, int len, Ipp8u* pMD);

#define   IPP_SHA1_DIGEST_BITSIZE  160   /* digest size (bits) */
#define IPP_SHA256_DIGEST_BITSIZE  256
#define IPP_SHA224_DIGEST_BITSIZE  224
#define IPP_SHA384_DIGEST_BITSIZE  384
#define IPP_SHA512_DIGEST_BITSIZE  512
#define    IPP_MD5_DIGEST_BITSIZE  128

                                    /* obsolete:          */
#define   SHA1_DIGEST_BITSIZE    IPP_SHA1_DIGEST_BITSIZE   /* digest size (bits) */
#define SHA256_DIGEST_BITSIZE    IPP_SHA256_DIGEST_BITSIZE
#define SHA224_DIGEST_BITSIZE    IPP_SHA224_DIGEST_BITSIZE
#define SHA384_DIGEST_BITSIZE    IPP_SHA384_DIGEST_BITSIZE
#define SHA512_DIGEST_BITSIZE    IPP_SHA512_DIGEST_BITSIZE
#define    MD5_DIGEST_BITSIZE    IPP_MD5_DIGEST_BITSIZE
                                    /* obsolete:          */
#define   SHA1_DIGEST_LENGTH     IPP_SHA1_DIGEST_BITSIZE   /* digest size (bits) */
#define SHA256_DIGEST_LENGTH     IPP_SHA256_DIGEST_BITSIZE
#define SHA224_DIGEST_LENGTH     IPP_SHA224_DIGEST_BITSIZE
#define SHA384_DIGEST_LENGTH     IPP_SHA384_DIGEST_BITSIZE
#define SHA512_DIGEST_LENGTH     IPP_SHA512_DIGEST_BITSIZE
#define    MD5_DIGEST_LENGTH     IPP_MD5_DIGEST_BITSIZE

/*
// =========================================================
// Keyed-Hash Message Authentication Codes
// =========================================================
*/
typedef struct _cpHMACSHA1    IppsHMACSHA1State;
typedef struct _cpHMACSHA256  IppsHMACSHA256State;
typedef struct _cpHMACSHA224  IppsHMACSHA224State;
typedef struct _cpHMACSHA384  IppsHMACSHA384State;
typedef struct _cpHMACSHA512  IppsHMACSHA512State;
typedef struct _cpHMACMD5     IppsHMACMD5State;

/*
// =========================================================
// Data Authentication Codes
// =========================================================
*/
typedef struct _cpDAATDES           IppsDAATDESState;
typedef struct _cpDAARijndael128    IppsDAARijndael128State;
typedef struct _cpDAARijndael192    IppsDAARijndael192State;
typedef struct _cpDAARijndael256    IppsDAARijndael256State;
typedef struct _cpXCBCRijndael128   IppsXCBCRijndael128State;
typedef struct _cpCMACRijndael128   IppsCMACRijndael128State;

/*
// =========================================================
// Big Number Integer Arithmetic
// =========================================================
*/
typedef enum {
   ippBigNumNEG = 0, IppsBigNumNEG = 0,
   ippBigNumPOS = 1, IppsBigNumPOS = 1
} IppsBigNumSGN;

typedef enum {
   ippBinaryMethod   = 0, IppsBinaryMethod = 0,
   ippSlidingWindows = 1, IppsSlidingWindows = 1
} IppsExpMethod;

typedef struct _cpBigNum      IppsBigNumState;
typedef struct _cpMontgomery  IppsMontState;
typedef struct _cpPRNG        IppsPRNGState;
typedef struct _cpPrime       IppsPrimeState;

/*  External Bit Supplier */
typedef IppStatus (__STDCALL *IppBitSupplier)(Ipp32u* pRand, int nBits, void* pEbsParams);

#define IPP_IS_EQ (0)
#define IPP_IS_NE (1)
#define IPP_IS_GT (2)
#define IPP_IS_LT (3)
#define IPP_IS_NA (4)

#define IPP_IS_PRIME       (5)
#define IPP_IS_COMPOSITE   (6)

#define IPP_IS_VALID       (7)
#define IPP_IS_INVALID     (8)
#define IPP_IS_INCOMPLETE  (9)
#define IPP_IS_ATINFINITY  (10)

#define IS_ZERO            IPP_IS_EQ
#define GREATER_THAN_ZERO  IPP_IS_GT
#define LESS_THAN_ZERO     IPP_IS_LT
#define IS_PRIME           IPP_IS_PRIME
#define IS_COMPOSITE       IPP_IS_COMPOSITE
#define IS_VALID_KEY       IPP_IS_VALID
#define IS_INVALID_KEY     IPP_IS_INVALID
#define IS_INCOMPLETED_KEY IPP_IS_INCOMPLETE

/*
// =========================================================
// RSA Cryptography
// =========================================================
*/
typedef struct _cpRSA IppsRSAState;

/* key types */
typedef enum {
   ippRSApublic  = 0x20000000, IppRSApublic  = 0x20000000,
   ippRSAprivate = 0x40000000, IppRSAprivate = 0x40000000
} IppRSAKeyType;

/* key component's tag */
typedef enum {
   ippRSAkeyN    = 0x01,  IppRSAkeyN    = 0x01,
   ippRSAkeyE    = 0x02,  IppRSAkeyE    = 0x02,
   ippRSAkeyD    = 0x04,  IppRSAkeyD    = 0x04,
   ippRSAkeyP    = 0x08,  IppRSAkeyP    = 0x08,
   ippRSAkeyQ    = 0x10,  IppRSAkeyQ    = 0x10,
   ippRSAkeyDp   = 0x20,  IppRSAkeyDp   = 0x20,
   ippRSAkeyDq   = 0x40,  IppRSAkeyDq   = 0x40,
   ippRSAkeyQinv = 0x80,  IppRSAkeyQinv = 0x80
} IppRSAKeyTag;

#define MIN_RSA_SIZE (32)
#define MAX_RSA_SIZE (65536)

/*
// =========================================================
// DL Cryptography
// =========================================================
*/
typedef struct _cpDLP IppsDLPState;

/* domain parameter tags */
typedef enum {
   IppDLPkeyP    = 0x01,
   IppDLPkeyR    = 0x02,
   IppDLPkeyG    = 0x04
} IppDLPKeyTag;

typedef enum {
   ippDLValid,                /* validation pass successfully  */

   ippDLBaseIsEven,           /* !(P is odd)                   */
   ippDLOrderIsEven,          /* !(R is odd)                   */
   ippDLInvalidBaseRange,     /* !(2^(L-1) < P < 2^L)          */
   ippDLInvalidOrderRange,    /* !(2^(M-1) < R < 2^M)          */
   ippDLCompositeBase,
   ippDLCompositeOrder,
   ippDLInvalidCofactor,      /* !( R|(P-1) )                  */
   ippDLInvalidGenerator,     /* !( G^R == 1 (mod P) )         */
                              /* !(1 < G < (P-1))              */
   ippDLInvalidPrivateKey,    /* !(1 < private < (R-1))        */
   ippDLInvalidPublicKey,     /* !(1 < public  <=(P-1))        */
   ippDLInvalidKeyPair,       /* !(G^private == public         */

   ippDLInvalidSignature       /* invalid signature             */
} IppDLResult;

#define MIN_DLP_BITSIZE      (512)
#define MIN_DLP_BITSIZER     (160)

#define MIN_DLPDH_BITSIZE    (512)
#define MIN_DLPDH_BITSIZER   (160)
#define DEF_DLPDH_BITSIZER   (160)

#define MIN_DLPDSA_BITSIZE   (512)
#define MAX_DLPDSA_BITSIZE  (1024)
#define MIN_DLPDSA_BITSIZER  (160)
#define DEF_DLPDSA_BITSIZER  (160)
#define MAX_DLPDSA_BITSIZER  (160)
#define MIN_DLPDSA_SEEDSIZE  (160)

/*
// =========================================================
// EC Cryptography
// =========================================================
*/
typedef struct _cpECCP      IppsECCPState;
typedef struct _cpECCB      IppsECCBState;
typedef struct _cpECCPPoint IppsECCPPointState;
typedef struct _cpECCBPoint IppsECCBPointState;

/* operation result */
typedef enum {
   ippECValid,             /* validation pass successfully     */

   ippECCompositeBase,     /* field based on composite         */
   ippECComplicatedBase,   /* number of non-zero terms in the polynomial (> PRIME_ARR_MAX) */
   ippECIsZeroDiscriminant,/* zero discriminant */
   ippECCompositeOrder,    /* composite order of base point    */
   ippECInvalidOrder,      /* invalid base point order         */
   ippECIsWeakMOV,         /* weak Meneze-Okamoto-Vanstone  reduction attack */
   ippECIsWeakSSSA,        /* weak Semaev-Smart,Satoh-Araki reduction attack */
   ippECIsSupersingular,   /* supersingular curve */

   ippECInvalidPrivateKey, /* !(0 < Private < order) */
   ippECInvalidPublicKey,  /* (order*PublicKey != Infinity)    */
   ippECInvalidKeyPair,    /* (Private*BasePoint != PublicKey) */

   ippECPointOutOfGroup,   /* out of group (order*P != Infinity)  */
   ippECPointIsAtInfinite, /* point (P=(Px,Py)) at Infinity  */
   ippECPointIsNotValid,   /* point (P=(Px,Py)) out-of EC    */

   ippECPointIsEqual,      /* compared points are equal     */
   ippECPointIsNotEqual,   /* compared points are different  */

   ippECInvalidSignature   /* invalid signature */
} IppECResult;

/* domain parameter set/get flags */
typedef enum {
   ippECarbitrary =0x00000,        IppECCArbitrary = 0x00000,       /* arbitrary ECC */

   ippECPstd      = 0x10000,       IppECCPStd      = 0x10000,       /* random (recommended) EC over FG(p): */
   ippECPstd112r1 = ippECPstd,     IppECCPStd112r1 = IppECCPStd,    /* secp112r1 request */
   ippECPstd112r2 = ippECPstd+1,   IppECCPStd112r2 = IppECCPStd+1,  /* secp112r2 request */
   ippECPstd128r1 = ippECPstd+2,   IppECCPStd128r1 = IppECCPStd+2,  /* secp128r1 request */
   ippECPstd128r2 = ippECPstd+3,   IppECCPStd128r2 = IppECCPStd+3,  /* secp128r2 request */
   ippECPstd160r1 = ippECPstd+4,   IppECCPStd160r1 = IppECCPStd+4,  /* secp160r1 request */
   ippECPstd160r2 = ippECPstd+5,   IppECCPStd160r2 = IppECCPStd+5,  /* secp160r2 request */
   ippECPstd192r1 = ippECPstd+6,   IppECCPStd192r1 = IppECCPStd+6,  /* secp192r1 request */
   ippECPstd224r1 = ippECPstd+7,   IppECCPStd224r1 = IppECCPStd+7,  /* secp224r1 request */
   ippECPstd256r1 = ippECPstd+8,   IppECCPStd256r1 = IppECCPStd+8,  /* secp256r1 request */
   ippECPstd384r1 = ippECPstd+9,   IppECCPStd384r1 = IppECCPStd+9,  /* secp384r1 request */
   ippECPstd521r1 = ippECPstd+10,  IppECCPStd521r1 = IppECCPStd+10, /* secp521r1 request */

   IppECCBStd      = 0x20000,       /* random (recommended) EC over FG(2^m): */
   IppECCBStd113r1 = IppECCBStd,    /* sect113r1 request */
   IppECCBStd113r2 = IppECCBStd+1,  /* sect113r2 request */
   IppECCBStd131r1 = IppECCBStd+2,  /* sect131r1 request */
   IppECCBStd131r2 = IppECCBStd+3,  /* sect131r2 request */
   IppECCBStd163r1 = IppECCBStd+4,  /* sect163r1 request */
   IppECCBStd163r2 = IppECCBStd+5,  /* sect163r2 request */
   IppECCBStd193r1 = IppECCBStd+6,  /* sect193r1 request */
   IppECCBStd193r2 = IppECCBStd+7,  /* sect193r2 request */
   IppECCBStd233r1 = IppECCBStd+8,  /* sect233r1 request */
   IppECCBStd283r1 = IppECCBStd+9,  /* sect283r1 request */
   IppECCBStd409r1 = IppECCBStd+10, /* sect409r1 request */
   IppECCBStd571r1 = IppECCBStd+11, /* sect571r1 request */

   IppECCKStd      = 0x40000,       /* Koblitz (recommended) EC over FG(2^m): */
   IppECCBStd163k1 = IppECCKStd,    /* Koblitz 163 request */
   IppECCBStd233k1 = IppECCKStd+1,  /* Koblitz 233 request */
   IppECCBStd239k1 = IppECCKStd+2,  /* Koblitz 239 request */
   IppECCBStd283k1 = IppECCKStd+3,  /* Koblitz 283 request */
   IppECCBStd409k1 = IppECCKStd+4,  /* Koblitz 409 request */
   IppECCBStd571k1 = IppECCKStd+5   /* Koblitz 571 request */
} IppsECType, IppECCType;

#endif /* _OWN_BLDPCS */

#ifdef __cplusplus
}
#endif

#endif /* __IPPCPDEFS_H__ */
