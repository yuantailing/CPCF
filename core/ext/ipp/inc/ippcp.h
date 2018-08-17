/* /////////////////////////////////////////////////////////////////////////////
//
//                  INTEL CORPORATION PROPRIETARY INFORMATION
//     This software is supplied under the terms of a license agreement or
//     nondisclosure agreement with Intel Corporation and may not be copied
//     or disclosed except in accordance with the terms of that agreement.
//          Copyright(c) 2002-2012 Intel Corporation. All Rights Reserved.
//
//              Intel(R) Integrated Performance Primitives
//                  Cryptographic Primitives (ippCP)
//
*/

#if !defined( __IPPCP_H__ ) || defined( _OWN_BLDPCS )
#define __IPPCP_H__


#if defined (_WIN32_WCE) && defined (_M_IX86) && defined (__stdcall)
  #define _IPP_STDCALL_CDECL
  #undef __stdcall
#endif


#ifndef __IPPDEFS_H__
  #include "ippdefs.h"
#endif

#ifndef __IPPCPDEFS_H__
  #include "ippcpdefs.h"
#endif


#ifdef  __cplusplus
extern "C" {
#endif

#if !defined( _IPP_NO_DEFAULT_LIB )
  #if defined( _IPP_PARALLEL_DYNAMIC )
    #pragma comment( lib, "ippcp" )
    #pragma comment( lib, "ippcore" )
  #elif defined( _IPP_PARALLEL_STATIC )
    #pragma comment( lib, "ippcp_t" )
    #pragma comment( lib, "ippcore_t" )
  #elif defined( _IPP_SEQUENTIAL_STATIC )
    #pragma comment( lib, "ippcp_l" )
    #pragma comment( lib, "ippcore_l" )
  #endif
#endif


/* /////////////////////////////////////////////////////////////////////////////
//  Name:       ippcpGetLibVersion
//  Purpose:    getting of the library version
//  Returns:    the structure of information about version of ippCP library
//  Parameters:
//
//  Notes:      not necessary to release the returned structure
*/
IPPAPI( const IppLibraryVersion*, ippcpGetLibVersion, (void) )



/*
// =========================================================
// Symmetric Ciphers
// =========================================================
*/

/* TDES */
IPPAPI(IppStatus, ippsDESGetSize,(int *size))
IPPAPI(IppStatus, ippsDESInit,(const Ipp8u* pKey, IppsDESSpec* pCtx))

IPPAPI(IppStatus, ippsDESPack,(const IppsDESSpec* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsDESUnpack,(const Ipp8u* pBuffer, IppsDESSpec* pCtx))

IPPAPI(IppStatus, ippsTDESEncryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                      const IppsDESSpec* pCtx1, const IppsDESSpec* pCtx2, const IppsDESSpec* pCtx3,
                                      IppsCPPadding padding))
IPPAPI(IppStatus, ippsTDESDecryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                      const IppsDESSpec* pCtx1, const IppsDESSpec* pCtx2, const IppsDESSpec* pCtx3,
                                      IppsCPPadding padding))

IPPAPI(IppStatus, ippsTDESEncryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                      const IppsDESSpec* pCtx1, const IppsDESSpec* pCtx2, const IppsDESSpec* pCtx3,
                                      const Ipp8u* pIV,
                                      IppsCPPadding padding))
IPPAPI(IppStatus, ippsTDESDecryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                      const IppsDESSpec* pCtx1, const IppsDESSpec* pCtx2, const IppsDESSpec* pCtx3,
                                      const Ipp8u* pIV,
                                      IppsCPPadding padding))

IPPAPI(IppStatus, ippsTDESEncryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int length, int cfbBlkSize,
                                      const IppsDESSpec* pCtx1, const IppsDESSpec* pCtx2, const IppsDESSpec* pCtx3,
                                      const Ipp8u* pIV,
                                      IppsCPPadding padding))
IPPAPI(IppStatus, ippsTDESDecryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int length, int cfbBlkSize,
                                      const IppsDESSpec* pCtx1, const IppsDESSpec* pCtx2, const IppsDESSpec* pCtx3,
                                      const Ipp8u* pIV,
                                      IppsCPPadding padding))

IPPAPI(IppStatus, ippsTDESEncryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int ofbBlkSize,
                                     const IppsDESSpec* pCtx1,
                                     const IppsDESSpec* pCtx2,
                                     const IppsDESSpec* pCtx3,
                                     Ipp8u* pIV))
IPPAPI(IppStatus, ippsTDESDecryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int ofbBlkSize,
                                     const IppsDESSpec* pCtx1,
                                     const IppsDESSpec* pCtx2,
                                     const IppsDESSpec* pCtx3,
                                     Ipp8u* pIV))

IPPAPI(IppStatus, ippsTDESEncryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                      const IppsDESSpec* pCtx1,
                                      const IppsDESSpec* pCtx2,
                                      const IppsDESSpec* pCtx3,
                                      Ipp8u* pCtrValue, int ctrNumBitSize))
IPPAPI(IppStatus, ippsTDESDecryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                      const IppsDESSpec* pCtx1,
                                      const IppsDESSpec* pCtx2,
                                      const IppsDESSpec* pCtx3,
                                      Ipp8u* pCtrValue, int ctrNumBitSize))

/* 128-bit data block size Rijndael (AES) */
IPPAPI(IppStatus, ippsRijndael128GetSize,(int *pSize))
IPPAPI(IppStatus, ippsRijndael128Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen, IppsRijndael128Spec* pCtx))
IPPAPI(IppStatus, ippsSafeRijndael128Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen, IppsRijndael128Spec* pCtx))
IPPAPI(IppStatus, ippsRijndael128SetKey,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen, IppsRijndael128Spec* pCtx))

IPPAPI(IppStatus, ippsRijndael128Pack,(const IppsRijndael128Spec* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsRijndael128Unpack,(const Ipp8u* pBuffer, IppsRijndael128Spec* pCtx))

IPPAPI(IppStatus, ippsRijndael128EncryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael128Spec* pCtx,
                                             IppsPadding padding))
IPPAPI(IppStatus, ippsRijndael128DecryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael128Spec* pCtx,
                                             IppsPadding padding))

IPPAPI(IppStatus, ippsRijndael128EncryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael128Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))
IPPAPI(IppStatus, ippsRijndael128DecryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael128Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))

IPPAPI(IppStatus, ippsRijndael128EncryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael128Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))
IPPAPI(IppStatus, ippsRijndael128DecryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael128Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))

IPPAPI(IppStatus, ippsRijndael128EncryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int ofbBlkSize,
                                             const IppsRijndael128Spec* pCtx,
                                             Ipp8u* pIV))
IPPAPI(IppStatus, ippsRijndael128DecryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int ofbBlkSize,
                                             const IppsRijndael128Spec* pCtx,
                                             Ipp8u* pIV))

IPPAPI(IppStatus, ippsRijndael128EncryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                      const IppsRijndael128Spec* pCtx,
                                      Ipp8u* pCtrValue, int ctrNumBitSize))
IPPAPI(IppStatus, ippsRijndael128DecryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                      const IppsRijndael128Spec* pCtx,
                                      Ipp8u* pCtrValue, int ctrNumBitSize))

/* 192-bit data block size Rijndael */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/") \
IPPAPI(IppStatus, ippsRijndael192GetSize,(int *pSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen, IppsRijndael192Spec* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192Pack,(const IppsRijndael192Spec* pCtx, Ipp8u* pBuffer))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192Unpack,(const Ipp8u* pBuffer, IppsRijndael192Spec* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192EncryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael192Spec* pCtx,
                                             IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192DecryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael192Spec* pCtx,
                                             IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192EncryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael192Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192DecryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael192Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192EncryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael192Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192DecryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael192Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192EncryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int ofbBlkSize,
                                             const IppsRijndael192Spec* pCtx,
                                             Ipp8u* pIV))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192DecryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int ofbBlkSize,
                                             const IppsRijndael192Spec* pCtx,
                                             Ipp8u* pIV))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192EncryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                      const IppsRijndael192Spec* pCtx,
                                      Ipp8u* pCtrValue, int ctrNumBitSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael192DecryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                      const IppsRijndael192Spec* pCtx,
                                      Ipp8u* pCtrValue, int ctrNumBitSize))

/* 256-bit data block size Rijndael */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256GetSize,(int *pSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen, IppsRijndael256Spec* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256Pack,(const IppsRijndael256Spec* pCtx, Ipp8u* pBuffer))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256Unpack,(const Ipp8u* pBuffer, IppsRijndael256Spec* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256EncryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael256Spec* pCtx,
                                             IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256DecryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael256Spec* pCtx,
                                             IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256EncryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael256Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256DecryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                             const IppsRijndael256Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256EncryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int ofbBlkSize,
                                             const IppsRijndael256Spec* pCtx,
                                             Ipp8u* pIV))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256DecryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int ofbBlkSize,
                                             const IppsRijndael256Spec* pCtx,
                                             Ipp8u* pIV))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256EncryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael256Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256DecryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int len, int cfbBlkSize,
                                             const IppsRijndael256Spec* pCtx,
                                             const Ipp8u* pIV,
                                             IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256EncryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                      const IppsRijndael256Spec* pCtx,
                                      Ipp8u* pCtrValue, int ctrNumBitSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsRijndael256DecryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                      const IppsRijndael256Spec* pCtx,
                                      Ipp8u* pCtrValue, int ctrNumBitSize))


/* 64-bit data block size RC5 */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64GetSize,(int rounds, int* pSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64Init,(const Ipp8u *pKey, int keylen,
                                     int rounds,
                                     IppsARCFive64Spec* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64Pack,(const IppsARCFive64Spec* pCtx, Ipp8u* pBuffer))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64Unpack,(const Ipp8u* pBuffer, IppsARCFive64Spec* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64EncryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                     const IppsARCFive64Spec* pCtx,
                                     IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64DecryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                     const IppsARCFive64Spec* pCtx,
                                     IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64EncryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                     const IppsARCFive64Spec* pCtx,
                                     const Ipp8u* pIV,
                                     IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64DecryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                     const IppsARCFive64Spec* pCtx,
                                     const Ipp8u* pIV,
                                     IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64EncryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int length, int cfbBlkSize,
                                     const IppsARCFive64Spec* pCtx,
                                     const Ipp8u* pIV,
                                     IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64DecryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int length, int cfbBlkSize,
                                     const IppsARCFive64Spec* pCtx,
                                     const Ipp8u* pIV,
                                     IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64EncryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int length, int cfbBlkSize,
                                     const IppsARCFive64Spec* pCtx,
                                     Ipp8u* pIV))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64DecryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int length, int cfbBlkSize,
                                     const IppsARCFive64Spec* pCtx,
                                     Ipp8u* pIV))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64EncryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                     const IppsARCFive64Spec* pCtx,
                                     Ipp8u* pCtrValue, int ctrNumBitSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive64DecryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                     const IppsARCFive64Spec* pCtx,
                                     Ipp8u* pCtrValue, int ctrNumBitSize))

/* 128-bit data block size RC5 */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128GetSize,(int rounds, int *size))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128Init,(const Ipp8u *pKey, int keylen,
                                      int rounds,
                                      IppsARCFive128Spec* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128Pack,(const IppsARCFive128Spec* pCtx, Ipp8u* pBuffer))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128Unpack,(const Ipp8u* pBuffer, IppsARCFive128Spec* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128EncryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                     const IppsARCFive128Spec* pCtx,
                                     IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128DecryptECB,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                     const IppsARCFive128Spec* pCtx,
                                     IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128EncryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                     const IppsARCFive128Spec* pCtx,
                                     const Ipp8u* pIV,
                                     IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128DecryptCBC,(const Ipp8u* pSrc, Ipp8u* pDst, int length,
                                     const IppsARCFive128Spec* pCtx,
                                     const Ipp8u* pIV,
                                     IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128EncryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int length, int cfbBlkSize,
                                     const IppsARCFive128Spec* pCtx,
                                     const Ipp8u* pIV,
                                     IppsPadding padding))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128DecryptCFB,(const Ipp8u* pSrc, Ipp8u* pDst, int length, int cfbBlkSize,
                                     const IppsARCFive128Spec* pCtx,
                                     const Ipp8u* pIV,
                                     IppsPadding padding))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128EncryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int length, int cfbBlkSize,
                                     const IppsARCFive128Spec* pCtx,
                                     Ipp8u* pIV))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128DecryptOFB,(const Ipp8u* pSrc, Ipp8u* pDst, int length, int cfbBlkSize,
                                     const IppsARCFive128Spec* pCtx,
                                     Ipp8u* pIV))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128EncryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                     const IppsARCFive128Spec* pCtx,
                                     Ipp8u* pCtrValue, int ctrNumBitSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsARCFive128DecryptCTR,(const Ipp8u* pSrc, Ipp8u* pDst, int len,
                                     const IppsARCFive128Spec* pCtx,
                                     Ipp8u* pCtrValue, int ctrNumBitSize))


/*
// =========================================================
// AES based  authentication & confidence Primitives
// =========================================================
*/

/*
// AES-CCM
*/
IPPAPI(IppStatus, ippsRijndael128CCMGetSize,(Ipp32u* pSize))
IPPAPI(IppStatus, ippsRijndael128CCMInit,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                       IppsRijndael128CCMState* pState))

IPPAPI(IppStatus, ippsRijndael128CCMMessageLen,(Ipp64u msgLen, IppsRijndael128CCMState* pState))
IPPAPI(IppStatus, ippsRijndael128CCMTagLen,(Ipp32u tagLen, IppsRijndael128CCMState* pState))

IPPAPI(IppStatus, ippsRijndael128CCMStart,(const Ipp8u* pIV,  Ipp32u ivLen,
                                       const Ipp8u* pAAD, Ipp32u aadLen,
                                       IppsRijndael128CCMState* pState))
IPPAPI(IppStatus, ippsRijndael128CCMEncrypt,(const Ipp8u* pSrc, Ipp8u* pDst, Ipp32u len,
                                       IppsRijndael128CCMState* pState))
IPPAPI(IppStatus, ippsRijndael128CCMDecrypt,(const Ipp8u* pSrc, Ipp8u* pDst, Ipp32u len,
                                       IppsRijndael128CCMState* pState))
IPPAPI(IppStatus, ippsRijndael128CCMGetTag,(Ipp8u* pTag, Ipp32u tagLen,
                                       const IppsRijndael128CCMState* pState))

IPPAPI(IppStatus, ippsRijndael128CCMEncryptMessage,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                        const Ipp8u* pIV,  Ipp32u ivLen,
                                        const Ipp8u* pAAD, Ipp32u aadLen,
                                        const Ipp8u* pSrc, Ipp8u* pDst, Ipp32u len,
                                              Ipp8u* pTag, Ipp32u tagLen))
IPPAPI(IppStatus, ippsRijndael128CCMDecryptMessage,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                        const Ipp8u* pIV,  Ipp32u ivLen,
                                        const Ipp8u* pAAD, Ipp32u aadLen,
                                        const Ipp8u* pSrc, Ipp8u* pDst, Ipp32u len,
                                              Ipp8u* pTag, Ipp32u tagLen))

/*
// AES-GCM
*/
IPPAPI(IppStatus, ippsRijndael128GCMGetSizeManaged,(IppsAESGCMbehaviour flag, Ipp32u* pSize))
IPPAPI(IppStatus, ippsRijndael128GCMInitManaged,(IppsAESGCMbehaviour flag, const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                       IppsRijndael128GCMState* pState))

IPPAPI(IppStatus, ippsRijndael128GCMReset,(IppsRijndael128GCMState* pState))
IPPAPI(IppStatus, ippsRijndael128GCMStart,(const Ipp8u* pIV,  Ipp32u ivLen,
                                       const Ipp8u* pAAD, Ipp32u aadLen,
                                       IppsRijndael128GCMState* pState))
IPPAPI(IppStatus, ippsRijndael128GCMEncrypt,(const Ipp8u* pSrc, Ipp8u* pDst,  Ipp32u len,
                                       IppsRijndael128GCMState* pState))
IPPAPI(IppStatus, ippsRijndael128GCMDecrypt,(const Ipp8u* pSrc, Ipp8u* pDst,  Ipp32u len,
                                       IppsRijndael128GCMState* pState))
IPPAPI(IppStatus, ippsRijndael128GCMGetTag,(Ipp8u* pDstTag, Ipp32u tagLen,
                                       const IppsRijndael128GCMState* pState))
IPPAPI(IppStatus, ippsRijndael128GCMProcessIV,(const Ipp8u* pIV,  Ipp32u ivLen,
                                       IppsRijndael128GCMState* pState))
IPPAPI(IppStatus, ippsRijndael128GCMProcessAAD,(const Ipp8u* pAAD,  Ipp32u ivAAD,
                                       IppsRijndael128GCMState* pState))


/*
// =========================================================
// RC4 Stream Ciphers
// =========================================================
*/
IPPAPI(IppStatus, ippsARCFourCheckKey, (const Ipp8u *pKey, int keyLen, IppBool* pIsWeak))

IPPAPI(IppStatus, ippsARCFourGetSize, (int* pSize))
IPPAPI(IppStatus, ippsARCFourInit, (const Ipp8u *pKey, int keyLen, IppsARCFourState *pCtx))
IPPAPI(IppStatus, ippsARCFourReset, (IppsARCFourState* pCtx))

IPPAPI(IppStatus, ippsARCFourPack,(const IppsARCFourState* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsARCFourUnpack,(const Ipp8u* pBuffer, IppsARCFourState* pCtx))

IPPAPI(IppStatus, ippsARCFourEncrypt, (const Ipp8u *pSrc, Ipp8u *pDst, int length, IppsARCFourState *pCtx))
IPPAPI(IppStatus, ippsARCFourDecrypt, (const Ipp8u *pSrc, Ipp8u *pDst, int length, IppsARCFourState *pCtx))


/*
// =========================================================
// One-Way Hash Functions
// =========================================================
*/

/* SHA1 Hash Primitives */
IPPAPI(IppStatus, ippsSHA1GetSize,(int* pSize))
IPPAPI(IppStatus, ippsSHA1Init,(IppsSHA1State* pCtx))
IPPAPI(IppStatus, ippsSHA1Duplicate,(const IppsSHA1State* pSrcCtx, IppsSHA1State* pDstCtx))

IPPAPI(IppStatus, ippsSHA1Pack,(const IppsSHA1State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsSHA1Unpack,(const Ipp8u* pBuffer, IppsSHA1State* pCtx))

IPPAPI(IppStatus, ippsSHA1Update,(const Ipp8u* pSrc, int len, IppsSHA1State* pCtx))
IPPAPI(IppStatus, ippsSHA1GetTag,(Ipp8u* pTag, Ipp32u tagLen, const IppsSHA1State* pCtx))
IPPAPI(IppStatus, ippsSHA1Final,(Ipp8u* pMD, IppsSHA1State* pCtx))
IPPAPI(IppStatus, ippsSHA1MessageDigest,(const Ipp8u* pMsg, int len, Ipp8u* pMD))

/* SHA224 Hash Primitives */
IPPAPI(IppStatus, ippsSHA224GetSize,(int* pSize))
IPPAPI(IppStatus, ippsSHA224Init,(IppsSHA224State* pCtx))
IPPAPI(IppStatus, ippsSHA224Duplicate,(const IppsSHA224State* pSrcCtx, IppsSHA224State* pDstCtx))

IPPAPI(IppStatus, ippsSHA224Pack,(const IppsSHA224State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsSHA224Unpack,(const Ipp8u* pBuffer, IppsSHA224State* pCtx))

IPPAPI(IppStatus, ippsSHA224Update,(const Ipp8u* pSrc, int len, IppsSHA224State* pCtx))
IPPAPI(IppStatus, ippsSHA224GetTag,(Ipp8u* pTag, Ipp32u tagLen, const IppsSHA224State* pCtx))
IPPAPI(IppStatus, ippsSHA224Final,(Ipp8u* pMD, IppsSHA224State* pCtx))
IPPAPI(IppStatus, ippsSHA224MessageDigest,(const Ipp8u* pMsg, int len, Ipp8u* pMD))

/* SHA256 Hash Primitives */
IPPAPI(IppStatus, ippsSHA256GetSize,(int* pSize))
IPPAPI(IppStatus, ippsSHA256Init,(IppsSHA256State* pCtx))
IPPAPI(IppStatus, ippsSHA256Duplicate,(const IppsSHA256State* pSrcCtx, IppsSHA256State* pDstCtx))

IPPAPI(IppStatus, ippsSHA256Pack,(const IppsSHA256State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsSHA256Unpack,(const Ipp8u* pBuffer, IppsSHA256State* pCtx))

IPPAPI(IppStatus, ippsSHA256Update,(const Ipp8u* pSrc, int len, IppsSHA256State* pCtx))
IPPAPI(IppStatus, ippsSHA256GetTag,(Ipp8u* pTag, Ipp32u tagLen, const IppsSHA256State* pCtx))
IPPAPI(IppStatus, ippsSHA256Final,(Ipp8u* pMD, IppsSHA256State* pCtx))
IPPAPI(IppStatus, ippsSHA256MessageDigest,(const Ipp8u* pMsg, int len, Ipp8u* pMD))

/* SHA384 Hash Primitives */
IPPAPI(IppStatus, ippsSHA384GetSize,(int* pSize))
IPPAPI(IppStatus, ippsSHA384Init,(IppsSHA384State* pCtx))
IPPAPI(IppStatus, ippsSHA384Duplicate,(const IppsSHA384State* pSrcCtx, IppsSHA384State* pDstCtx))

IPPAPI(IppStatus, ippsSHA384Pack,(const IppsSHA384State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsSHA384Unpack,(const Ipp8u* pBuffer, IppsSHA384State* pCtx))

IPPAPI(IppStatus, ippsSHA384Update,(const Ipp8u* pSrc, int len, IppsSHA384State* pCtx))
IPPAPI(IppStatus, ippsSHA384GetTag,(Ipp8u* pTag, Ipp32u tagLen, const IppsSHA384State* pCtx))
IPPAPI(IppStatus, ippsSHA384Final,(Ipp8u* pMD, IppsSHA384State* pCtx))
IPPAPI(IppStatus, ippsSHA384MessageDigest,(const Ipp8u* pMsg, int len, Ipp8u* pMD))

/* SHA512 Hash Primitives */
IPPAPI(IppStatus, ippsSHA512GetSize,(int* pSize))
IPPAPI(IppStatus, ippsSHA512Init,(IppsSHA512State* pCtx))
IPPAPI(IppStatus, ippsSHA512Duplicate,(const IppsSHA512State* pSrcCtx, IppsSHA512State* pDstCtx))

IPPAPI(IppStatus, ippsSHA512Pack,(const IppsSHA512State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsSHA512Unpack,(const Ipp8u* pBuffer, IppsSHA512State* pCtx))

IPPAPI(IppStatus, ippsSHA512Update,(const Ipp8u* pSrc, int len, IppsSHA512State* pCtx))
IPPAPI(IppStatus, ippsSHA512GetTag,(Ipp8u* pTag, Ipp32u tagLen, const IppsSHA512State* pCtx))
IPPAPI(IppStatus, ippsSHA512Final,(Ipp8u* pMD, IppsSHA512State* pCtx))
IPPAPI(IppStatus, ippsSHA512MessageDigest,(const Ipp8u* pMsg, int len, Ipp8u* pMD))

/* MD5 Hash Primitives */
IPPAPI(IppStatus, ippsMD5GetSize,(int* pSize))
IPPAPI(IppStatus, ippsMD5Init,(IppsMD5State* pCtx))
IPPAPI(IppStatus, ippsMD5Duplicate,(const IppsMD5State* pSrcCtx, IppsMD5State* pDstCtx))

IPPAPI(IppStatus, ippsMD5Pack,(const IppsMD5State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsMD5Unpack,(const Ipp8u* pBuffer, IppsMD5State* pCtx))

IPPAPI(IppStatus, ippsMD5Update,(const Ipp8u* pSrc, int len, IppsMD5State* pCtx))
IPPAPI(IppStatus, ippsMD5GetTag,(Ipp8u* pTag, Ipp32u tagLen, const IppsMD5State* pCtx))
IPPAPI(IppStatus, ippsMD5Final,(Ipp8u* pMD, IppsMD5State* pCtx))
IPPAPI(IppStatus, ippsMD5MessageDigest,(const Ipp8u* pMsg, int len, Ipp8u* pMD))

/* HASH based MFG Primitives */
IPPAPI(IppStatus, ippsMGF_SHA1,  (const Ipp8u* pSeed, int seedLen, Ipp8u* pMask, int maskLen))
IPPAPI(IppStatus, ippsMGF_SHA224,(const Ipp8u* pSeed, int seedLen, Ipp8u* pMask, int maskLen))
IPPAPI(IppStatus, ippsMGF_SHA256,(const Ipp8u* pSeed, int seedLen, Ipp8u* pMask, int maskLen))
IPPAPI(IppStatus, ippsMGF_SHA384,(const Ipp8u* pSeed, int seedLen, Ipp8u* pMask, int maskLen))
IPPAPI(IppStatus, ippsMGF_SHA512,(const Ipp8u* pSeed, int seedLen, Ipp8u* pMask, int maskLen))
IPPAPI(IppStatus, ippsMGF_MD5,   (const Ipp8u* pSeed, int seedLen, Ipp8u* pMask, int maskLen))


/*
// =========================================================
// Keyed-Hash Message Authentication Codes
// =========================================================
*/

/* SHA1 based HMAC Primitives */
IPPAPI(IppStatus, ippsHMACSHA1GetSize,(int* size))
IPPAPI(IppStatus, ippsHMACSHA1Init,(const Ipp8u* pKey, int keyLen, IppsHMACSHA1State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA1Duplicate,(const IppsHMACSHA1State* pSrcCtx, IppsHMACSHA1State* pDstCtx))

IPPAPI(IppStatus, ippsHMACSHA1Pack,(const IppsHMACSHA1State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsHMACSHA1Unpack,(const Ipp8u* pBuffer, IppsHMACSHA1State* pCtx))

IPPAPI(IppStatus, ippsHMACSHA1Update,(const Ipp8u* pSrc, int len, IppsHMACSHA1State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA1GetTag,(Ipp8u* pTag, int tagLen, const IppsHMACSHA1State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA1Final, (Ipp8u* pMD, int mdLen, IppsHMACSHA1State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA1MessageDigest,(const Ipp8u* pMsg, int msgLen,
                                             const Ipp8u* pKey, int keyLen,
                                             Ipp8u* pMD, int mdLen))
/* SHA256 based HMAC Primitives */
IPPAPI(IppStatus, ippsHMACSHA256GetSize,(int* size))
IPPAPI(IppStatus, ippsHMACSHA256Init,(const Ipp8u* pKey, int keyLen, IppsHMACSHA256State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA256Duplicate,(const IppsHMACSHA256State* pSrcCtx, IppsHMACSHA256State* pDstCtx))

IPPAPI(IppStatus, ippsHMACSHA256Pack,(const IppsHMACSHA256State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsHMACSHA256Unpack,(const Ipp8u* pBuffer, IppsHMACSHA256State* pCtx))

IPPAPI(IppStatus, ippsHMACSHA256Update,(const Ipp8u* pSrc, int len, IppsHMACSHA256State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA256GetTag,(Ipp8u* pTag, int tagLen, const IppsHMACSHA256State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA256Final, (Ipp8u* pMD, int mdLen, IppsHMACSHA256State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA256MessageDigest,(const Ipp8u* pMsg, int msgLen,
                                               const Ipp8u* pKey, int keyLen,
                                               Ipp8u* pMD, int mdLen))
/* SHA224 based HMAC Primitives */
IPPAPI(IppStatus, ippsHMACSHA224GetSize,(int* size))
IPPAPI(IppStatus, ippsHMACSHA224Init,(const Ipp8u* pKey, int keyLen, IppsHMACSHA224State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA224Duplicate,(const IppsHMACSHA224State* pSrcCtx, IppsHMACSHA224State* pDstCtx))

IPPAPI(IppStatus, ippsHMACSHA224Pack,(const IppsHMACSHA224State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsHMACSHA224Unpack,(const Ipp8u* pBuffer, IppsHMACSHA224State* pCtx))

IPPAPI(IppStatus, ippsHMACSHA224Update,(const Ipp8u* pSrc, int len, IppsHMACSHA224State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA224GetTag,(Ipp8u* pTag, int tagLen, const IppsHMACSHA224State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA224Final, (Ipp8u* pMD, int mdLen, IppsHMACSHA224State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA224MessageDigest,(const Ipp8u* pMsg, int msgLen,
                                               const Ipp8u* pKey, int keyLen,
                                               Ipp8u* pMD, int mdLen))
/* SHA384 based HMAC Primitives */
IPPAPI(IppStatus, ippsHMACSHA384GetSize,(int* size))
IPPAPI(IppStatus, ippsHMACSHA384Init,(const Ipp8u* pKey, int keyLen, IppsHMACSHA384State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA384Duplicate,(const IppsHMACSHA384State* pSrcCtx, IppsHMACSHA384State* pDstCtx))

IPPAPI(IppStatus, ippsHMACSHA384Pack,(const IppsHMACSHA384State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsHMACSHA384Unpack,(const Ipp8u* pBuffer, IppsHMACSHA384State* pCtx))

IPPAPI(IppStatus, ippsHMACSHA384Update,(const Ipp8u* pSrc, int len, IppsHMACSHA384State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA384GetTag,(Ipp8u* pTag, int tagLen, const IppsHMACSHA384State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA384Final, (Ipp8u* pMD, int mdLen, IppsHMACSHA384State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA384MessageDigest,(const Ipp8u* pMsg, int msgLen,
                                               const Ipp8u* pKey, int keyLen,
                                               Ipp8u* pMD, int mdLen))
/* SHA512 based HMAC Primitives */
IPPAPI(IppStatus, ippsHMACSHA512GetSize,(int* size))
IPPAPI(IppStatus, ippsHMACSHA512Init,(const Ipp8u* pKey, int keyLen, IppsHMACSHA512State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA512Duplicate,(const IppsHMACSHA512State* pSrcCtx, IppsHMACSHA512State* pDstCtx))

IPPAPI(IppStatus, ippsHMACSHA512Pack,(const IppsHMACSHA512State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsHMACSHA512Unpack,(const Ipp8u* pBuffer, IppsHMACSHA512State* pCtx))

IPPAPI(IppStatus, ippsHMACSHA512Update,(const Ipp8u* pSrc, int len, IppsHMACSHA512State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA512GetTag,(Ipp8u* pTag, int tagLen, const IppsHMACSHA512State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA512Final, (Ipp8u* pMD, int mdLen, IppsHMACSHA512State* pCtx))
IPPAPI(IppStatus, ippsHMACSHA512MessageDigest,(const Ipp8u* pMsg, int msgLen,
                                               const Ipp8u* pKey, int keyLen,
                                               Ipp8u* pMD, int mdLen))
/* MD5 based HMAC Primitives */
IPPAPI(IppStatus, ippsHMACMD5GetSize,(int* size))
IPPAPI(IppStatus, ippsHMACMD5Init,(const Ipp8u* pKey, int keyLen, IppsHMACMD5State* pCtx))
IPPAPI(IppStatus, ippsHMACMD5Duplicate,(const IppsHMACMD5State* pSrcCtx, IppsHMACMD5State* pDstCtx))

IPPAPI(IppStatus, ippsHMACMD5Pack,(const IppsHMACMD5State* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsHMACMD5Unpack,(const Ipp8u* pBuffer, IppsHMACMD5State* pCtx))

IPPAPI(IppStatus, ippsHMACMD5Update,(const Ipp8u* pSrc, int len, IppsHMACMD5State* pCtx))
IPPAPI(IppStatus, ippsHMACMD5GetTag,(Ipp8u* pTag, int tagLen, const IppsHMACMD5State* pCtx))
IPPAPI(IppStatus, ippsHMACMD5Final, (Ipp8u* pMD, int mdLen, IppsHMACMD5State* pCtx))
IPPAPI(IppStatus, ippsHMACMD5MessageDigest,(const Ipp8u* pMsg, int msgLen,
                                            const Ipp8u* pKey, int keyLen,
                                            Ipp8u* pMD, int mdLen))


/*
// =========================================================
// Data Authentication Codes
// =========================================================
*/

/* TDES based DAA Primitives */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAATDESGetSize,(int* pSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAATDESInit,(const Ipp8u* pKey1, const Ipp8u* pKey2, const Ipp8u* pKey3,
                                   IppsDAATDESState* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAATDESUpdate,(const Ipp8u* pSrc, int len, IppsDAATDESState* pCtx))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAATDESFinal,(Ipp8u* pMD, int mdLen, IppsDAATDESState* pCtx))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAATDESMessageDigest,(const Ipp8u* pMsg, int msgLen,
                                            const Ipp8u* pKey1, const Ipp8u* pKey2, const Ipp8u* pKey3,
                                            Ipp8u* pMD, int mdLen))

/* Rijndael128 based DAA Primitives */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael128GetSize,(int* pSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael128Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                          IppsDAARijndael128State* pCtx))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAASafeRijndael128Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                          IppsDAARijndael128State* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael128Update,(const Ipp8u* pSrc, int len, IppsDAARijndael128State* pCtx))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael128Final,(Ipp8u* pMD, int mdLen, IppsDAARijndael128State* pCtx))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael128MessageDigest,(const Ipp8u* pMsg, int msgLen,
                                                   const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                                   Ipp8u* pMD, int mdLen))
/* Rijndael192 based DAA Primitives */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael192GetSize,(int* pSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael192Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                          IppsDAARijndael192State* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael192Update,(const Ipp8u* pSrc, int len, IppsDAARijndael192State* pCtx))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael192Final,(Ipp8u* pMD, int mdLen, IppsDAARijndael192State* pCtx))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael192MessageDigest,(const Ipp8u* pMsg, int msgLen,
                                                   const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                                   Ipp8u* pMD, int mdLen))
/* Rijndael256 based DAA Primitives */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael256GetSize,(int* pSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael256Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                          IppsDAARijndael256State* pCtx))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael256Update,(const Ipp8u* pSrc, int len, IppsDAARijndael256State* pCtx))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael256Final,(Ipp8u* pMD, int mdLen, IppsDAARijndael256State* pCtx))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsDAARijndael256MessageDigest,(const Ipp8u* pMsg, int msgLen,
                                                   const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                                   Ipp8u* pMD, int mdLen))
/* AES based MAC Primitives */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsXCBCRijndael128GetSize,(Ipp32u* pSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsXCBCRijndael128Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                           IppsXCBCRijndael128State* pState))


IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsXCBCRijndael128Update,(const Ipp8u* pSrc, Ipp32u len, IppsXCBCRijndael128State* pState))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsXCBCRijndael128GetTag,(Ipp8u* pTag, Ipp32u tagLen, const IppsXCBCRijndael128State* pState))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsXCBCRijndael128Final,(Ipp8u* pTag, Ipp32u tagLen, IppsXCBCRijndael128State* pState))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsXCBCRijndael128MessageTag,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                 const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                                 Ipp8u* pTag, Ipp32u tagLen))

/* AES-CMAC Primitives */
IPPAPI(IppStatus, ippsCMACRijndael128GetSize,(int* pSize))
IPPAPI(IppStatus, ippsCMACRijndael128Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                          IppsCMACRijndael128State* pState))
IPPAPI(IppStatus, ippsCMACSafeRijndael128Init,(const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                          IppsCMACRijndael128State* pState))

IPPAPI(IppStatus, ippsCMACRijndael128Update,(const Ipp8u* pSrc, int len, IppsCMACRijndael128State* pState))
IPPAPI(IppStatus, ippsCMACRijndael128Final,(Ipp8u* pMD, int mdLen, IppsCMACRijndael128State* pState))
IPPAPI(IppStatus, ippsCMACRijndael128MessageDigest,(const Ipp8u* pMsg, int msgLen,
                                                    const Ipp8u* pKey, IppsRijndaelKeyLength keyLen,
                                                    Ipp8u* pMD, int mdLen))


/*
// =========================================================
// Big Number Integer Arithmetic
// =========================================================
*/

/* Signed BigNum Operations */
IPPAPI(IppStatus, ippsBigNumGetSize,(int length, int* pSize))
IPPAPI(IppStatus, ippsBigNumInit,(int length, IppsBigNumState* pBN))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use ippsCmp_BN instead. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsCmpZero_BN,(const IppsBigNumState* pBN, Ipp32u* pResult))
IPPAPI(IppStatus, ippsCmp_BN,(const IppsBigNumState* pA, const IppsBigNumState* pB, Ipp32u* pResult))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use ippsRef_BN instead. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsGetSize_BN,(const IppsBigNumState* pBN, int* pSize))
IPPAPI(IppStatus, ippsSet_BN,(IppsBigNumSGN sgn,
                              int length, const Ipp32u* pData,
                              IppsBigNumState* pBN))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use ippsRef_BN instead. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsGet_BN,(IppsBigNumSGN* pSgn,
                              int* pLength, Ipp32u* pData,
                              const IppsBigNumState* pBN))
IPPAPI(IppStatus, ippsRef_BN,(IppsBigNumSGN* pSgn, int* bitSize, Ipp32u** const ppData,
                              const IppsBigNumState* pBN))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use ippsRef_BN instead. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsExtGet_BN,(IppsBigNumSGN* pSgn,
                              int* pBitSize, Ipp32u* pData,
                              const IppsBigNumState* pBN))

IPPAPI(IppStatus, ippsAdd_BN,   (IppsBigNumState* pA, IppsBigNumState* pB, IppsBigNumState* pR))
IPPAPI(IppStatus, ippsSub_BN,   (IppsBigNumState* pA, IppsBigNumState* pB, IppsBigNumState* pR))
IPPAPI(IppStatus, ippsMul_BN,   (IppsBigNumState* pA, IppsBigNumState* pB, IppsBigNumState* pR))
IPPAPI(IppStatus, ippsMAC_BN_I, (IppsBigNumState* pA, IppsBigNumState* pB, IppsBigNumState* pR))
IPPAPI(IppStatus, ippsDiv_BN,   (IppsBigNumState* pA, IppsBigNumState* pB, IppsBigNumState* pQ, IppsBigNumState* pR))
IPPAPI(IppStatus, ippsMod_BN,   (IppsBigNumState* pA, IppsBigNumState* pM, IppsBigNumState* pR))
IPPAPI(IppStatus, ippsGcd_BN,   (IppsBigNumState* pA, IppsBigNumState* pB, IppsBigNumState* pGCD))
IPPAPI(IppStatus, ippsModInv_BN,(IppsBigNumState* pA, IppsBigNumState* pM, IppsBigNumState* pInv))

IPPAPI(IppStatus, ippsSetOctString_BN,(const Ipp8u* pStr, int strLen, IppsBigNumState* pBN))
IPPAPI(IppStatus, ippsGetOctString_BN,(Ipp8u* pStr, int strLen, const IppsBigNumState* pBN))

/* Montgomery Operations */
IPPAPI(IppStatus, ippsMontGetSize,(IppsExpMethod method, int length, int* pSize))
IPPAPI(IppStatus, ippsMontInit,(IppsExpMethod method, int length, IppsMontState* pCtx))

IPPAPI(IppStatus, ippsMontSet,(const Ipp32u* pModulo, int size, IppsMontState* pCtx))
IPPAPI(IppStatus, ippsMontGet,(Ipp32u* pModulo, int* pSize, const IppsMontState* pCtx))

IPPAPI(IppStatus, ippsMontForm,(const IppsBigNumState* pA, IppsMontState* pCtx, IppsBigNumState* pR))
IPPAPI(IppStatus, ippsMontMul, (const IppsBigNumState* pA, const IppsBigNumState* pB, IppsMontState* m, IppsBigNumState* pR))
IPPAPI(IppStatus, ippsMontExp, (const IppsBigNumState* pA, const IppsBigNumState* pE, IppsMontState* m, IppsBigNumState* pR))

/* Pseudo-Random Number Generation */
IPPAPI(IppStatus, ippsPRNGGetSize,(int* pSize))
IPPAPI(IppStatus, ippsPRNGInit,   (int seedBits, IppsPRNGState* pCtx))

IPPAPI(IppStatus, ippsPRNGSetModulus,(const IppsBigNumState* pMod, IppsPRNGState* pCtx))
IPPAPI(IppStatus, ippsPRNGSetH0,     (const IppsBigNumState* pH0,  IppsPRNGState* pCtx))
IPPAPI(IppStatus, ippsPRNGSetAugment,(const IppsBigNumState* pAug, IppsPRNGState* pCtx))
IPPAPI(IppStatus, ippsPRNGSetSeed,   (const IppsBigNumState* pSeed,IppsPRNGState* pCtx))
IPPAPI(IppStatus, ippsPRNGGetSeed,   (const IppsPRNGState* pCtx,IppsBigNumState* pSeed))

IPPAPI(IppStatus, ippsPRNGen,   (Ipp32u* pRand, int nBits, void* pCtx))
IPPAPI(IppStatus, ippsPRNGen_BN,(IppsBigNumState* pRand, int nBits, void* pCtx))

/* Probable Prime Number Generation */
IPPAPI(IppStatus, ippsPrimeGetSize,(int nMaxBits, int* pSize))
IPPAPI(IppStatus, ippsPrimeInit,   (int nMaxBits, IppsPrimeState* pCtx))

IPPAPI(IppStatus, ippsPrimeGen, (int nBits, int nTrials, IppsPrimeState* pCtx,
                                 IppBitSupplier rndFunc, void* pRndParam))
IPPAPI(IppStatus, ippsPrimeTest,(int nTrials, Ipp32u* pResult, IppsPrimeState* pCtx,
                                 IppBitSupplier rndFunc, void* pRndParam))

IPPAPI(IppStatus, ippsPrimeGet,   (Ipp32u* pPrime, int* pLen, const IppsPrimeState* pCtx))
IPPAPI(IppStatus, ippsPrimeGet_BN,(IppsBigNumState* pPrime, const IppsPrimeState* pCtx))

IPPAPI(IppStatus, ippsPrimeSet,   (const Ipp32u* pPrime, int nBits, IppsPrimeState* pCtx))
IPPAPI(IppStatus, ippsPrimeSet_BN,(const IppsBigNumState* pPrime, IppsPrimeState* pCtx))


/*
// =========================================================
// RSA Cryptography
// =========================================================
*/
IPPAPI(IppStatus, ippsRSAGetSize,(int kn, int kp, IppRSAKeyType flag, int* pSize))
IPPAPI(IppStatus, ippsRSAInit,   (int kn, int kp, IppRSAKeyType flag, IppsRSAState* pCtx))

IPPAPI(IppStatus, ippsRSAPack,(const IppsRSAState* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsRSAUnpack,(const Ipp8u* pBuffer, IppsRSAState* pCtx))

IPPAPI(IppStatus, ippsRSASetKey,(const IppsBigNumState* pKey, IppRSAKeyTag tag, IppsRSAState* pCtx))
IPPAPI(IppStatus, ippsRSAGetKey,(IppsBigNumState* pKey, IppRSAKeyTag tag, const IppsRSAState* pCtx))

IPPAPI(IppStatus, ippsRSAGenerate,(IppsBigNumState* pE,
                                   int kn, int kp, int nTrials, IppsRSAState* pCtx,
                                   IppBitSupplier rndFunc, void* pRndParam))
IPPAPI(IppStatus, ippsRSAValidate,(const IppsBigNumState* pE,
                                   int nTrials, Ipp32u* pResult, IppsRSAState* pCtx,
                                   IppBitSupplier rndFunc, void* pRndParam))

IPPAPI(IppStatus, ippsRSAEncrypt,(const IppsBigNumState* pX, IppsBigNumState* pY, IppsRSAState* pCtx))
IPPAPI(IppStatus, ippsRSADecrypt,(const IppsBigNumState* pX, IppsBigNumState* pY, IppsRSAState* pCtx))

/* RSA schemes: RSAES-OAEP */
IPPAPI(IppStatus, ippsRSAOAEPEncrypt,(const Ipp8u* pSrc, int srcLen,
                                      const Ipp8u* pLabel, int labLen,
                                      const Ipp8u* pSeed,
                                            Ipp8u* pDst, IppsRSAState* pRSA,
                                      IppHASH hashFun, int hashLen,
                                      IppMGF mgfFunc))
IPPAPI(IppStatus, ippsRSAOAEPEncrypt_SHA1,(const Ipp8u* pSrc, int srcLen,
                                       const Ipp8u* pLabel, int labLen,
                                       const Ipp8u* pSeed,
                                             Ipp8u* pDst, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSAOAEPEncrypt_SHA224,(const Ipp8u* pSrc, int srcLen,
                                       const Ipp8u* pLabel, int labLen,
                                       const Ipp8u* pSeed,
                                             Ipp8u* pDst, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSAOAEPEncrypt_SHA256,(const Ipp8u* pSrc, int srcLen,
                                       const Ipp8u* pLabel, int labLen,
                                       const Ipp8u* pSeed,
                                             Ipp8u* pDst, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSAOAEPEncrypt_SHA384,(const Ipp8u* pSrc, int srcLen,
                                       const Ipp8u* pLabel, int labLen,
                                       const Ipp8u* pSeed,
                                             Ipp8u* pDst, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSAOAEPEncrypt_SHA512,(const Ipp8u* pSrc, int srcLen,
                                       const Ipp8u* pLabel, int labLen,
                                       const Ipp8u* pSeed,
                                             Ipp8u* pDst, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSAOAEPEncrypt_MD5,(const Ipp8u* pSrc, int srcLen,
                                       const Ipp8u* pLabel, int labLen,
                                       const Ipp8u* pSeed,
                                             Ipp8u* pDst, IppsRSAState* pRSA))

IPPAPI(IppStatus, ippsRSAOAEPDecrypt,(const Ipp8u* pSrc,
                                      const Ipp8u* pLabel, int labLen,
                                            Ipp8u* pDst, int* pDstLen, IppsRSAState* pRSA,
                                      IppHASH hashFun, int hashLen,
                                      IppMGF mgfFunc))
IPPAPI(IppStatus, ippsRSAOAEPDecrypt_SHA1,(const Ipp8u* pSrc,
                                       const Ipp8u* pLabel, int labLen,
                                             Ipp8u* pDst, int* pDstLen, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSAOAEPDecrypt_SHA224,(const Ipp8u* pSrc,
                                       const Ipp8u* pLabel, int labLen,
                                             Ipp8u* pDst, int* pDstLen, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSAOAEPDecrypt_SHA256,(const Ipp8u* pSrc,
                                       const Ipp8u* pLabel, int labLen,
                                             Ipp8u* pDst, int* pDstLen, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSAOAEPDecrypt_SHA384,(const Ipp8u* pSrc,
                                       const Ipp8u* pLabel, int labLen,
                                             Ipp8u* pDst, int* pDstLen, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSAOAEPDecrypt_SHA512,(const Ipp8u* pSrc,
                                       const Ipp8u* pLabel, int labLen,
                                             Ipp8u* pDst, int* pDstLen, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSAOAEPDecrypt_MD5,(const Ipp8u* pSrc,
                                       const Ipp8u* pLabel, int labLen,
                                             Ipp8u* pDst, int* pDstLen, IppsRSAState* pRSA))

/* RSA schemes: RSASSA-PKCS_v1_5 */
IPPAPI(IppStatus, ippsRSAEncrypt_PKCSv15,(const Ipp8u* pSrc, Ipp32u srcLen,
                                          const Ipp8u* pRandPS,
                                                Ipp8u* pDst, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSADecrypt_PKCSv15,(const Ipp8u* pSrc,
                                                Ipp8u* pDst, Ipp32u* pDstLen, IppsRSAState* pRSA))

/* RSA schemes: RSASSA-PSS */
IPPAPI(IppStatus, ippsRSASSASign,(const Ipp8u* pHMsg, int hashLen,
                                  const Ipp8u* pSalt, int saltLen,
                                        Ipp8u* pSign, IppsRSAState* pRSA,
                                  IppHASH hashFun, IppMGF mgfFun))
IPPAPI(IppStatus, ippsRSASSASign_SHA1,(const Ipp8u* pHMsg,
                                  const Ipp8u* pSalt, int saltLen,
                                        Ipp8u* pSign, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSASign_SHA224,(const Ipp8u* pHMsg,
                                  const Ipp8u* pSalt, int saltLen,
                                        Ipp8u* pSign, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSASign_SHA256,(const Ipp8u* pHMsg,
                                  const Ipp8u* pSalt, int saltLen,
                                        Ipp8u* pSign, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSASign_SHA384,(const Ipp8u* pHMsg,
                                  const Ipp8u* pSalt, int saltLen,
                                        Ipp8u* pSign, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSASign_SHA512,(const Ipp8u* pHMsg,
                                  const Ipp8u* pSalt, int saltLen,
                                        Ipp8u* pSign, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSASign_MD5,(const Ipp8u* pHMsg,
                                  const Ipp8u* pSalt, int saltLen,
                                        Ipp8u* pSign, IppsRSAState* pRSA))

IPPAPI(IppStatus, ippsRSASSAVerify,(const Ipp8u* pHMsg, int hashLen,
                                    const Ipp8u* pSign,
                                    IppBool* pIsValid, IppsRSAState* pRSA,
                                    IppHASH hashFun, IppMGF mgfFun))
IPPAPI(IppStatus, ippsRSASSAVerify_SHA1,(const Ipp8u* pHMsg,
                                    const Ipp8u* pSign,
                                    IppBool* pIsValid, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSAVerify_SHA224,(const Ipp8u* pHMsg,
                                    const Ipp8u* pSign,
                                    IppBool* pIsValid, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSAVerify_SHA256,(const Ipp8u* pHMsg,
                                    const Ipp8u* pSign,
                                    IppBool* pIsValid, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSAVerify_SHA384,(const Ipp8u* pHMsg,
                                    const Ipp8u* pSign,
                                    IppBool* pIsValid, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSAVerify_SHA512,(const Ipp8u* pHMsg,
                                    const Ipp8u* pSign,
                                    IppBool* pIsValid, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSAVerify_MD5,(const Ipp8u* pHMsg,
                                    const Ipp8u* pSign,
                                    IppBool* pIsValid, IppsRSAState* pRSA))

/* RSA schemes: RSASSA-PSS-PKCS-v1_5 */
IPPAPI(IppStatus, ippsRSASSASign_SHA1_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                               Ipp8u* pSign, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSASign_SHA224_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                 Ipp8u* pSign, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSASign_SHA256_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                 Ipp8u* pSign, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSASign_SHA384_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                 Ipp8u* pSign, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSASign_SHA512_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                 Ipp8u* pSign, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSASign_MD5_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                              Ipp8u* pSign, IppsRSAState* pRSA))

IPPAPI(IppStatus, ippsRSASSAVerify_SHA1_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                 const Ipp8u* pSign,
                                                 IppBool* pIsValid, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSAVerify_SHA224_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                   const Ipp8u* pSign,
                                                   IppBool* pIsValid, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSAVerify_SHA256_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                   const Ipp8u* pSign,
                                                   IppBool* pIsValid, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSAVerify_SHA384_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                   const Ipp8u* pSign,
                                                   IppBool* pIsValid, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSAVerify_SHA512_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                   const Ipp8u* pSign,
                                                   IppBool* pIsValid, IppsRSAState* pRSA))
IPPAPI(IppStatus, ippsRSASSAVerify_MD5_PKCSv15,(const Ipp8u* pMsg, Ipp32u msgLen,
                                                const Ipp8u* pSign,
                                                IppBool* pIsValid, IppsRSAState* pRSA))

/*
// =========================================================
// DL Cryptography
// =========================================================
*/
IPPAPI( const char*, ippsDLGetResultString, (IppDLResult code))

/* Initialization */
IPPAPI(IppStatus, ippsDLPGetSize,(int bitSizeP, int bitSizeR, int* pSize))
IPPAPI(IppStatus, ippsDLPInit,   (int bitSizeP, int bitSizeR, IppsDLPState* pCtx))

IPPAPI(IppStatus, ippsDLPPack,(const IppsDLPState* pCtx, Ipp8u* pBuffer))
IPPAPI(IppStatus, ippsDLPUnpack,(const Ipp8u* pBuffer, IppsDLPState* pCtx))

/* Set Up and Retrieve Domain Parameters */
IPPAPI(IppStatus, ippsDLPSet,(const IppsBigNumState* pP,
                              const IppsBigNumState* pR,
                              const IppsBigNumState* pG,
                              IppsDLPState* pCtx))
IPPAPI(IppStatus, ippsDLPGet,(IppsBigNumState* pP,
                              IppsBigNumState* pR,
                              IppsBigNumState* pG,
                              IppsDLPState* pCtx))
IPPAPI(IppStatus, ippsDLPSetDP,(const IppsBigNumState* pDP, IppDLPKeyTag tag, IppsDLPState* pCtx))
IPPAPI(IppStatus, ippsDLPGetDP,(IppsBigNumState* pDP, IppDLPKeyTag tag, const IppsDLPState* pCtx))

/* Key Generation, Validation and Set Up */
IPPAPI(IppStatus, ippsDLPGenKeyPair,(IppsBigNumState* pPrvKey, IppsBigNumState* pPubKey,
                                     IppsDLPState* pCtx,
                                     IppBitSupplier rndFunc, void* pRndParam))
IPPAPI(IppStatus, ippsDLPPublicKey, (const IppsBigNumState* pPrvKey,
                                     IppsBigNumState* pPubKey,
                                     IppsDLPState* pCtx))
IPPAPI(IppStatus, ippsDLPValidateKeyPair,(const IppsBigNumState* pPrvKey,
                                     const IppsBigNumState* pPubKey,
                                     IppDLResult* pResult,
                                     IppsDLPState* pCtx))

IPPAPI(IppStatus, ippsDLPSetKeyPair,(const IppsBigNumState* pPrvKey,
                                     const IppsBigNumState* pPubKey,
                                     IppsDLPState* pCtx))

/* Singing/Verifying (DSA version) */
IPPAPI(IppStatus, ippsDLPSignDSA,  (const IppsBigNumState* pMsgDigest,
                                    const IppsBigNumState* pPrvKey,
                                    IppsBigNumState* pSignR, IppsBigNumState* pSignS,
                                    IppsDLPState* pCtx))
IPPAPI(IppStatus, ippsDLPVerifyDSA,(const IppsBigNumState* pMsgDigest,
                                    const IppsBigNumState* pSignR, const IppsBigNumState* pSignS,
                                    IppDLResult* pResult,
                                    IppsDLPState* pCtx))

/* Shared Secret Element (DH version) */
IPPAPI(IppStatus, ippsDLPSharedSecretDH,(const IppsBigNumState* pPrvKeyA,
                                         const IppsBigNumState* pPubKeyB,
                                         IppsBigNumState* pShare,
                                         IppsDLPState* pCtx))

/* DSA's parameter Generation and Validation */
IPPAPI(IppStatus, ippsDLPGenerateDSA,(const IppsBigNumState* pSeedIn,
                                      int nTrials, IppsDLPState* pCtx,
                                      IppsBigNumState* pSeedOut, int* pCounter,
                                      IppBitSupplier rndFunc, void* pRndParam))
IPPAPI(IppStatus, ippsDLPValidateDSA,(int nTrials, IppDLResult* pResult, IppsDLPState* pCtx,
                                      IppBitSupplier rndFunc, void* pRndParam))

/* DH parameter's Generation and Validation */
IPPAPI(IppStatus, ippsDLPGenerateDH,(const IppsBigNumState* pSeedIn,
                                     int nTrials, IppsDLPState* pCtx,
                                     IppsBigNumState* pSeedOut, int* pCounter,
                                     IppBitSupplier rndFunc, void* pRndParam))
IPPAPI(IppStatus, ippsDLPValidateDH,(int nTrials, IppDLResult* pResult, IppsDLPState* pCtx,
                                     IppBitSupplier rndFunc, void* pRndParam))


/*
// =========================================================
// EC Cryptography
// =========================================================
*/
IPPAPI( const char*, ippsECCGetResultString, (IppECResult code))

/*
// EC over Prime Fields
*/
/* Initialization */
IPPAPI(IppStatus, ippsECCPGetSize,(int feBitSize, int* pSize))
IPPAPI(IppStatus, ippsECCPInit,(int feBitSize, IppsECCPState* pECC))

/*  Setup and retrieve and validation of Domain Parameters */
IPPAPI(IppStatus, ippsECCPSet,(const IppsBigNumState* pPrime,
                               const IppsBigNumState* pA, const IppsBigNumState* pB,
                               const IppsBigNumState* pGX,const IppsBigNumState* pGY,const IppsBigNumState* pOrder,
                               int cofactor,
                               IppsECCPState* pECC))

IPPAPI(IppStatus, ippsECCPSetStd,(IppECCType flag, IppsECCPState* pECC))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCPGet,(IppsBigNumState* pPrime,
                               IppsBigNumState* pA, IppsBigNumState* pB,
                               IppsBigNumState* pGX,IppsBigNumState* pGY,IppsBigNumState* pOrder,
                               int* cofactor,
                               IppsECCPState* pECC))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCPGetOrderBitSize,(int* pBitSize, IppsECCPState* pECC))

IPPAPI(IppStatus, ippsECCPValidate,(int nTrials, IppECResult* pResult, IppsECCPState* pECC,
                                    IppBitSupplier rndFunc, void* pRndParam))

/* EC Point */
IPPAPI(IppStatus, ippsECCPPointGetSize,(int feBitSize, int* pSize))

IPPAPI(IppStatus, ippsECCPPointInit,(int feBitSize, IppsECCPPointState* pPoint))

/* Setup/retrieve point's coordinates */
IPPAPI(IppStatus, ippsECCPSetPoint,(const IppsBigNumState* pX, const IppsBigNumState* pY,
                                    IppsECCPPointState* pPoint, IppsECCPState* pECC))

IPPAPI(IppStatus, ippsECCPSetPointAtInfinity,(IppsECCPPointState* pPoint, IppsECCPState* pECC))

IPPAPI(IppStatus, ippsECCPGetPoint,(IppsBigNumState* pX, IppsBigNumState* pY,
                                    const IppsECCPPointState* pPoint, IppsECCPState* pECC))

/* EC Point Operations */
IPPAPI(IppStatus, ippsECCPCheckPoint,(const IppsECCPPointState* pP,
                                      IppECResult* pResult, IppsECCPState* pECC))
IPPAPI(IppStatus, ippsECCPComparePoint,(const IppsECCPPointState* pP, const IppsECCPPointState* pQ,
                                        IppECResult* pResult, IppsECCPState* pECC))
IPPAPI(IppStatus, ippsECCPNegativePoint,(const IppsECCPPointState* pP,
                                         IppsECCPPointState* pR, IppsECCPState* pECC))
IPPAPI(IppStatus, ippsECCPAddPoint,(const IppsECCPPointState* pP, const IppsECCPPointState* pQ,
                                    IppsECCPPointState* pR, IppsECCPState* pECC))
IPPAPI(IppStatus, ippsECCPMulPointScalar,(const IppsECCPPointState* pP, const IppsBigNumState* pK,
                                          IppsECCPPointState* pR, IppsECCPState* pECC))

/* Key Generation, Setup and Validation */
IPPAPI(IppStatus, ippsECCPGenKeyPair,(IppsBigNumState* pPrivate, IppsECCPPointState* pPublic,
                                      IppsECCPState* pECC,
                                      IppBitSupplier rndFunc, void* pRndParam))
IPPAPI(IppStatus, ippsECCPPublicKey,(const IppsBigNumState* pPrivate,
                                     IppsECCPPointState* pPublic,
                                     IppsECCPState* pECC))
IPPAPI(IppStatus, ippsECCPValidateKeyPair,(const IppsBigNumState* pPrivate, const IppsECCPPointState* pPublic,
                                           IppECResult* pResult,
                                           IppsECCPState* pECC))
IPPAPI(IppStatus, ippsECCPSetKeyPair,(const IppsBigNumState* pPrivate, const IppsECCPPointState* pPublic,
                                      IppBool regular,
                                      IppsECCPState* pECC))

/* Shared Secret (DH scheme ) */
IPPAPI(IppStatus, ippsECCPSharedSecretDH,(const IppsBigNumState* pPrivateA,
                                          const IppsECCPPointState* pPublicB,
                                          IppsBigNumState* pShare,
                                          IppsECCPState* pECC))
IPPAPI(IppStatus, ippsECCPSharedSecretDHC,(const IppsBigNumState* pPrivateA,
                                           const IppsECCPPointState* pPublicB,
                                           IppsBigNumState* pShare,
                                           IppsECCPState* pECC))

/* Sing/Verify */
IPPAPI(IppStatus, ippsECCPSignDSA,  (const IppsBigNumState* pMsgDigest,
                                     const IppsBigNumState* pPrivate,
                                     IppsBigNumState* pSignX, IppsBigNumState* pSignY,
                                     IppsECCPState* pECC))
IPPAPI(IppStatus, ippsECCPVerifyDSA,(const IppsBigNumState* pMsgDigest,
                                     const IppsBigNumState* pSignX, const IppsBigNumState* pSignY,
                                     IppECResult* pResult,
                                     IppsECCPState* pECC))

IPPAPI(IppStatus, ippsECCPSignNR,  (const IppsBigNumState* pMsgDigest,
                                    const IppsBigNumState* pPrivate,
                                    IppsBigNumState* pSignX, IppsBigNumState* pSignY,
                                    IppsECCPState* pECC))
IPPAPI(IppStatus, ippsECCPVerifyNR,(const IppsBigNumState* pMsgDigest,
                                    const IppsBigNumState* pSignX, const IppsBigNumState* pSignY,
                                    IppECResult* pResult,
                                    IppsECCPState* pECC))

/*
// EC over Binary Fields
*/
/* Initialization */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBGetSize,(int feBitSize, int* pSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBInit,(int feBitSize, IppsECCBState* pECC))

/*  Setup and retrieve and validation of Domain Parameters */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBSet,(const IppsBigNumState* pPrime,
                               const IppsBigNumState* pA, const IppsBigNumState* pB,
                               const IppsBigNumState* pGX,const IppsBigNumState* pGY,const IppsBigNumState* pOrder,
                               int cofactor,
                               IppsECCBState* pECC))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBSetStd,(IppECCType flag, IppsECCBState* pECC))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBGet,(IppsBigNumState* pPrime,
                               IppsBigNumState* pA, IppsBigNumState* pB,
                               IppsBigNumState* pGX,IppsBigNumState* pGY,IppsBigNumState* pOrder,
                               int* cofactor,
                               IppsECCBState* pECC))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBGetOrderBitSize,(int* pBitSize, IppsECCBState* pECC))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBValidate,(int nTrials, IppECResult* pResult, IppsECCBState* pECC,
                                    IppBitSupplier rndFunc, void* pRndParam))

/* EC Point */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBPointGetSize,(int feBitSize, int* pSize))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBPointInit,(int feBitSize, IppsECCBPointState* pPoint))

/* Setup/retrieve point's coordinates */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBSetPoint,(const IppsBigNumState* pX, const IppsBigNumState* pY,
                                    IppsECCBPointState* pPoint, IppsECCBState* pECC))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBSetPointAtInfinity,(IppsECCBPointState* pPoint, IppsECCBState* pECC))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBGetPoint,(IppsBigNumState* pX, IppsBigNumState* pY,
                                    const IppsECCBPointState* pPoint, IppsECCBState* pECC))

/* EC Point Operations */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBCheckPoint,(const IppsECCBPointState* pP,
                                      IppECResult* pResult, IppsECCBState* pECC))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBComparePoint,(const IppsECCBPointState* pP, const IppsECCBPointState* pQ,
                                        IppECResult* pResult, const IppsECCBState* pECC))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBNegativePoint,(const IppsECCBPointState* pP,
                                         IppsECCBPointState* pR, IppsECCBState* pECC))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBAddPoint,(const IppsECCBPointState* pP, const IppsECCBPointState* pQ,
                                    IppsECCBPointState* pR, IppsECCBState* pECC))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBMulPointScalar,(const IppsECCBPointState* pP, const IppsBigNumState* pK,
                                          IppsECCBPointState* pR, IppsECCBState* pECC))

/* Key Generation, Setup and Validation */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBGenKeyPair,(IppsBigNumState* pPrivate, IppsECCBPointState* pPublic,
                                      IppsECCBState* pECC,
                                      IppBitSupplier rndFunc, void* pRndParam))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBPublicKey,(const IppsBigNumState* pPrivate,
                                     IppsECCBPointState* pPublic,
                                     IppsECCBState* pECC))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBValidateKeyPair,(const IppsBigNumState* pPrivate, const IppsECCBPointState* pPublic,
                                           IppECResult* pResult,
                                           IppsECCBState* pECC))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBSetKeyPair,(const IppsBigNumState* pPrivate, const IppsECCBPointState* pPublic,
                                      IppBool regular,
                                      IppsECCBState* pECC))

/* Shared Secret (DH scheme ) */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBSharedSecretDH,(const IppsBigNumState* pPrivateA,
                                          const IppsECCBPointState* pPublicB,
                                          IppsBigNumState* pShare,
                                          IppsECCBState* pECC))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBSharedSecretDHC,(const IppsBigNumState* pPrivateA,
                                           const IppsECCBPointState* pPublicB,
                                           IppsBigNumState* pShare,
                                           IppsECCBState* pECC))

/* Sing/Verify */
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBSignDSA,  (const IppsBigNumState* pMsgDigest,
                                     const IppsBigNumState* pPrivate,
                                     IppsBigNumState* pSignX, IppsBigNumState* pSignY,
                                     IppsECCBState* pECC))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBVerifyDSA,(const IppsBigNumState* pMsgDigest,
                                     const IppsBigNumState* pSignX, const IppsBigNumState* pSignY,
                                     IppECResult* pResult,
                                     IppsECCBState* pECC))

IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBSignNR,  (const IppsBigNumState* pMsgDigest,
                                    const IppsBigNumState* pPrivate,
                                    IppsBigNumState* pSignX, IppsBigNumState* pSignY,
                                    IppsECCBState* pECC))
IPP_DEPRECATED("is deprecated. This function is obsolete and will be removed in one of the future IPP releases. Use the following link for details: http://software.intel.com/en-us/articles/intel-ipp-71-deprecated-features/")\
IPPAPI(IppStatus, ippsECCBVerifyNR,(const IppsBigNumState* pMsgDigest,
                                    const IppsBigNumState* pSignX, const IppsBigNumState* pSignY,
                                    IppECResult* pResult,
                                    IppsECCBState* pECC))

#ifdef  __cplusplus
}
#endif


#if defined (_IPP_STDCALL_CDECL)
  #undef  _IPP_STDCALL_CDECL
  #define __stdcall __cdecl
#endif


#endif /* __IPPCP_H__ */
