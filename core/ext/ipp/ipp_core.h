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

#include "../../rt/small_math.h"
#include "../../rt/buffer_type.h"
#include "../../os/file_dir.h"


#include "ipp_config.h"

#ifdef PLATFORM_INTEL_IPP_SUPPORT
#include "inc/ipp.h"

///////////////////////////////////////////////////
//Helper macro 
#define IPPVERIFY(x) VERIFY(0 == (x))	// ippStsNoErr == 0
#define IPPI_INTER_DEFAULT IPPI_INTER_LINEAR

namespace ipp
{

enum _tagAxisOrientation
{
	AxisHorizontal = ippAxsHorizontal,
	AxisVertical = ippAxsVertical,
	AxisBoth = ippAxsBoth
};

} // namespace ipp

#else // PLATFORM_INTEL_IPP_SUPPORT

#include "inc/ippdefs.h"

typedef unsigned char   Ipp8u;
typedef unsigned short  Ipp16u;
typedef unsigned int    Ipp32u;

typedef signed char		Ipp8s;
typedef signed short	Ipp16s;
typedef signed int		Ipp32s;
typedef float			Ipp32f;
typedef LONGLONG		Ipp64s;
typedef ULONGLONG		Ipp64u;
typedef double			Ipp64f;

#endif // PLATFORM_INTEL_IPP_SUPPORT

namespace ipp
{

/////////////////////////////////////////////////////////////////
// IppEnv
enum _tagIppEnvOption
{
#ifdef PLATFORM_INTEL_IPP_SUPPORT
	DitherMode_None = ippDitherNone,
	DitherMode_FS = ippDitherFS,
	DitherMode_JJN = ippDitherJJN,
	DitherMode_Stucki = ippDitherStucki,
	DitherMode_Bayer = ippDitherBayer,

	BlendMethod_AlphaOver = ippAlphaOver,		//OVER	¦Á(A)*A+[1-¦Á(A)]*¦Á(B)*B 
	BlendMethod_AlphaIn   = ippAlphaIn,			//IN	¦Á(A)*A*¦Á(B)
	BlendMethod_AlphaOut  = ippAlphaOut,		//OUT	¦Á(A)*A*[1-¦Á(B)]
	BlendMethod_AlphaATop = ippAlphaATop,		//ATOP	¦Á(A)*A*¦Á(B)+[1-¦Á(A)]*¦Á(B)*B 
	BlendMethod_AlphaXor  = ippAlphaXor,		//XOR	¦Á(A)*A*[1-¦Á(B)]+[1-¦Á(A)]*¦Á(B)*B 
	BlendMethod_AlphaPlus = ippAlphaPlus,		//PLUS	¦Á(A)*A + ¦Á(B)*B
#endif

	RoundMode_Zero = ippRndZero,
	RoundMode_Near = ippRndNear,
	
	HintAlgorithm_Default  = ippAlgHintNone,
	HintAlgorithm_Fast  = ippAlgHintFast,
	HintAlgorithm_Accurate  = ippAlgHintAccurate,

	InterpolationMode_Nearest = IPPI_INTER_NN,
	InterpolationMode_Linear = IPPI_INTER_LINEAR,
	InterpolationMode_Cubic = IPPI_INTER_CUBIC,
	InterpolationMode_Super = IPPI_INTER_SUPER,
	InterpolationMode_SmoothEdge = IPPI_SMOOTH_EDGE,

	IppiOption_Max
};


class IppiEnvParam
{	
	friend IppiEnvParam * GetEnv();
public:

#ifdef PLATFORM_INTEL_IPP_SUPPORT
	IppiDitherType		DitherMode;
#endif

	IppRoundMode		RoundMode;
	IppHintAlgorithm	HintAlgorithm;
	IppiBorderType		BorderType;
	DWORD				InterpolationMode;
	float				FloatMin;
	float				FloatMax;
	int					ResultBitShift;  // return result*2^(-ResultBitShift)
	int					IntegerScaleFactor;

	int					JpegEncodeQuality;
	int					GifEncodeColorCount;

protected:
	static IppiEnvParam	g_IppEnv;
	LPBYTE				m_pEnvParamStack;
	int					m_StackPointer;

public:
	~IppiEnvParam();
	IppiEnvParam();
	void	Push();
	void	Pop();
};
INLFUNC IppiEnvParam * GetEnv(){ return &IppiEnvParam::g_IppEnv; }


enum _tagImageCodec
{
	ImageCodec_Auto = 0,
	ImageCodec_PNG,
	ImageCodec_JPG,
	ImageCodec_GIF,
	ImageCodec_GIF_ANI,
	ImageCodec_PFM,
	ImageCodec_EXR,			
	ImageCodec_EXR_PIZ = ImageCodec_EXR,		// exr with wavelet, lossy
	ImageCodec_EXR_ZIP,		// exr with zlib, lossless
	ImageCodec_EXR_PXR24,	// exr with lossy 24-bit float compression
	ImageCodec_EXR_END,
	//ImageCodec_BMP,
	ImageCodec_Unk = -1,
};

enum _tagCodecFlag
{
	JPEG_ColorSampleNONE    = 0,    /* Corresponds to "No Subsampling". */
									/* Valid on a JPEG w/ any number of channels. */
	JPEG_ColorSample411     = 1,    /* Valid on a JPEG w/ 3 channels. */
	JPEG_ColorSample422     = 2,    /* Valid on a JPEG w/ 3 channels. */
};

namespace _details
{
class ImageCodec
{
protected:
	rt::Buffer<BYTE>	m_TempBuffer;
	int					m_BufferUsedLen;
	bool				_SetBufferSize(int size){ m_BufferUsedLen=0; return m_TempBuffer.SetSize(rt::max((UINT)size,(UINT)m_TempBuffer.GetSize())); }
public:
	ImageCodec(){ m_BufferUsedLen = 0; }
	LPCBYTE				GetOutput()const { return m_TempBuffer; }
	UINT				GetOutputSize()const { return m_BufferUsedLen; }
};
} // namespace _details

class ImageDecoder: public _details::ImageCodec
{
	int		m_DecodedImageWidth;
	int		m_DecodedImageHeight;
	int		m_DecodedImageStep;
	int		m_DecodedImageChannel;
	int		m_FrameCount;
	DWORD	m_ImageCodec;

public:
	ImageDecoder(){ m_DecodedImageWidth = m_DecodedImageHeight = m_DecodedImageStep = m_DecodedImageChannel = 0; }
	//bool	DecodeHeader(LPCBYTE image, UINT len, DWORD image_codec = ImageCodec_Auto);
	static	_tagImageCodec	DecodeFormat(LPCBYTE image, UINT DataLen);
	bool	Decode(LPCVOID image, UINT len, DWORD image_codec = ImageCodec_Auto);

	UINT	GetImageWidth()const { return m_DecodedImageWidth; }
	UINT	GetImageHeight()const { return m_DecodedImageHeight; }
	UINT	GetImageStep()const { return m_DecodedImageStep; }
	UINT	GetImageChannel()const { return m_DecodedImageChannel; }
	UINT	GetImageCodec()const { return m_ImageCodec; }
	UINT	GetFrameCount() const { return m_FrameCount; }

	LPCBYTE	GetOutput(UINT frame = 0)const { return m_TempBuffer.Begin() + frame*m_DecodedImageStep*m_DecodedImageHeight; }
	UINT	GetOutputSize()const { return m_DecodedImageStep*m_DecodedImageHeight; }
};

class ImageEncoder: public _details::ImageCodec
{
	int		m_Quality;
	int		m_Flag;
public:
	ImageEncoder(){ m_Quality = 95; m_Flag = 0; }
	void	SetQualityRatio(int quality){ ASSERT(quality<=100 && quality>=0); m_Quality = quality; }
	void	SetSubSamplingType(int	mode = 0){ m_Flag = mode; }

	bool	Encode(LPCBYTE pData,int Channel,int Width,int Height,int Step, DWORD codec = ImageCodec_JPG);	// codec:=_tagImageCodec
	static _tagImageCodec CodecFromExtName(const rt::String_Ref& filename);
};

} // namespace ipp

namespace ipp
{

struct Size:public IppiSize
{
	FORCEINL Size(){}
	FORCEINL Size(const IppiSize&x){ width=x.width; height=x.height;  }
	FORCEINL Size(int w,int h){ width = w; height = h; }
	FORCEINL Size(int s){ width = s; height = s; }
	FORCEINL Size AddBorder(int border_x,int border_y) const
	{	return Size(width+border_x*2,height+border_y*2); 
	}
	FORCEINL Size AddBorder(int border) const
	{	return AddBorder(border,border);
	}
	FORCEINL Size ScaleTo(int min_sz) const	// making the short dimension to min_sz
	{	if(width > height)return Size((int)(width*min_sz/height + 0.5f), min_sz);
		else{ return Size(min_sz, (int)(height*min_sz/width + 0.5f)); }
	}
	FORCEINL Size ScaleTo(int max_width, int max_height) const	// making both w <= max_width and h <= max_height
	{	int neww = width*max_height/height;
		if(neww <= max_width){ return Size(neww, max_height); }
		else{ return Size(max_width, height*max_width/width); }
	}
	FORCEINL bool operator == (const Size& x) const
	{	return width == x.width && height == x.height;
	}
	FORCEINL int	Area() const { return width*height; }
	FORCEINL int GetShortDimen() const { return rt::min(width, height); }
};

struct Point:public IppiPoint
{
	FORCEINL Point(){}
	FORCEINL Point(int ix,int iy){ x = ix; y = iy; }
	FORCEINL bool operator == (const Point& q) const
	{	return x == q.x && y == q.y;
	}
	FORCEINL Point Translate(int dx, int dy) const { return Point(x+dx, y+dy); }
	FORCEINL Point Translate(const rt::Vec2i& m) const { return Point(x+m.x, y+m.y); }
};

struct Rect:public IppiRect
{
	FORCEINL Rect(){}
	FORCEINL Rect(int ix,int iy,int w,int h){ x = ix; y = iy; width = w; height = h; }
	FORCEINL Rect(const Point& pt,int w,int h){ x = pt.x; y = pt.y; width = w; height = h; }
	FORCEINL Rect(const Point& pt1,const Point& pt2)
	{	if(pt1.x < pt2.x){ x=pt1.x; width=pt2.x-pt1.x; }
		else{ x=pt2.x; width=pt1.x-pt2.x; }
		if(pt1.y < pt2.y){ y=pt1.y; height=pt2.y-pt1.y; }
		else{ y=pt2.y; height=pt1.y-pt2.y; }
	}
	FORCEINL int Area() const { return width*height; }
	FORCEINL Point& Position(){ return *((Point*)this); }
	FORCEINL const Point& Position() const { return *((Point*)this); }
	FORCEINL bool IsHit(const Point& p) const { return p.x>=x && p.x<=x+width && p.y>=y && p.y<=y+height; }
	FORCEINL void BoundingBoxOf(const Rect& rect1, const Rect& rect2)
	{	int xx = rt::min(rect1.x, rect2.x);	int yy = rt::min(rect1.y, rect2.y);
		width = rt::max(rect1.x+rect1.width, rect2.x+rect2.width) - xx;
		height = rt::max(rect1.y+rect1.height, rect2.y+rect2.height) - yy;
		x = xx;
		y = yy;
	}
};

// Type definitions
#define IPP_TYPE_EXT(x) typedef x * LP##x; typedef const x * LPC##x;
	IPP_TYPE_EXT(Ipp8u);
	IPP_TYPE_EXT(Ipp8s);
	IPP_TYPE_EXT(Ipp16u);
	IPP_TYPE_EXT(Ipp16s);
	IPP_TYPE_EXT(Ipp32u);
	IPP_TYPE_EXT(Ipp32s);
	IPP_TYPE_EXT(Ipp32f);
	IPP_TYPE_EXT(Ipp64s);
	IPP_TYPE_EXT(Ipp64f);
#undef IPP_TYPE_EXT


namespace ipp_cpp
{
// Haar wavelet transform
IppStatus ippiHaarWTInv_C1R(LPCIpp8u pSrc,int srcStep, LPIpp8u pDst,int dstStep, IppiSize roi);
IppStatus ippiHaarWTFwd_C1R(LPCIpp8u pSrc,int srcStep, LPIpp8u pDst,int dstStep, IppiSize roi);
IppStatus ippiHaarWTInv_C1R(LPCIpp32f pSrc,int srcStep, LPIpp32f pDst,int dstStep, IppiSize roi);
IppStatus ippiHaarWTFwd_C1R(LPCIpp32f pSrc,int srcStep, LPIpp32f pDst,int dstStep, IppiSize roi);
}


namespace image_codec
{
	class _PFM_Header
	{	friend bool		_Open_PFM(LPCSTR fn,_PFM_Header* pHeader);
		friend bool		_Read_PFM(const _PFM_Header* pHeader,LPFLOAT pData,UINT ch,UINT step);
		os::File		file;
	public:
		UINT			width;
		UINT			height;
		UINT			ch;
	};
	extern bool _Write_PFM(LPCSTR fn,LPCFLOAT pData,UINT ch,UINT w,UINT h,UINT step);
	extern bool _Open_PFM(LPCSTR fn,_PFM_Header* pHeader);
	extern bool _Read_PFM(const _PFM_Header* pHeader,LPFLOAT pData,UINT ch,UINT step);
}


} // namespace ipp

