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

#include "ipp_core.h"
// OpenEXR
#include "openexr/IlmImf/ImfOutputFile.h"
#include "openexr/IlmImf/ImfInputFile.h"
#include "openexr/IlmImf/ImfChannelList.h"

#include "../../os/file_dir.h"
#include "../../rt/runtime_base.h"
#include <math.h>

#ifdef PLATFORM_INTEL_IPP_SUPPORT
#include "inc/ippi_cpp.h"
#include "inc/ipps_cpp.h"
#endif


#define ASSERT_SIZE(x1,x2) ASSERT((x1).GetWidth() == (x2).GetWidth() && (x1).GetHeight() == (x2).GetHeight())

#define IPPARG_IMG(x)	 ((x).GetImageData()),((int)(x).GetStep())
#define IPPARG_IMG2(x)	 ((x).GetImageData()),(x),((int)(x).GetStep())

namespace ipp
{

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

namespace _details
{

template<typename t_Value,UINT Channel>
struct _ValueType
{	typedef rt::Vec<t_Value,Channel> t_Val;
};
	template<typename t_Value>
	struct _ValueType<t_Value, 1>
	{	typedef t_Value t_Val;
	};

template<typename t_Value>
struct _ValueEnum;
	template<> struct _ValueEnum<BYTE>
	{	static BYTE	One(){ return 255; }
	};
	template<> struct _ValueEnum<float>
	{	static float One(){ return 1.0f; }
	};


} // namespace _details


template<typename t_Value,UINT Channel>
class Signal_Ref
{
public:
	typedef typename _details::_ValueType<t_Value,Channel>::t_Val t_Val;
	static const int chan_num = Channel;

	typedef t_Val*			LPValueType;
	typedef const t_Val*	LPCValueType;

	typedef Signal_Ref<t_Value,Channel>	Ref;

protected:
	t_Val*	_p;
	UINT	_len;

public:
	FORCEINL static UINT	GetChannels(){ return chan_num;}
	FORCEINL static UINT	GetBPS(){ return (sizeof(t_Val)/chan_num)<<3;}
	FORCEINL UINT           GetLength() const { return _len; }
	FORCEINL LPVOID         GetBits(){ return _p; }
	FORCEINL LPCVOID		GetBits() const { return _p; }

	FORCEINL Signal_Ref(const t_Val* p = nullptr, UINT len = 0){ _p = (t_Val*)p; _len = len; }
	FORCEINL Signal_Ref(const Signal_Ref& x):Signal_Ref(x._p, x._len){}

	template<typename T,UINT ch>
	FORCEINL const Signal_Ref& operator = (const Signal_Ref<T,ch> & x){ CopyFrom(x); return *this; }

	FORCEINL t_Val& operator()(int x){ return _p[x]; }
	FORCEINL const t_Val& operator()(int x) const { return _p[x]; }

	FORCEINL Ref GetRef(UINT x,UINT len) const { ASSERT(x+len<=GetLength()); return Ref(&_p[x],len); }
	FORCEINL Ref GetRef_Inside(UINT border) const {	ASSERT(border*2<GetLength()); return GetSubSignal(border,GetLength()-border*2);	}

	FORCEINL bool IsEmpty() const { _p == nullptr || _len == 0; }

	FORCEINL operator LPValueType (){ return _p; }
	FORCEINL operator LPCValueType () const { return _p; }

#ifdef PLATFORM_INTEL_IPP_SUPPORT
	FORCEINL void Zero(){ ipp_cpp::ippsZero(_p, _len); }
#endif
};

template<typename t_Val,UINT Channel>
class Signal: public Signal_Ref<t_Val, Channel>
{
	typedef Signal_Ref<t_Val, Channel> _SC;
	FORCEINL void __SafeFree()
	{	
#ifdef PLATFORM_INTEL_IPP_SUPPORT
		if(_SC::_p){ IPPCALL(ippsFree)(_SC::_p); _SC::_p=nullptr; }
#else
		_SafeFree32AL(_SC::_p);
#endif
	}
public:
	typedef Signal_Ref<t_Val,Channel>	Ref;
public:
	FORCEINL Signal(){}
	FORCEINL Signal(const Signal_Ref<t_Val, Channel>& x){ Ref::CopyFrom(x); }
	INLFUNC  bool SetLength(UINT co)
	{	if(co == _SC::_len){ return true; }
		else
		{	__SafeFree();
			if(co)
			{	
#ifdef PLATFORM_INTEL_IPP_SUPPORT
				switch(sizeof(t_Val)*8)
				{
				case 8:  // 1c8u
					_SC::_p = (t_Val*)IPPCALL(ippsMalloc_8u)((int)co); break;
				case 16: // 2c8u,1c16u
					_SC::_p = (t_Val*)IPPCALL(ippsMalloc_16u)((int)co); break;
				case 32: // 2c16u,1c32f,1c32s
					_SC::_p = (t_Val*)IPPCALL(ippsMalloc_32f)((int)co); break;
				case 64: // 2c32f,1c64f
					_SC::_p = (t_Val*)IPPCALL(ippsMalloc_64f)((int)co); break;
				case 128:// 2c64f
					_SC::_p = (t_Val*)IPPCALL(ippsMalloc_64fc)((int)co); break;
				default:
					ASSERT(0); //unsupported content format
				}
#else
				_SC::_p = _Malloc32AL(BYTE, co*sizeof(_SC::t_Val));
#endif
				if(_SC::_p == nullptr){ _SC::_len = 0; return false; }
			}
			_SC::_len = co;
			return true;
		}
	}
};


template<typename t_Value,UINT Channel>
class Volume_Ref
{
public:
	typedef rt::Vec<t_Value,Channel> t_Val;
	static const int chan_num = Channel;

	typedef t_Value*		LPValueType;
	typedef const t_Value*	LPCValueType;

	typedef Volume_Ref<t_Value,Channel>	Ref;

protected:
	t_Val*	_p;
	UINT	Width,Height,Depth;
	UINT	StepSize_WH;
	UINT	StepSize_H;

public:
	FORCEINL static UINT	GetChannels(){ return chan_num;}
	FORCEINL static UINT	GetBPS(){ return (sizeof(t_Val)/chan_num)<<3;}
	FORCEINL UINT			GetWidth() const { return Width; }
	FORCEINL UINT			GetHeight() const { return Height; }
	FORCEINL UINT			GetDepth() const { return Depth; }
	FORCEINL LPVOID			GetBits(){ return _p; }
	FORCEINL LPCVOID		GetBits() const { return _p; }

	FORCEINL Volume_Ref(const t_Val* p = nullptr, UINT w = 0,UINT h = 0,UINT d = 0,UINT step_wh = 0,UINT step_h = 0){ _p = (t_Val*)p; Width = w; Height = h; Depth = d; StepSize_WH = step_wh; StepSize_H = step_h; }
	FORCEINL Volume_Ref(const Volume_Ref& x):Volume_Ref(x._p, x.Width, x.Height, x.Depth, x.StepSize_WH, x.StepSize_H){}

	template<typename T,UINT ch>
	FORCEINL const Volume_Ref& operator = (const Volume_Ref<T,ch> & x){ CopyFrom(x); return *this; }

	FORCEINL Volume_Ref& Set(t_Value v)
	{	for(UINT z = 0; z<Depth; z++)for(UINT y = 0; y<Height; y++)for(UINT x = 0; x<Width; x++)
			(*this)(x,y,z) = v;
		return *this;
	}

	//FORCEINL Ref GetRef(UINT x,UINT len) const { ASSERT(x+len<=GetLength()); return Ref(&_p[x],len); }
	//FORCEINL Ref GetRef_Inside(UINT border) const {	ASSERT(border*2<GetLength()); return GetSubSignal(border,GetLength()-border*2);	}

	FORCEINL t_Val& operator()(int x, int y, int z){ return *(x + (t_Val*)&((LPBYTE)_p)[y*StepSize_H + z*StepSize_WH]); }
	FORCEINL const t_Val& operator()(int x, int y, int z) const { return *(x + (const t_Val*)&((LPCBYTE)_p)[x + y*StepSize_H + z*StepSize_WH]); }
	FORCEINL bool IsEmpty() const { _p == nullptr || Width == 0 || Height == 0 || Depth == 0; }
};

template<typename t_Value,UINT Channel>
class Volume: public Volume_Ref<t_Value, Channel>
{
	typedef Volume_Ref<t_Value, Channel> _SC;
	FORCEINL void __SafeFree(){ _SafeFree32AL(_SC::_p); }
public:
	typedef Volume_Ref<t_Value,Channel>	Ref;
public:
	FORCEINL Volume(){}
	FORCEINL Volume(const Signal_Ref<t_Value, Channel>& x){ SetSize(x); Ref::CopyFrom(x); }
	INLFUNC  bool SetSize(UINT w, UINT h, UINT d)
	{	if(w == _SC::Width && h == _SC::Height && d == _SC::Depth){ return true; }
		else
		{	__SafeFree();
			_SC::Width = _SC::Height = _SC::Depth = 0;
			if(w && h && d)
			{	
				int step_h = _EnlargeTo32AL(sizeof(t_Value)*Channel*w);
				_SC::_p = (_Malloc32AL(BYTE, step_h*h*d));
				if(_SC::_p)
				{	_SC::Width = d; _SC::Height = h; _SC::Depth = d;
					_SC::StepSize_WH = step_h*_SC::Height;
					_SC::StepSize_H = step_h;
				}else return false;
			}
			return true;
		}
	}
};


#ifdef PLATFORM_INTEL_IPP_SUPPORT

namespace _details
{
template<typename T> struct _HistogramLevelType;
template<> struct _HistogramLevelType<Ipp32f>{ typedef Ipp32f t_Result; };
template<> struct _HistogramLevelType<Ipp8u>{ typedef Ipp32s t_Result; };
template<> struct _HistogramLevelType<Ipp8s>{ typedef Ipp32s t_Result; };
template<> struct _HistogramLevelType<Ipp16u>{ typedef Ipp32s t_Result; };
template<> struct _HistogramLevelType<Ipp16s>{ typedef Ipp32s t_Result; };
template<> struct _HistogramLevelType<Ipp32u>{ typedef Ipp32u t_Result; };
template<> struct _HistogramLevelType<Ipp32s>{ typedef Ipp32s t_Result; };
};

#endif

template<typename t_Value,int Channel>
class Image;

template<typename t_Value,int Channel>
class Image_Ref
{
public:
	typedef rt::Vec<t_Value,Channel>	t_Val;
	typedef t_Value						ValueType;
	typedef t_Value*					LPValueType;
	typedef const t_Value*				LPCValueType;
	static const int chan_num = Channel;

protected:
	UINT	Width;
	UINT	Height;
	UINT	Step_Bytes; //Step = Width*sizeof(t_Val) + Pad_Bytes
	t_Val*	lpData;

	FORCEINL t_Value*		GetValueAddress(UINT x, UINT y){ return (t_Value*)GetPixelAddress(x,y); }
	FORCEINL const t_Value*	GetValueAddress(UINT x, UINT y)const{ return (const t_Value*)GetPixelAddress(x,y); }

public:
	FORCEINL void Attach(LPCVOID p = nullptr, UINT w = 0, UINT h = 0, UINT step = 0){ Step_Bytes = step?step:w*Channel*sizeof(t_Value); Width = w; Height = h; lpData = (t_Val*)p; }
	FORCEINL void Attach(const Image_Ref& x){ Attach(x.lpData, x.Width, x.Height, x.Step_Bytes); }
	Image_Ref(LPCVOID p = nullptr, UINT w = 0, UINT h = 0, UINT step = 0){ Attach(p,w,h,step); }
	Image_Ref(const Image_Ref& x){ Attach(x); }
	operator const IppiSize& () const { return (IppiSize&)(*this); }
	template<typename T,int ch>
	FORCEINL const Image_Ref<t_Value, Channel>& operator = (const Image_Ref<T,ch> & x){ CopyFrom(x); return *this; }
	FORCEINL const Image_Ref<t_Value, Channel>& operator = (const Image_Ref<t_Value, Channel>& x){ CopyFrom(x); return *this; }

	FORCEINL static UINT	GetChannels(){ return chan_num;}
	FORCEINL static UINT	GetBPV(){ return sizeof(t_Value)<<3;}
	FORCEINL static UINT	GetBPP(){ return Channel*sizeof(t_Value)<<3;}
	FORCEINL UINT			GetWidth() const { return Width; }
	FORCEINL UINT			GetHeight() const { return Height; }
	FORCEINL UINT			GetStep() const { return Step_Bytes; }	// in Bytes
	FORCEINL LPVOID			GetBits(){ return lpData; }
	FORCEINL LPCVOID		GetBits() const { return lpData; }
	FORCEINL bool			IsEmpty() const { return lpData == nullptr || Width == 0 || Height == 0; }
	FORCEINL t_Val &		operator ()(UINT x, UINT y){ return *(t_Val*)(&((LPBYTE)lpData)[y*Step_Bytes+x*sizeof(t_Val)]); }
	FORCEINL const t_Val &	operator ()(UINT x, UINT y) const{ return *(t_Val*)(&((LPBYTE)lpData)[y*Step_Bytes+x*sizeof(t_Val)]); }
	FORCEINL t_Val*			GetPixelAddress(UINT x, UINT y){ return &(*this)(x,y); }
	FORCEINL const t_Val*	GetPixelAddress(UINT x, UINT y) const { return &(*this)(x,y); }
	FORCEINL t_Val*			GetScanline(UINT y){ return (t_Val*)(((LPBYTE)lpData) + y*Step_Bytes); }
	FORCEINL const t_Val*	GetScanline(UINT y) const { return (const t_Val*)(((LPCBYTE)lpData) + y*Step_Bytes); }
	FORCEINL t_Val			GetInterplotedPixel(float x, float y) const
	{	t_Val	ret;
		if(x>0 && x<Width-1 && y>0 && y<Height-1)
		{	int ix = (int)x;
			int iy = (int)y;
			ret.Interpolate((*this)(ix, iy), (*this)(ix+1, iy), (*this)(ix, iy+1), (*this)(ix+1, iy+1), x - ix, y - iy);
			return ret;
		}
		if(x>0 && x<Width-1)
		{
			t_Val	ret;	int ix = (int)x;
			if(y<0)
				ret.Interpolate((*this)(ix,0), (*this)(ix+1,0), x - ix);
			else
				ret.Interpolate((*this)(ix,Height-1), (*this)(ix+1,Height-1), x - ix);
			return ret;
		}
		if(y>0 && y<Height-1)
		{
			t_Val	ret;	int iy = (int)y;
			if(x<0)
				ret.Interpolate((*this)(0, iy), (*this)(0, iy+1), y - iy);
			else
				ret.Interpolate((*this)(Width-1,iy), (*this)(Width-1,iy+1), y - iy);
			return ret;
		}
		if(x<0 && y<0)
		{	return (*this)(0,0);
		}
		else if(x>=Width-1 && y<0)
		{	return (*this)(Width-1,0);
		}
		else if(x<0 && y>=Height-1)
		{	return (*this)(0,Height-1);
		}
		else
		{	return (*this)(Width-1,Height-1);
		}
	}
public:
	typedef Image_Ref<t_Value,Channel>	Ref;
	typedef Image_Ref<t_Value,1>		Ref_1C;

	FORCEINL Ref GetSub(const Size& sz){ return Ref(lpData,sz.width,sz.height,GetStep());	}
	FORCEINL Ref GetSub(UINT w,UINT h){ return Ref(lpData,w,h,GetStep()); }
	FORCEINL Ref Crop(int x,int y,UINT w,UINT h)
	{	if(x<0){ w += x; x=0; }
		if(y<0){ h += y; y=0; }
		return Ref(GetPixelAddress(x,y),rt::min(Width - x,w),rt::min(Height - y,h),GetStep());
	}
	FORCEINL const Ref Crop(int x,int y,UINT w,UINT h) const { return rt::_CastToNonconst(this)->Crop(x,y,w,h); }
	FORCEINL const Ref GetSub(UINT x,UINT y,UINT w,UINT h) const
	{	return rt::_CastToNonconst(this)->GetSub(x,y,w,h); }
	FORCEINL Ref GetSub(UINT x,UINT y,UINT w,UINT h)
	{	ASSERT(x + w <= Width);
		ASSERT(y + h <= Height);
		return Ref(GetPixelAddress(x,y),w,h,GetStep());
	}
	
	FORCEINL Ref GetSub_Inside(UINT border)
	{	ASSERT(border*2<GetWidth());
		ASSERT(border*2<GetHeight());
		return GetSub(border,border,GetWidth()-border*2,GetHeight()-border*2);
	}
	FORCEINL const Ref GetSub_Inside(UINT border) const
	{	return rt::_CastToNonconst(this)->GetSub_Inside(border); }

	FORCEINL Ref GetSub_Inside(UINT border_x,UINT border_y)
	{	ASSERT(border_x*2<GetWidth());
		ASSERT(border_y*2<GetHeight());
		return GetSub(border_x,border_y,GetWidth()-border_x*2,GetHeight()-border_y*2);
	}
	FORCEINL const Ref GetSub_Inside(UINT border_x,UINT border_y) const
	{	return rt::_CastToNonconst(this)->GetSub_Inside(border_x,border_y); 
	}
	FORCEINL Ref GetSub_MaxBox(UINT aspect_x = 1, UINT asqect_y = 1)
	{	if(GetWidth()*asqect_y <= GetHeight()*aspect_x)
		{	int h = GetWidth()*asqect_y/aspect_x;
			return GetSub(0, (GetHeight()-h)/2, GetWidth(), h);
		}
		else
		{	int w = GetHeight()*aspect_x/asqect_y;
			return GetSub((GetWidth()-w)/2, 0, w, GetHeight());
		}
	}
public:
	FORCEINL const Size&		GetRegion()const{ return *((const Size*)this); }
	FORCEINL LPValueType		GetImageData() const { return (LPValueType)GetBits(); }

public:
	INLFUNC  bool Save(LPCSTR fn, _tagImageCodec ic = ImageCodec_Auto) const	// ipp::GetEnv()->JpegEncodeQuality for controling of the quality of the jpeg file
	{	if(IsEmpty())return false;
		if(ic == ImageCodec_Auto)ic = ImageEncoder::CodecFromExtName(fn);
		if(ic == ImageCodec_PFM)
		{
			if(	rt::IsTypeSame<float, t_Value>::Result)
			{	return image_codec::_Write_PFM(fn, (LPCFLOAT)this->lpData, Channel, GetWidth(), GetHeight(), GetStep());	}
			else
			{	Image<float, Channel> img_temp;
				img_temp.SetSize(GetWidth(), GetHeight());
				img_temp.CopyFrom(*this);
				return img_temp.Save(fn, ic);
			}
		}
		else if(ic>=ImageCodec_EXR && ic<ImageCodec_EXR_END)
		{
			Imf::Compression cm;
			switch(ic)
			{	case ImageCodec_EXR_ZIP:	cm = Imf::ZIP_COMPRESSION;		break;
				case ImageCodec_EXR_PXR24:	cm = Imf::PXR24_COMPRESSION;	break;
				case ImageCodec_EXR_PIZ:	cm = Imf::PIZ_COMPRESSION;		break;
				default: return false;
			}

			Imf::PixelType pt;
			if(rt::IsTypeSame<float, t_Value>::Result)
			{	pt = Imf::FLOAT;
			}
			else if(rt::IsTypeSame<UINT, t_Value>::Result)
			{	pt = Imf::UINT;
			}
			else
			{	Image<float, Channel> img_temp;
				img_temp.SetSize(GetWidth(), GetHeight());
				img_temp.CopyFrom(*this);
				return img_temp.Save(fn, ic);
			}

			Imf::Header header(GetWidth(), GetHeight(), 1.0f, Imath::V2f(0,0), 1.0f, Imf::INCREASING_Y, cm);
			Imf::FrameBuffer frameBuffer;
			switch(Channel)
			{	case 1: header.channels().insert ("G", Imf::Channel (pt));
						frameBuffer.insert ("G", Imf::Slice(pt,	(char *) lpData, sizeof(t_Val),	GetStep()));
						break;
				case 2:	header.channels().insert ("G", Imf::Channel (pt));
						header.channels().insert ("Z", Imf::Channel (pt));
						frameBuffer.insert ("G", Imf::Slice(pt,	(char *)lpData					  , sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("Z", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value), sizeof(t_Val),	GetStep()));
						break;
				case 3: header.channels().insert ("R", Imf::Channel (pt));
						header.channels().insert ("G", Imf::Channel (pt));
						header.channels().insert ("B", Imf::Channel (pt));
						frameBuffer.insert ("R", Imf::Slice(pt,	(char *)lpData					    , sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("G", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value)  , sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("B", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value)*2, sizeof(t_Val),	GetStep()));
						break;
				case 4:	header.channels().insert ("R", Imf::Channel (pt));
						header.channels().insert ("G", Imf::Channel (pt));
						header.channels().insert ("B", Imf::Channel (pt));
						header.channels().insert ("A", Imf::Channel (pt));
						frameBuffer.insert ("R", Imf::Slice(pt,	(char *)lpData					    , sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("G", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value)  , sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("B", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value)*2, sizeof(t_Val),	GetStep()));
						frameBuffer.insert ("A", Imf::Slice(pt,	((char *)lpData) + sizeof(t_Value)*3, sizeof(t_Val),	GetStep()));
						break;
				default: return false;
			}

			try{
				os::File::CreateDirectories(fn);
				Imf::OutputFile file(fn, header);
				file.setFrameBuffer(frameBuffer);
				file.writePixels(GetHeight());
				return true;
			}
			catch(...){ return false; }
		}
		else if(ic != ImageCodec_Unk)
		{	// LDR formats
			if(rt::IsTypeSame<BYTE, t_Value>::Result)
			{
				os::File		file;
				ImageEncoder	enc;

				if(ic == ImageCodec_JPG)
					enc.SetQualityRatio(GetEnv()->JpegEncodeQuality);

				if(	enc.Encode((LPCBYTE)lpData, chan_num, GetWidth(), GetHeight(), GetStep(), ic) &&
					file.Open(fn, os::File::Normal_Write, true) &&
					file.Write(enc.GetOutput(), enc.GetOutputSize()) == enc.GetOutputSize()
				){	return true; }
				else
				{	file.Close();
					os::File::Remove(fn);
					return false;
				}
			}
			else
			{	Image<BYTE, Channel> img_temp;
				img_temp.SetSize(GetWidth(), GetHeight());
				img_temp.CopyFrom(*this);
				return img_temp.Save(fn, ic);
			}
		}
		return false;
	}

public:
	template<typename VAL, int CHAN2>
	FORCEINL void CopyFrom(const Image_Ref<VAL,CHAN2> & in)
	{	ASSERT_SIZE(*this, in);
		for(UINT y=0;y<Height;y++)
		{	t_Val* d = GetScanline(y);
			t_Val* dend = d + Width;
			const typename Image_Ref<VAL,CHAN2>::t_Val* s = in.GetScanline(y);
			for(;d<dend;d++,s++)*d = *s;
	}	}
#ifndef PLATFORM_INTEL_IPP_SUPPORT
	template<typename VAL>
	FORCEINL void Mean(rt::Vec<VAL, Channel>& mean) const
	{	mean = 0;
		for(UINT y=0;y<Height;y++)
			for(UINT x=0;x<Width;x++)
				mean += (*this)(x,y);
		mean /= Width*Height;
	}
	template<typename VAL>
	FORCEINL void Sum(rt::Vec<VAL, Channel>& sum) const
	{	rt::Vec<double, Channel> sumv(0);
		for(UINT y=0;y<Height;y++)
			for(UINT x=0;x<Width;x++)
				sumv += (*this)(x,y);
		sum = sumv;
	}
	FORCEINL void Set(const t_Val& v)
	{	for(UINT y=0;y<Height;y++)
			for(UINT x=0;x<Width;x++)
				(*this)(x,y) = v;
	}
	FORCEINL void MinMax(t_Val& vmin, t_Val& vmax) const
	{	vmin = vmax = (*this)(0,0);
		for(UINT y=0;y<Height;y++)
			for(UINT x=0;x<Width;x++)
			{	const t_Val& p = (*this)(x,y);
				vmin.Min(vmin, p);
				vmax.Max(vmax, p);
			}
	}
	FORCEINL void CopyFrom(const Ref & in)
	{	ASSERT_SIZE(*this, in);
		for(UINT y=0;y<Height;y++)
			memcpy(GetScanline(y), in.GetScanline(y), sizeof(t_Val)*Width);
	}
	FORCEINL void Zero()
	{	for(UINT y=0;y<Height;y++)
			memset(GetScanline(y), 0, sizeof(t_Val)*Width);
	}
	template<typename VAL>
	FORCEINL void Add(const Image_Ref<VAL,Channel>& in)
	{	ASSERT_SIZE(*this, in);
		for(UINT y=0;y<Height;y++)
		{	t_Val* d = GetScanline(y);
			t_Val* dend = d + Width;
			const typename Image_Ref<VAL,Channel>::t_Val* s = in.GetScanline(y);
			for(;d<dend;d++,s++)*d += *s;
	}	}
#else
	FORCEINL void CopyFrom(const Ref & in)
	{	ASSERT_SIZE(*this, in);
		switch(chan_num)
		{
		case 1: ipp::ipp_cpp::ippiCopy_C1R(IPPARG_IMG(in),IPPARG_IMG(*this),*this); return;
		case 2:	switch(sizeof(t_Value)*8){
				case 8:	IPPCALL(ippiCopy_16s_C1R)((const Ipp16s*)in.GetBits(),in.GetStep(),(Ipp16s*)GetBits(),GetStep(),*this); return;
				case 16:IPPCALL(ippiCopy_32f_C1R)((const Ipp32f*)in.GetBits(),in.GetStep(),(Ipp32f*)GetBits(),GetStep(),*this); return;
				case 32:IPPCALL(ippiCopy_16s_C4R)((const Ipp16s*)in.GetBits(),in.GetStep(),(Ipp16s*)GetBits(),GetStep(),*this); return;
				} break;
		case 3: ::ipp::ipp_cpp::ippiCopy_C3R(IPPARG_IMG(in),IPPARG_IMG(*this),*this); return;
		case 4: ::ipp::ipp_cpp::ippiCopy_C4R(IPPARG_IMG(in),IPPARG_IMG(*this),*this); return;
		default: ASSERT(0);
	}	}
	template<typename VAL>
	FORCEINL void CopyFrom(const Image_Ref<VAL,chan_num> & in)
	{	ASSERT_SIZE(*this, in);
		switch(chan_num)
		{
		case 1: ::ipp::ipp_cpp::ippiScale_C1R(IPPARG_IMG(in),IPPARG_IMG(*this),*this); return;
		case 3: ::ipp::ipp_cpp::ippiScale_C3R(IPPARG_IMG(in),IPPARG_IMG(*this),*this); return;
		case 4: ::ipp::ipp_cpp::ippiScale_C4R(IPPARG_IMG(in),IPPARG_IMG(*this),*this); return;
		default: ASSERT(0);
	}	}
	template<int chan_num2>
	FORCEINL void CopyFrom(const Image_Ref<t_Value,chan_num2> & in)
	{	ASSERT_SIZE(*this, in);
		switch(chan_num2)
		{
		case 1: //1->x
			switch(chan_num) 
			{	case 3: {	const ValueType* const SrcImg[3] = { in.GetImageData(),in.GetImageData(),in.GetImageData() };
							ipp::ipp_cpp::ippiCopy_P3C3R(SrcImg,in.GetStep(),IPPARG_IMG(*this),*this);
						}	return;
				case 4:	{	const ValueType* const SrcImg[4] = { in.GetImageData(),in.GetImageData(),in.GetImageData(),in.GetImageData() };
							ipp::ipp_cpp::ippiCopy_P4C4R(SrcImg,in.GetStep(),IPPARG_IMG(*this),*this);
						}	return;
                default: ASSERT(0);
 			} break;
		case 3: //3->x
			switch(chan_num)
			{	case 1: ipp::ipp_cpp::ippiRGBToGray_C3C1R(IPPARG_IMG(in),IPPARG_IMG(*this),*this); return;
				case 4: ipp::ipp_cpp::ippiSet_C1R(_details::_ValueEnum<t_Value>::One(),IPPARG_IMG(*this), Size(GetWidth()*4,GetHeight()));
						ipp::ipp_cpp::ippiCopy_C3AC4R(IPPARG_IMG(in),IPPARG_IMG(*this),*this);
						return;
                default: ASSERT(0);
			} break;
		case 4: //4->x
			switch(chan_num)
			{	case 1: ipp::ipp_cpp::ippiRGBToGray_AC4C1R(IPPARG_IMG(in),IPPARG_IMG(*this),*this); return;
				case 3: ipp::ipp_cpp::ippiCopy_AC4C3R(IPPARG_IMG(in),IPPARG_IMG(*this),*this); return;
                default: ASSERT(0);
			} break;
        default: ASSERT(0);
		}
	}
	
	template<typename t_Val2,int ch>	//conversion images if possible
	void CopyTo( Image_Ref<t_Val2,ch>& in ) const { in.CopyFrom(*this); }

public:
	FORCEINL void BoxFilter(const Size& isize)
	{	Size size = isize;
		if(size.width&1){}else{ size.width++; }
		if(size.height&1){}else{ size.height++; }
		Point pt(size.width>>1,size.height>>1);
		Ref win = GetSub_Inside(pt.x,pt.y);
		switch(chan_num) {
			case 1: ipp_cpp::ippiFilterBox_C1R(IPPARG_IMG(win),win,size,pt); break;
			case 3: ipp_cpp::ippiFilterBox_C3R(IPPARG_IMG(win),win,size,pt); break;
			case 4: ipp_cpp::ippiFilterBox_C4R(IPPARG_IMG(win),win,size,pt); break;
	}	}
	FORCEINL void BoxFilterTo(Image_Ref& dst, const Size& isize)
	{	Size size = isize;
		if(size.width&1){}else{ size.width++; }
		if(size.height&1){}else{ size.height++; }
		Point pt(size.width>>1,size.height>>1);
		Ref win = GetSub_Inside(pt.x,pt.y);
		switch(chan_num) {
			case 1: ipp_cpp::ippiFilterBox_C1R(IPPARG_IMG(win),IPPARG_IMG(dst),win,size,pt); break;
			case 3: ipp_cpp::ippiFilterBox_C3R(IPPARG_IMG(win),IPPARG_IMG(dst),win,size,pt); break;
			case 4: ipp_cpp::ippiFilterBox_C4R(IPPARG_IMG(win),IPPARG_IMG(dst),win,size,pt); break;
	}	}
	FORCEINL void RightShift(Ipp32u count)
	{	switch(chan_num) {
			case 1: ipp_cpp::ippiRShift_C1R(IPPARG_IMG(*this),count,IPPARG_IMG(*this), *this); break;
			case 3: {	Ipp32u c[3] = { count,count,count };
						ipp_cpp::ippiRShift_C3R(IPPARG_IMG(*this),c,IPPARG_IMG(*this), *this); break;
					}
			case 4: {	Ipp32u c[4] = { count,count,count,count };
						ipp_cpp::ippiRShift_C4R(IPPARG_IMG(*this),c,IPPARG_IMG(*this), *this); break;
					}
	}	}
	FORCEINL void LeftShift(Ipp32u count)
	{	switch(chan_num) {
			case 1: ipp_cpp::ippiLShift_C1R(IPPARG_IMG(*this),count,IPPARG_IMG(*this), *this); break;
			case 3: {	Ipp32u c[3] = { count,count,count };
						ipp_cpp::ippiLShift_C3R(IPPARG_IMG(*this),c,IPPARG_IMG(*this), *this); break;
					}
			case 4: {	Ipp32u c[4] = { count,count,count,count };
						ipp_cpp::ippiLShift_C4R(IPPARG_IMG(*this),c,IPPARG_IMG(*this), *this); break;
					}
	}	}
	FORCEINL void Gamma()
	{	switch(chan_num) {	
			case 1: ASSERT(0); break;
			case 3: ipp_cpp::ippiGammaFwd_C3R(IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiGammaFwd_AC4R(IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void GammaTo(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {	
			case 1: ASSERT(0); break;
			case 3: ipp_cpp::ippiGammaFwd_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiGammaFwd_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void GammaInv()
	{	switch(chan_num) {	
			case 1: ASSERT(0); break;
			case 3: ipp_cpp::ippiGammaInv_C3R(IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiGammaInv_AC4R(IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void GammaInvTo(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {	
			case 1: ASSERT(0); break;
			case 3: ipp_cpp::ippiGammaInv_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiGammaInv_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void HaarWaveletFwd(int level) // dimension much be power of two
	{	{	int mask = (1<<level)-1;
			ASSERT((GetWidth()&mask)==0);
			ASSERT((GetHeight()&mask)==0);
			ASSERT(chan_num == 1);
		}
		ipp::Image<t_Value,chan_num>	aux;
		aux.SetSize(GetWidth(),GetHeight());
		IppiSize	roi = *this;
		for(int i=0;i<level;i++)
		{	ipp_cpp::ippiHaarWTFwd_C1R(IPPARG_IMG(*this),IPPARG_IMG(aux),roi);
			*this = aux;
			roi.width>>=1;
			roi.height>>=1;
	}	}
	FORCEINL void HaarWaveletInv(int level) // dimension much be power of two
	{	{	int mask = (1<<level)-1;
			ASSERT((GetWidth()&mask)==0);
			ASSERT((GetHeight()&mask)==0);
			ASSERT(chan_num == 1);
		}
		ipp::Image<t_Value,chan_num>	aux;
		aux.SetSize(GetWidth(),GetHeight());
		IppiSize	roi = *this;
		roi.width>>=level;
		roi.height>>=level;
		for(int i=0;i<level;i++)
		{	roi.width<<=1;
			roi.height<<=1;
			ipp_cpp::ippiHaarWTInv_C1R(IPPARG_IMG(*this),IPPARG_IMG(aux),roi);
			GetSub(roi) = aux.GetSub(roi);
	}	}
	INLFUNC void ResizeTo_Nearest(Image_Ref& dst) const
	{	int specSize, initSize, bufSize;
		ipp_cpp::ippiResizeGetSize<ValueType>(*this,dst,ippNearest, 0, &specSize, &initSize);
		IppiResizeSpec_32f* pSpec=(IppiResizeSpec_32f*)_Alloca32AL(specSize);
		ipp_cpp::ippiResizeNearestInit<ValueType>(*this, dst, pSpec);
		ipp_cpp::ippiResizeGetBufferSize<ValueType>(pSpec,dst,chan_num,&bufSize);
		Ipp8u* pBuffer=IPPCALL(ippsMalloc_8u)(bufSize);
		switch(chan_num)
		{	case 1: ipp_cpp::ippiResizeNearest_C1R(IPPARG_IMG(*this), IPPARG_IMG(dst), Point(0,0), dst, pSpec, pBuffer); break;
			case 3: ipp_cpp::ippiResizeNearest_C3R(IPPARG_IMG(*this), IPPARG_IMG(dst), Point(0,0), dst, pSpec, pBuffer); break;
			case 4: ipp_cpp::ippiResizeNearest_C4R(IPPARG_IMG(*this), IPPARG_IMG(dst), Point(0,0), dst, pSpec, pBuffer); break;
			default: ASSERT(0);
		}
		IPPCALL(ippsFree)(pBuffer);
	}
	INLFUNC void ResizeTo_Bilinear(Image_Ref& dst) const
	{	int specSize, initSize, bufSize;
		ipp_cpp::ippiResizeGetSize<ValueType>(*this,dst,ippLinear, 0, &specSize, &initSize);
		IppiResizeSpec_32f* pSpec=(IppiResizeSpec_32f*)_Alloca32AL(specSize);
		ipp_cpp::ippiResizeLinearInit<ValueType>(*this, dst, pSpec);
		ipp_cpp::ippiResizeGetBufferSize<ValueType>(pSpec,dst,chan_num,&bufSize);
		Ipp8u* pBuffer=IPPCALL(ippsMalloc_8u)(bufSize);
		switch(chan_num)
		{	case 1: ipp_cpp::ippiResizeLinear_C1R(IPPARG_IMG(*this), IPPARG_IMG(dst), Point(0,0), dst, pSpec, pBuffer); break;
			case 3: ipp_cpp::ippiResizeLinear_C3R(IPPARG_IMG(*this), IPPARG_IMG(dst), Point(0,0), dst, pSpec, pBuffer); break;
			case 4: ipp_cpp::ippiResizeLinear_C4R(IPPARG_IMG(*this), IPPARG_IMG(dst), Point(0,0), dst, pSpec, pBuffer); break;
			default: ASSERT(0);
		}
		IPPCALL(ippsFree)(pBuffer);
	}
	INLFUNC void ResizeTo_SuperSampling(Image_Ref& dst) const
	{	if(dst.GetWidth()*dst.GetHeight() < GetWidth()*GetHeight())
		{
			int specSize, initSize, bufSize;
			ipp_cpp::ippiResizeGetSize<ValueType>(*this,dst,ippSuper, 0, &specSize, &initSize);
			IppiResizeSpec_32f* pSpec=(IppiResizeSpec_32f*)_Alloca32AL(specSize);
			ipp_cpp::ippiResizeSuperInit<ValueType>(*this, dst, pSpec);
			ipp_cpp::ippiResizeGetBufferSize<ValueType>(pSpec,dst,chan_num,&bufSize);
			Ipp8u* pBuffer=IPPCALL(ippsMalloc_8u)(bufSize);
			switch(chan_num)
			{	case 1: ipp_cpp::ippiResizeSuper_C1R(IPPARG_IMG(*this), IPPARG_IMG(dst), Point(0,0), dst, pSpec, pBuffer); break;
				case 3: ipp_cpp::ippiResizeSuper_C3R(IPPARG_IMG(*this), IPPARG_IMG(dst), Point(0,0), dst, pSpec, pBuffer); break;
				case 4: ipp_cpp::ippiResizeSuper_C4R(IPPARG_IMG(*this), IPPARG_IMG(dst), Point(0,0), dst, pSpec, pBuffer); break;
				default: ASSERT(0);
			}
			IPPCALL(ippsFree)(pBuffer);
		}
		else{ ResizeTo_Bilinear(dst); }
	}
	INLFUNC void ResizeTo_9GridsSampling(Image_Ref& dst, int border_x, int border_y) const
	{	ASSERT(GetWidth() <= dst.GetWidth());	ASSERT(GetHeight() <= dst.GetHeight());
		ASSERT(GetWidth() - 2*border_x > 1);	ASSERT(GetHeight() - 2*border_y > 1);

		dst.GetSub(0,0,border_x,border_y) = GetSub(0,0,border_x,border_y);
		dst.GetSub(0,dst.GetHeight()-border_y,border_x,border_y) = GetSub(0,Height-border_y,border_x,border_y);
		dst.GetSub(dst.GetWidth()-border_x,0,border_x,border_y) = GetSub(Width-border_x,0,border_x,border_y);
		dst.GetSub(dst.GetWidth()-border_x,dst.GetHeight()-border_y,border_x,border_y) = GetSub(Width-border_x,Height-border_y,border_x,border_y);

		GetSub(0, border_y, border_x, Height-2*border_y).ResizeTo_Bilinear(dst.GetSub(0,border_y,border_x,dst.GetHeight()-2*border_y));
		GetSub(Width-border_x, border_y, border_x, Height-2*border_y).ResizeTo_Bilinear(dst.GetSub(dst.GetWidth()-border_x,border_y,border_x,dst.GetHeight()-2*border_y));
		GetSub(border_x, 0, Width-2*border_x, border_y).ResizeTo_Bilinear(dst.GetSub(border_x,0,dst.GetWidth()-2*border_x,border_y));
		GetSub(border_x, Height-border_y, Width-2*border_x, border_y).ResizeTo_Bilinear(dst.GetSub(border_x,dst.GetHeight()-border_y,dst.GetWidth()-2*border_x,border_y));

		GetSub(border_x, border_y, Width-2*border_x, Height-2*border_y).ResizeTo_Bilinear(dst.GetSub(border_x, border_y, dst.GetWidth()-2*border_x, dst.GetHeight()-2*border_y));
	}
	FORCEINL void ChannelJoin(const Ref_1C& r, const Ref_1C& g, const Ref_1C& b)
	{	ASSERT(Channel == 3);
		ASSERT_SIZE(*this,r); ASSERT_SIZE(*this,g); ASSERT_SIZE(*this,b);
		ASSERT(r.GetStep() == g.GetStep());
		ASSERT(b.GetStep() == g.GetStep());
		
		const t_Value* pCh[3] = { r.GetImageData(), g.GetImageData(), b.GetImageData() };
		ipp_cpp::ippiCopy_P3C3R(pCh,r.GetStep(),IPPARG_IMG(*this),*this);
	}
	FORCEINL void ChannelSplit(Ref_1C& r, Ref_1C& g, Ref_1C& b) const
	{	ASSERT(Channel == 3);
		ASSERT_SIZE(*this,r); ASSERT_SIZE(*this,g); ASSERT_SIZE(*this,b);
		ASSERT(r.GetStep() == g.GetStep());
		ASSERT(b.GetStep() == g.GetStep());
		
		t_Value* pCh[3] = { r.GetImageData(), g.GetImageData(), b.GetImageData() };
		ipp_cpp::ippiCopy_C3P3R(IPPARG_IMG(*this),pCh,r.GetStep(),*this);
	}
	FORCEINL void ChannelJoin(const Ref_1C& r, const Ref_1C& g, const Ref_1C& b, const Ref_1C& a)
	{	ASSERT(Channel == 4);
		ASSERT_SIZE(*this,r); ASSERT_SIZE(*this,g); ASSERT_SIZE(*this,b); ASSERT_SIZE(*this,a);
		ASSERT(r.GetStep() == g.GetStep());
		ASSERT(b.GetStep() == g.GetStep());
		ASSERT(a.GetStep() == g.GetStep());
		const t_Value* pCh[4] = { r.GetImageData(), g.GetImageData(), b.GetImageData(), a.GetImageData() };
		ipp_cpp::ippiCopy_P4C4R(pCh,r.GetStep(),IPPARG_IMG(*this),*this);
	}
	FORCEINL void ChannelSplit(Ref_1C& r, Ref_1C& g, Ref_1C& b, Ref_1C& a) const
	{	ASSERT(Channel == 4);
		ASSERT_SIZE(*this,r); ASSERT_SIZE(*this,g); ASSERT_SIZE(*this,b); ASSERT_SIZE(*this,a);
		ASSERT(r.GetStep() == g.GetStep());
		ASSERT(b.GetStep() == g.GetStep());
		ASSERT(a.GetStep() == g.GetStep());
		t_Value* pCh[4] = { r.GetImageData() , g.GetImageData(), b.GetImageData(), a.GetImageData() };
		ipp_cpp::ippiCopy_C4P4R(IPPARG_IMG(*this),pCh,r.GetStep(),*this);
	}
	/*
	FORCEINL void Normalize()	//Not tested for 8u 16s image, only used in HDR
	{	Vec<Ipp64f,chan_num>	s;
		L2Norm(s);
		if(chan_num==1)
		{	Multiply(Vec1<ValueType>((ValueType)(1.0/s.x)));
			return;
		}
		if(chan_num==3)
		{	ValueType d = (ValueType)(3.0/s.Sum()); 
			Multiply(Vec3<ValueType>(d,d,d));
			return;
		}
		if(chan_num==4)
		{	ValueType d = (ValueType)(4.0/s.Sum()); 
			Multiply(Vec4<ValueType>(d,d,d,d));
			return;
		}
	}
	*/
	template<typename t_Val2>
	FORCEINL void L2Norm(rt::Vec<t_Val2,chan_num>& out) const
	{	rt::Vec<Ipp64f,chan_num>	s;
		switch(chan_num) {
		case 1: ipp_cpp::ippiNorm_L2_C1R(IPPARG_IMG(*this),*this,s); break;
		case 3: ipp_cpp::ippiNorm_L2_C3R(IPPARG_IMG(*this),*this,s); break;
		case 4: ipp_cpp::ippiNorm_L2_C4R(IPPARG_IMG(*this),*this,s); break;
		}
		out = s;
	}
	template<typename t_Val2>
	FORCEINL void L2Norm(const Image_Ref&x, rt::Vec<t_Val2,chan_num>& out) const
	{	rt::Vec<Ipp64f,chan_num>	s;
		switch(chan_num) {
		case 1: ipp_cpp::ippiNormDiff_L2_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,s); break;
		case 3: ipp_cpp::ippiNormDiff_L2_C3R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,s); break;
		case 4: ipp_cpp::ippiNormDiff_L2_C4R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,s); break;
		}
		out = s;
	}
	FORCEINL bool operator == (const Image_Ref&x) const
	{	rt::Vec<Ipp64f,chan_num> s;
		L2Norm(x, s);
		return s.Sum()/(((double)chan_num)*(GetWidth()*GetHeight())) < rt::TypeTraits<float>::Epsilon();
	}
	template<typename t_Val2>
	FORCEINL void Sum(rt::Vec<t_Val2,chan_num>& out) const
	{	rt::Vec<Ipp64f,chan_num>	s;
		switch(chan_num) {
		case 1: ipp_cpp::ippiSum_C1R(IPPARG_IMG(*this),*this,s); break;
		case 3: ipp_cpp::ippiSum_C3R(IPPARG_IMG(*this),*this,s); break;
		case 4: ipp_cpp::ippiSum_C4R(IPPARG_IMG(*this),*this,s); break;
		}
		out = s;
	}
	template<typename t_Val2>
	FORCEINL void Mean(rt::Vec<t_Val2,chan_num>& out) const
	{	rt::Vec<Ipp64f,chan_num>	s;
		switch(chan_num) {
		case 1: ipp_cpp::ippiMean_C1R(IPPARG_IMG(*this),*this,s); break;
		case 3: ipp_cpp::ippiMean_C3R(IPPARG_IMG(*this),*this,s); break;
		case 4: ipp_cpp::ippiMean_C4R(IPPARG_IMG(*this),*this,s); break;
		}
		out = s;
	}
	/*
	template<typename t_Val2,typename t_Val3>
	FORCEINL void Deviation(rt::Vec<t_Val2,chan_num>& dev, Vec<t_Val3,chan_num>& mean) const
	{	rt::Vec<Ipp64f,chan_num>	s,d;
		switch(chan_num) {
			ippiMean_StdDev_8u_C1R
		case 1: ipp_cpp::ippiMean_StdDev_C1R(IPPARG_IMG(*this),*this,s,d); break;
		case 3: ASSERT(0); break;
		case 4: ASSERT(0); break;
		}
		mean = s;
		dev = d;
	}
	*/
	template<typename t_Val2>
	FORCEINL void Deviation(rt::Vec<t_Val2,chan_num>& dev) const
	{	rt::Vec<Ipp64f,chan_num>	d;
		switch(chan_num) {
		case 1: ipp_cpp::ippiMean_StdDev_C1R(IPPARG_IMG(*this), *this, nullptr, d); break;
		case 3: ASSERT(0); break;
		case 4: ASSERT(0); break;
		}
		dev = d;
	}
	template<typename t_Val2>
	FORCEINL void Dot(const Ref& x, rt::Vec<t_Val2,chan_num>& dotprod) const
	{	rt::Vec<Ipp64f,chan_num>	d;
		switch(chan_num) {
		case 1: ipp_cpp::ippiDotProd_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,d); break;
		case 3: ipp_cpp::ippiDotProd_C3R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,d); break;
		case 4: ipp_cpp::ippiDotProd_C4R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,d); break;
		}
		dotprod = d;
	}
	FORCEINL double Dot(const Ref& x) const
	{	rt::Vec<Ipp64f,chan_num>	d;
		switch(chan_num) {
		case 1: ipp_cpp::ippiDotProd_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,d); break;
		case 3: ipp_cpp::ippiDotProd_C3R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,d); break;
		case 4: ipp_cpp::ippiDotProd_C4R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,d); break;
		}
		return d.Sum();
	}
	FORCEINL void Min(const Ref& x)
	{	ASSERT_SIZE(x, *this);
		switch(chan_num) {
		case 1: ipp_cpp::ippiMinEvery_C1IR(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
		case 3: ipp_cpp::ippiMinEvery_C3IR(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
		case 4: ipp_cpp::ippiMinEvery_C4IR(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void Min(t_Val & p) const
	{	switch(chan_num) {
		case 1: ipp_cpp::ippiMin_C1R(IPPARG_IMG(*this),*this,p); break;
		case 3: ipp_cpp::ippiMin_C3R(IPPARG_IMG(*this),*this,p); break;
		case 4: ipp_cpp::ippiMin_C4R(IPPARG_IMG(*this),*this,p); break;
	}	}
	FORCEINL void Min(t_Val & p, rt::Vec<rt::Vec2i,chan_num> & pos) const
	{	int x[4]; int y[4];
		switch(chan_num) {
		case 1: ipp_cpp::ippiMinIndx_C1R(IPPARG_IMG(*this),*this,p,x,y);
				pos[0].x = x[0];
				pos[0].y = y[0];
				break;
		case 3: ipp_cpp::ippiMinIndx_C3R(IPPARG_IMG(*this),*this,p,x,y); 
				pos[0].x = x[0];
				pos[0].y = y[0];
				pos[1].x = x[1];
				pos[1].y = y[1];
				pos[2].x = x[2];
				pos[2].y = y[2];
				break;
		case 4: ipp_cpp::ippiMinIndx_C4R(IPPARG_IMG(*this),*this,p,x,y); 
				pos[0].x = x[0];
				pos[0].y = y[0];
				pos[1].x = x[1];
				pos[1].y = y[1];
				pos[2].x = x[2];
				pos[2].y = y[2];
				pos[3].x = x[3];
				pos[3].y = y[3];
				break;
	}	}
	FORCEINL void Max(const Ref& x)
	{	ASSERT_SIZE(x, *this);
		switch(chan_num) {
		case 1: ipp_cpp::ippiMaxEvery_C1IR(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
		case 3: ipp_cpp::ippiMaxEvery_C3IR(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
		case 4: ipp_cpp::ippiMaxEvery_C4IR(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void Max(t_Val & p) const
	{	switch(chan_num) {
		case 1: ipp_cpp::ippiMax_C1R(IPPARG_IMG(*this),*this,p); break;
		case 3: ipp_cpp::ippiMax_C3R(IPPARG_IMG(*this),*this,p); break;
		case 4: ipp_cpp::ippiMax_C4R(IPPARG_IMG(*this),*this,p); break;
	}	}
	FORCEINL void Max(t_Val & p, rt::Vec<rt::Vec2i,chan_num> & pos) const
	{	int x[4]; int y[4];
		switch(chan_num) {
		case 1: ipp_cpp::ippiMaxIndx_C1R(IPPARG_IMG(*this),*this,p,x,y);
				pos[0].x = x[0];
				pos[0].y = y[0];
				break;
		case 3: ipp_cpp::ippiMaxIndx_C3R(IPPARG_IMG(*this),*this,p,x,y); 
				pos[0].x = x[0];
				pos[0].y = y[0];
				pos[1].x = x[1];
				pos[1].y = y[1];
				pos[2].x = x[2];
				pos[2].y = y[2];
				break;
		case 4: ipp_cpp::ippiMaxIndx_C4R(IPPARG_IMG(*this),*this,p,x,y); 
				pos[0].x = x[0];
				pos[0].y = y[0];
				pos[1].x = x[1];
				pos[1].y = y[1];
				pos[2].x = x[2];
				pos[2].y = y[2];
				pos[3].x = x[3];
				pos[3].y = y[3];
				break;
	}	}
	FORCEINL void MinMax(t_Val & pmin,t_Val & pmax) const
	{	switch(chan_num) {
		case 1: ipp_cpp::ippiMinMax_C1R(IPPARG_IMG(*this),*this,pmin,pmax); break;
		case 3: ipp_cpp::ippiMinMax_C3R(IPPARG_IMG(*this),*this,pmin,pmax); break;
		case 4: ipp_cpp::ippiMinMax_C4R(IPPARG_IMG(*this),*this,pmin,pmax); break;
	}	}
	FORCEINL void Threshold_LessThan(const t_Val& v)
	{	switch(chan_num) {
		case 1: ipp_cpp::ippiThreshold_LT_C1R(IPPARG_IMG(*this),*this,v[0]); break;
		case 3: ipp_cpp::ippiThreshold_LT_C3R(IPPARG_IMG(*this),*this,v); break;
		case 4: ipp_cpp::ippiThreshold_LT_AC4R(IPPARG_IMG(*this),*this,v); break;
	}	}
	FORCEINL void ThresholdTo_LessThan(const t_Val& v,Image_Ref& x) const
	{	ASSERT_SIZE(*this,x); 
		switch(chan_num) {
		case 1: ipp_cpp::ippiThreshold_LT_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),x,v[0]); break;
		case 3: ipp_cpp::ippiThreshold_LT_C3R(IPPARG_IMG(*this),IPPARG_IMG(x),x,v); break;
		case 4: ipp_cpp::ippiThreshold_LT_AC4R(IPPARG_IMG(*this),IPPARG_IMG(x),x,v); break;
	}	}
	FORCEINL void Threshold_GreatThan(const t_Val& v)
	{	switch(chan_num) {
		case 1: ipp_cpp::ippiThreshold_GT_C1R(IPPARG_IMG(*this),*this,v[0]); break;
		case 3: ipp_cpp::ippiThreshold_GT_C3R(IPPARG_IMG(*this),*this,v); break;
		case 4: ipp_cpp::ippiThreshold_GT_AC4R(IPPARG_IMG(*this),*this,v); break;
	}	}
	FORCEINL void ThresholdTo_GreatThan(const t_Val& v,Image_Ref& x) const
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {
		case 1: ipp_cpp::ippiThreshold_GT_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),x,v[0]); break;
		case 3: ipp_cpp::ippiThreshold_GT_C3R(IPPARG_IMG(*this),IPPARG_IMG(x),x,v); break;
		case 4: ipp_cpp::ippiThreshold_GT_AC4R(IPPARG_IMG(*this),IPPARG_IMG(x),x,v); break;
	}	}
	FORCEINL void Mirror(int axis = AxisHorizontal)
	{	switch(chan_num) {
		case 1: ipp::ipp_cpp::ippiMirror_C1R(IPPARG_IMG(*this),*this,(IppiAxis)axis); break;
		case 3: ipp::ipp_cpp::ippiMirror_C3R(IPPARG_IMG(*this),*this,(IppiAxis)axis); break;
		case 4: ipp::ipp_cpp::ippiMirror_C4R(IPPARG_IMG(*this),*this,(IppiAxis)axis); break;
	}	}
	FORCEINL void MirrorTo(Image_Ref x,int axis = AxisHorizontal) const
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {
		case 1: ipp_cpp::ippiMirror_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,(IppiAxis)axis); break;
		case 3: ipp_cpp::ippiMirror_C3R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,(IppiAxis)axis); break;
		case 4: ipp_cpp::ippiMirror_C4R(IPPARG_IMG(*this),IPPARG_IMG(x),*this,(IppiAxis)axis); break;
	}	}
	FORCEINL void RotateTo(double angle_deg, const rt::Vec2d& cent_src, const Image_Ref& x) const
	{	ASSERT(!x.IsEmpty());
		IppiRect rc; 	
		rc.x = 0;				rc.y = 0;		
		rc.width = GetWidth();	rc.height = GetHeight();
		IppiRect rc_d; 	
		rc_d.x = 0;				rc_d.y = 0;
		rc_d.width = x.GetWidth();	rc_d.height = x.GetHeight();

		double xShift, yShift;
		IPPCALL(ippiGetRotateShift)(cent_src.x,cent_src.y,angle_deg,&xShift,&yShift);
		xShift += cent_src.x * (x.GetWidth()/(double)GetWidth() - 1);
		yShift += cent_src.y * (x.GetHeight()/(double)GetHeight() - 1);

		switch(chan_num) {
		case 1: ipp_cpp::ippiRotate_C1R(IPPARG_IMG2(*this),rc,IPPARG_IMG(x),rc_d,angle_deg,xShift,yShift,GetEnv()->InterpolationMode); break;
		case 3: ipp_cpp::ippiRotate_C3R(IPPARG_IMG2(*this),rc,IPPARG_IMG(x),rc_d,angle_deg,xShift,yShift,GetEnv()->InterpolationMode); break;
		case 4: ipp_cpp::ippiRotate_C4R(IPPARG_IMG2(*this),rc,IPPARG_IMG(x),rc_d,angle_deg,xShift,yShift,GetEnv()->InterpolationMode); break;
	}	}
	FORCEINL void Multiply(const t_Val& scale)
	{	switch(chan_num) {
			case 1: ipp_cpp::ippiMulCScale_C1R(scale[0],IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiMulCScale_C3R(scale,IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiMulCScale_C4R(scale,IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void MultiplyTo(Image_Ref& x,const t_Val& scale) const
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {
			case 1: ipp_cpp::ippiMulCScale_C1R(IPPARG_IMG(*this),scale[0],IPPARG_IMG(x),*this); break;
			case 3: ipp_cpp::ippiMulCScale_C3R(IPPARG_IMG(*this),scale,IPPARG_IMG(x),*this); break;
			case 4: ipp_cpp::ippiMulCScale_C4R(IPPARG_IMG(*this),scale,IPPARG_IMG(x),*this); break;
	}	}
	FORCEINL void Multiply(const Image_Ref& src)
	{	ASSERT_SIZE(*this,src);
		switch(chan_num) {
			case 1: ipp_cpp::ippiMul_C1R(IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiMul_C3R(IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiMul_C4R(IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void MultiplyTo(const Image_Ref& src,Image_Ref& dst) const
	{	ASSERT_SIZE(*this,src);
		ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ipp_cpp::ippiMul_C1R(IPPARG_IMG(src),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 3: ipp_cpp::ippiMul_C3R(IPPARG_IMG(src),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiMul_C4R(IPPARG_IMG(src),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
    /*
	FORCEINL void Xor(const Image_Ref& src)
	{	ASSERT_SIZE(*this,src);
		switch(chan_num) {
			case 1: ipp_cpp::ippiXor_C1IR(IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiXor_C3IR(IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiXor_C4IR(IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
	}	}*/
	FORCEINL void XorTo(const Image_Ref& src,Image_Ref& dst) const
	{	ASSERT_SIZE(*this,src);
		switch(chan_num) {
			case 1: ipp_cpp::ippiXor_C1R(IPPARG_IMG(src),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 3: ipp_cpp::ippiXor_C3R(IPPARG_IMG(src),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiXor_C4R(IPPARG_IMG(src),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void Divide(const Image_Ref& src)
	{	ASSERT_SIZE(*this,src);
		switch(chan_num) {
			case 1: ipp_cpp::ippiDiv_C1R(IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiDiv_C3R(IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiDiv_C4R(IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void DivideTo(const Image_Ref& src,Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ipp_cpp::ippiDiv_C1R(IPPARG_IMG(src),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 3: ipp_cpp::ippiDiv_C3R(IPPARG_IMG(src),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiDiv_C4R(IPPARG_IMG(src),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void Divide(const t_Val& scale){ DivideTo(*this,scale); }
	FORCEINL void DivideTo(Image_Ref& dst, const t_Val& scale) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ipp_cpp::ippiDiv_C1R(IPPARG_IMG(*this),scale[0],IPPARG_IMG(dst),*this); break;
			case 3: ipp_cpp::ippiDiv_C3R(IPPARG_IMG(*this),scale,IPPARG_IMG(dst),*this); break;
			case 4: ASSERT(0); // ipp_cpp::ippiDiv_C4R(IPPARG_IMG(*this),scale,IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void Sqr()
	{	switch(chan_num) {
			case 1: ipp_cpp::ippiSqr_C1R(IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiSqr_C3R(IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiSqr_C4R(IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void SqrTo(Image_Ref& x) const
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {
			case 1: ipp_cpp::ippiSqr_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),*this); break;
			case 3: ipp_cpp::ippiSqr_C3R(IPPARG_IMG(*this),IPPARG_IMG(x),*this); break;
			case 4: ipp_cpp::ippiSqr_C4R(IPPARG_IMG(*this),IPPARG_IMG(x),*this); break;
	}	}
	FORCEINL void Sqrt()
	{	switch(chan_num) {
			case 1: ipp_cpp::ippiSqrt_C1R(IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiSqrt_C3R(IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiSqrt_AC4R(IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void SqrtTo(Image_Ref& x) const
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {
			case 1: ipp_cpp::ippiSqrt_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),*this); break;
			case 3: ipp_cpp::ippiSqrt_C3R(IPPARG_IMG(*this),IPPARG_IMG(x),*this); break;
			case 4: ipp_cpp::ippiSqrt_AC4R(IPPARG_IMG(*this),IPPARG_IMG(x),*this); break;
	}	}
	FORCEINL void CopyChannelFrom(const Image_Ref<t_Value,1>& x,int chan_id)
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {
			case 1: ASSERT(chan_id<1); ipp_cpp::ippiCopy_C1R(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
			case 3: ASSERT(chan_id<3); ipp_cpp::ippiCopy_C1C3R(IPPARG_IMG(x),((LPValueType)GetImageData())+chan_id,GetStep(),*this); break;
			case 4: ASSERT(chan_id<4); ipp_cpp::ippiCopy_C1C4R(IPPARG_IMG(x),((LPValueType)GetImageData())+chan_id,GetStep(),*this); break;
	}	}
	FORCEINL void CopyChannelTo(Image_Ref<t_Value,1>& x,int chan_id) const
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {
			case 1: ASSERT(chan_id<1); ipp_cpp::ippiCopy_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),*this); break;
			case 3: ASSERT(chan_id<3); ipp_cpp::ippiCopy_C3C1R(((LPCValueType)GetImageData())+chan_id,GetStep(),IPPARG_IMG(x),*this); break;
			case 4: ASSERT(chan_id<4); ipp_cpp::ippiCopy_C4C1R(((LPCValueType)GetImageData())+chan_id,GetStep(),IPPARG_IMG(x),*this); break;
	}	}
	FORCEINL void AddWeightedTo(Image_Ref<Ipp32f,chan_num>& x,float alpha) const//x = x*(1-a)+this*a
	{	ASSERT_SIZE(*this,x);
		ipp::Size sz = x;
		sz.width*=chan_num;
		ipp_cpp::ippiAddWeighted_C1R( IPPARG_IMG(*this),IPPARG_IMG(x),sz,alpha);
	}
	FORCEINL void Add(const Image_Ref& x)
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {	
			case 1: ipp_cpp::ippiAdd_C1R(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiAdd_C3R(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiAdd_C4R(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void Accumulate(const Image_Ref<Ipp8u,1>& x)	// this = 32f, x = 8u
	{	ASSERT_SIZE(*this,x);
		IppiSize sz = *this;
		sz.width *= chan_num;
		IPPCALL(ippiAdd_8u32f_C1IR)(IPPARG_IMG(x), IPPARG_IMG(*this), sz);
	}
	FORCEINL void AddTo(const Image_Ref& src1,Image_Ref& dst) const
	{	ASSERT_SIZE(*this,src1); 
		ASSERT_SIZE(*this,dst);
		switch(chan_num) {	
			case 1: ipp_cpp::ippiAdd_C1R(IPPARG_IMG(src1),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 3: ipp_cpp::ippiAdd_C3R(IPPARG_IMG(src1),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiAdd_C4R(IPPARG_IMG(src1),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void Add(const t_Val& val)
	{	switch(chan_num) {	
			case 1: ipp_cpp::ippiAdd_C1R(val[0],IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiAdd_C3R(val,IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiAdd_C4R(val,IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void AddTo(const t_Val& val,Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {	
			case 1: ipp_cpp::ippiAdd_C1R(IPPARG_IMG(dst),val[0],IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiAdd_C3R(IPPARG_IMG(dst),val,IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiAdd_C4R(IPPARG_IMG(dst),val,IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void Subtract(const Image_Ref& x)
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {	
			case 1: ipp_cpp::ippiSub_C1R(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiSub_C3R(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiSub_C4R(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void SubtractTo(const Image_Ref& src1,Image_Ref& dst) const
	{	ASSERT_SIZE(*this,src1); 
		ASSERT_SIZE(*this,dst);
		switch(chan_num) {	
			case 1: ipp_cpp::ippiSub_C1R(IPPARG_IMG(src1),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 3: ipp_cpp::ippiSub_C3R(IPPARG_IMG(src1),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiSub_C4R(IPPARG_IMG(src1),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void AbsDiffTo(const Image_Ref& src1,Image_Ref& dst) const
	{	ASSERT_SIZE(*this,src1); 
		ASSERT_SIZE(*this,dst);
		ipp_cpp::ippiAbsDiff_C1R(IPPARG_IMG(src1),IPPARG_IMG(*this),IPPARG_IMG(dst),Size(GetWidth()*chan_num, GetHeight()));
	}
	FORCEINL void Subtract(const t_Val& val)
	{	switch(chan_num) {	
			case 1: ipp_cpp::ippiSub_C1R(val[0],IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiSub_C3R(val,IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiSub_C4R(val,IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void SubtractTo(const t_Val& val,Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {	
			case 1: ipp_cpp::ippiSub_C1R(IPPARG_IMG(dst),val[0],IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiSub_C3R(IPPARG_IMG(dst),val,IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiSub_C4R(IPPARG_IMG(dst),val,IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void Zero(){ Set(0); }
	FORCEINL void Set(const t_Val& value)
	{	switch(chan_num) {
			case 1: ipp_cpp::ippiSet_C1R(value[0],IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiSet_C3R(value,IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiSet_C4R(value,IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void Ln()
	{	switch(chan_num) {	
			case 1: ipp_cpp::ippiLn_C1R(IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiLn_C3R(IPPARG_IMG(*this),*this); break;
			case 4: {	ipp::Size sz(Width*4,Height);
						ipp_cpp::ippiLn_C1R(IPPARG_IMG(*this),sz);
						break;
	}	}			}
	FORCEINL void LnTo(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst); 
		switch(chan_num) {	
			case 1: ipp_cpp::ippiLn_C1R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 3: ipp_cpp::ippiLn_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: {	ipp::Size sz(Width*4,Height);
						ipp_cpp::ippiLn_C1R(IPPARG_IMG(*this),IPPARG_IMG(dst),sz);
						break;
	}	}			}
	FORCEINL void Exp()
	{	switch(chan_num) {	
			case 1: ipp_cpp::ippiExp_C1R(IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiExp_C3R(IPPARG_IMG(*this),*this); break;
			case 4: {	ipp::Size sz(Width*4,Height);
						ipp_cpp::ippiExp_C1R(IPPARG_IMG(*this),sz);
						break;
	}	}			}
	FORCEINL void ExpTo(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst); 
		switch(chan_num) {	
			case 1: ipp_cpp::ippiExp_C1R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 3: ipp_cpp::ippiExp_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: {	ipp::Size sz(Width*4,Height);
						ipp_cpp::ippiExp_C1R(IPPARG_IMG(*this),IPPARG_IMG(dst),sz);
						break;
	}	}			}
	void GaussTo(Image_Ref& dst, UINT FilterSize,float SmoothRatio = 5.0f) const
	{	if( FilterSize > 1 )
		{	FilterSize = rt::min(FilterSize,rt::min(GetWidth(),GetHeight()));
			if( FilterSize&1 ){}else{ FilterSize++; }
			float* filter = (float*)_Alloca32AL(FilterSize*sizeof(float));
			// Gauss kernel
			{	double tot = 0;
				UINT half = (FilterSize+1)>>1;
				float center = (FilterSize-1)*0.5f;
				float scale = SmoothRatio/(float)(FilterSize-1);
				UINT i=0;
				for(;i<=half;i++)tot += filter[i] = exp(-rt::Sqr(scale*(center-i)));
				for(;i<FilterSize;i++)tot += (filter[i] = filter[FilterSize-i-1]);
				for(UINT i=0;i<FilterSize;i++)filter[i] = (float)(filter[i]/tot);
			}
			Image<ValueType,chan_num>	tmp;
			tmp.SetSize(GetRegion().AddBorder(FilterSize>>1));
			SeparableFilterTo(dst,filter,FilterSize,tmp);
		}
	}
	FORCEINL void Gauss(UINT FilterSize,float SmoothRatio = 5.0f)
	{	if( FilterSize > 1 )
		{	FilterSize = rt::min(FilterSize,min(GetWidth(),GetHeight() ));
			if( FilterSize&1 ){}else{ FilterSize++; }
			// Gauss kernel
			float* filter = (float*)_Alloca32AL(FilterSize*sizeof(float));
			{	double tot = 0;
				UINT half = (FilterSize+1)>>1;
				float center = (FilterSize-1)*0.5f;
				float scale = SmoothRatio/(float)(FilterSize-1);
				UINT i=0;
				for(;i<=half;i++)tot += filter[i] = exp(-rt::Sqr(scale*(center-i)));
				for(;i<FilterSize;i++)tot += (filter[i] = filter[FilterSize-i-1]);
				for(UINT i=0;i<FilterSize;i++)filter[i] = (float)(filter[i]/tot);
			}
			Image<ValueType,chan_num>	tmp;
			tmp.SetSize(GetRegion().AddBorder(FilterSize>>1));
			SeparableFilter(filter,FilterSize,tmp);
		}
	}
	FORCEINL void SeparableFilter(LPCFLOAT pKernel_1D,UINT KernelSize,Image_Ref& workspace)
	{	ASSERT(KernelSize&1); //must be odd
		int archor = (int)KernelSize>>1;
		ASSERT_SIZE(*this,workspace.GetSub_Inside((UINT)archor)); //workspace must be initialized

		Image_Ref& dstwin = workspace.GetSub_Inside((UINT)archor);
		switch(chan_num) {
			case 1: //copy border along y and filter
					CopyTo(dstwin);
					GetSub(0,0,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,0,GetWidth(),archor),AxisHorizontal);
					GetSub(0,GetHeight()-archor-1,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,GetHeight()+archor,GetWidth(),archor),AxisHorizontal);
					ipp_cpp::ippiFilterColumn32f_C1R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);

					//copy with border along x and fileter
					CopyTo(dstwin);
					GetSub(0,0,archor,GetHeight()).MirrorTo(workspace.GetSub(0,archor,archor,GetHeight()),AxisVertical);
					GetSub(GetWidth()-archor-1,0,archor,GetHeight()).MirrorTo(workspace.GetSub(GetWidth()+archor,archor,archor,GetHeight()),AxisVertical);

					ipp_cpp::ippiFilterRow32f_C1R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);
					break;
			case 3: 
					CopyTo(dstwin);
					GetSub(0,0,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,0,GetWidth(),archor),AxisHorizontal);
					GetSub(0,GetHeight()-archor-1,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,GetHeight()+archor,GetWidth(),archor),AxisHorizontal);
					ipp_cpp::ippiFilterColumn32f_C3R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);

					CopyTo(dstwin);
					GetSub(0,0,archor,GetHeight()).MirrorTo(workspace.GetSub(0,archor,archor,GetHeight()),AxisVertical);
					GetSub(GetWidth()-archor-1,0,archor,GetHeight()).MirrorTo(workspace.GetSub(GetWidth()+archor,archor,archor,GetHeight()),AxisVertical);
					ipp_cpp::ippiFilterRow32f_C3R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);
					break;
			case 4: 
					CopyTo(dstwin);
					GetSub(0,0,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,0,GetWidth(),archor),AxisHorizontal);
					GetSub(0,GetHeight()-archor-1,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,GetHeight()+archor,GetWidth(),archor),AxisHorizontal);
					ipp_cpp::ippiFilterColumn32f_C4R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);

					CopyTo(dstwin);
					GetSub(0,0,archor,GetHeight()).MirrorTo(workspace.GetSub(0,archor,archor,GetHeight()),AxisVertical);
					GetSub(GetWidth()-archor-1,0,archor,GetHeight()).MirrorTo(workspace.GetSub(GetWidth()+archor,archor,archor,GetHeight()),AxisVertical);
					ipp_cpp::ippiFilterRow32f_C4R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);
					break;
	}	}
	FORCEINL void SeparableFilterTo(Image_Ref& dst, LPCFLOAT pKernel_1D, UINT KernelSize, Image_Ref& workspace) const
	{	ASSERT_SIZE(dst,*this);
		ASSERT(KernelSize&1); //must be odd

		int archor = (int)KernelSize>>1;
		ASSERT_SIZE(*this,workspace.GetSub_Inside((UINT)archor)); //workspace must be initialized

		Image_Ref dstwin = workspace.GetSub_Inside((UINT)archor);
		switch(chan_num) {
			case 1: //copy border along y and filter
					CopyTo(dstwin);
					GetSub(0,0,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,0,GetWidth(),archor),AxisHorizontal);
					GetSub(0,GetHeight()-archor-1,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,GetHeight()+archor,GetWidth(),archor),AxisHorizontal);
					ipp_cpp::ippiFilterColumn32f_C1R(IPPARG_IMG(dstwin),IPPARG_IMG(dst),dst,pKernel_1D,(int)KernelSize,archor);

					//copy with border along x and fileter
					dst.CopyTo(dstwin);
					dst.GetSub(0,0,archor,GetHeight()).MirrorTo(workspace.GetSub(0,archor,archor,GetHeight()),AxisVertical);
					dst.GetSub(GetWidth()-archor-1,0,archor,GetHeight()).MirrorTo(workspace.GetSub(GetWidth()+archor,archor,archor,GetHeight()),AxisVertical);

					ipp_cpp::ippiFilterRow32f_C1R(IPPARG_IMG(dstwin),IPPARG_IMG(dst),dst,pKernel_1D,(int)KernelSize,archor);
					break;
			case 3: 
					CopyTo(dstwin);
					GetSub(0,0,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,0,GetWidth(),archor),AxisHorizontal);
					GetSub(0,GetHeight()-archor-1,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,GetHeight()+archor,GetWidth(),archor),AxisHorizontal);
					ipp_cpp::ippiFilterColumn32f_C3R(IPPARG_IMG(dstwin),IPPARG_IMG(dst),dst,pKernel_1D,(int)KernelSize,archor);

					dst.CopyTo(dstwin);
					dst.GetSub(0,0,archor,GetHeight()).MirrorTo(workspace.GetSub(0,archor,archor,GetHeight()),AxisVertical);
					dst.GetSub(GetWidth()-archor-1,0,archor,GetHeight()).MirrorTo(workspace.GetSub(GetWidth()+archor,archor,archor,GetHeight()),AxisVertical);
					ipp_cpp::ippiFilterRow32f_C3R(IPPARG_IMG(dstwin),IPPARG_IMG(dst),dst,pKernel_1D,(int)KernelSize,archor);
					break;
			case 4: 
					CopyTo(dstwin);
					GetSub(0,0,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,0,GetWidth(),archor),AxisHorizontal);
					GetSub(0,GetHeight()-archor-1,GetWidth(),archor).MirrorTo(workspace.GetSub(archor,GetHeight()+archor,GetWidth(),archor),AxisHorizontal);
					ipp_cpp::ippiFilterColumn32f_C4R(IPPARG_IMG(dstwin),IPPARG_IMG(dst),dst,pKernel_1D,(int)KernelSize,archor);

					dst.CopyTo(dstwin);
					dst.GetSub(0,0,archor,GetHeight()).MirrorTo(workspace.GetSub(0,archor,archor,GetHeight()),AxisVertical);
					dst.GetSub(GetWidth()-archor-1,0,archor,GetHeight()).MirrorTo(workspace.GetSub(GetWidth()+archor,archor,archor,GetHeight()),AxisVertical);
					ipp_cpp::ippiFilterRow32f_C4R(IPPARG_IMG(dstwin),IPPARG_IMG(dst),dst,pKernel_1D,(int)KernelSize,archor);
					break;
	}	}
	/*
	FORCEINL void DotProduct(const Image_Ref& src, t_Val& out)
	{	ASSERT_SIZE(*this,src);
		switch(chan_num) {
			case 1: ipp_cpp::ippiConvValid_C1R(IPPARG_IMG(*this),*this,IPPARG_IMG(src),*this,value,1); break;
			case 3: ipp_cpp::ippiConvValid_C3R(IPPARG_IMG(*this),*this,IPPARG_IMG(src),*this,value,1); break;
			case 4: ipp_cpp::ippiConvValid_AC4R(IPPARG_IMG(*this),*this,IPPARG_IMG(src),*this,value,1); break;
	}	}
	*/
	FORCEINL void SwapChannels_8u(LPCSTR pOrder = "\2\0\0\0\1\0\0\0\0\0\0\0\3\0\0\0")  //RGB<-->BGR, BGRA<-->RGBA by default
	{	ASSERT(GetBPV()==8);
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: IPPCALL(ippiSwapChannels_8u_C3IR)(IPPARG_IMG(*this),*this,(const int *)pOrder); break;
			case 4: IPPCALL(ippiSwapChannels_8u_C4IR)(IPPARG_IMG(*this),*this,(const int *)pOrder); break;
	}	}
	/*
	FORCEINL void SwapChannelsTo(Image_Ref& dst, Vec3i& Order = Vec3u(2,1,0))  //RGB<-->BGR by default
	{	ASSERT_SIZE(*this,dst);
		ASSERT(chan_num == 3 || chan_num == 4);
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: ipp_cpp::ippiSwapChannels_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this,(const int *)&Order); break;
			case 4: ipp_cpp::ippiSwapChannels_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this,(const int *)&Order); break;
	}	}
	*/
	FORCEINL void AlphaBlend(const Image_Ref& src1,const Image_Ref& src2,float alpha_1, float alpha_2=1.0f,int AlphaType = BlendMethod_AlphaOver)
	{	ASSERT_SIZE(*this,src1);
		ASSERT_SIZE(*this,src2);
		ASSERT(GetBPV()==8 || GetBPV()==16); //LDR only 
		ASSERT(alpha_1>=0.0f && alpha_1<=1.0f);
		ASSERT(alpha_2>=0.0f && alpha_2<=1.0f);
		rt::Vec2i a;
		a.x = (int)(rt::TypeTraits<ValueType>::MaxVal()*alpha_1);
		a.y = (int)(rt::TypeTraits<ValueType>::MaxVal()*alpha_2);

		switch(chan_num) {
			case 1: ipp_cpp::ippiAlphaComp_C1R(	IPPARG_IMG(src1),a.x,
										IPPARG_IMG(src2),a.y,
										IPPARG_IMG(*this),*this,(IppiAlphaType)BlendMethod_AlphaOver); 
					break;
			case 3: ipp_cpp::ippiAlphaComp_C3R(	IPPARG_IMG(src1),a.x,
										IPPARG_IMG(src2),a.y,
										IPPARG_IMG(*this),*this,(IppiAlphaType)BlendMethod_AlphaOver); 
					break;
			case 4: ipp_cpp::ippiAlphaComp_C4R(	IPPARG_IMG(src1),a.x,
										IPPARG_IMG(src2),a.y,
										IPPARG_IMG(*this),*this,(IppiAlphaType)BlendMethod_AlphaOver); 
					break;
	}	}
	FORCEINL void AlphaBlend(const Image_Ref& src1,const Image_Ref& src2,int AlphaType = BlendMethod_AlphaOver)
	{	ASSERT_SIZE(*this,src1);
		ASSERT_SIZE(*this,src2);
		ASSERT(GetBPV()==8 || GetBPV()==16); //LDR only 
		ASSERT(chan_num==4);
		ipp_cpp::ippiAlphaComp_AC4R(IPPARG_IMG(src1),IPPARG_IMG(src2),IPPARG_IMG(*this),*this,(IppiAlphaType)AlphaType);
	}
	FORCEINL void Difference(const Image_Ref& x1, const Image_Ref& x2)
	{	ASSERT_SIZE(*this,x1);
		ASSERT_SIZE(*this,x2);
		ipp_cpp::ippiAbsDiff_C1R(	IPPARG_IMG(x1),IPPARG_IMG(x2),IPPARG_IMG(*this),
									ipp::Size(GetWidth()*chan_num, GetHeight()));
	}
	FORCEINL void Sub(const Image_Ref& x)
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {	
			case 1: ipp_cpp::ippiSub_C1R(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiSub_C3R(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiSub_C4R(IPPARG_IMG(x),IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void SubTo(const Image_Ref& src1,Image_Ref& dst) const
	{	ASSERT_SIZE(*this,src1); 
		ASSERT_SIZE(*this,dst);
		switch(chan_num) {	
			case 1: ipp_cpp::ippiSub_C1R(IPPARG_IMG(src1),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 3: ipp_cpp::ippiSub_C3R(IPPARG_IMG(src1),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiSub_C4R(IPPARG_IMG(src1),IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void Sub(const t_Val& val)
	{	switch(chan_num) {	
			case 1: ipp_cpp::ippiSub_C1R(val[0],IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiSub_C3R(val,IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiSub_C4R(val,IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void SubTo(const t_Val& val,Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {	
			case 1: ipp_cpp::ippiSub_C1R(IPPARG_IMG(dst),val[0],IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiSub_C3R(IPPARG_IMG(dst),val,IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiSub_C4R(IPPARG_IMG(dst),val,IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void BackwardRemap(const Ref_1C& map_x, const Ref_1C& map_y, const Image_Ref& src)
	{	ASSERT_SIZE(*this,map_x);
		ASSERT_SIZE(*this,map_y);
		IppiRect rc;
		rc.x = rc.y = 0;
		rc.width = src.GetWidth();
		rc.height= src.GetHeight();
		switch(chan_num) {	
			case 1: ipp_cpp::ippiRemap_C1R(IPPARG_IMG2(src),rc,IPPARG_IMG(map_x),IPPARG_IMG(map_y),IPPARG_IMG(*this),*this,GetEnv()->InterpolationMode); break;
			case 3: ipp_cpp::ippiRemap_C3R(IPPARG_IMG2(src),rc,IPPARG_IMG(map_x),IPPARG_IMG(map_y),IPPARG_IMG(*this),*this,GetEnv()->InterpolationMode); break;
			case 4: ipp_cpp::ippiRemap_C4R(IPPARG_IMG2(src),rc,IPPARG_IMG(map_x),IPPARG_IMG(map_y),IPPARG_IMG(*this),*this,GetEnv()->InterpolationMode); break;
	}	}
	INLFUNC void HistogramEven(LPFLOAT pHistBins[chan_num], UINT BinCount, t_Val Max_val, t_Val Min_val = 0) const
	{
		typedef typename _details::_HistogramLevelType<t_Value>::t_Result t_Level;
		LPBYTE temp = (LPBYTE)alloca(sizeof(t_Level)*(BinCount+1)*chan_num + sizeof(Ipp32s)*chan_num*BinCount);
		// Compute levels
		t_Level* pLevels[chan_num];
		for(UINT c=0;c<chan_num;c++)
		{	pLevels[c] = ((t_Level*)temp) + c*(BinCount+1);
 			for(UINT i=0;i<=BinCount;i++)
				pLevels[c][i] = (Min_val[c]*(BinCount-i) + Max_val[c]*i)/BinCount;
		}

		Ipp32s* pHist = (Ipp32s*)&temp[sizeof(t_Level)*chan_num*(BinCount+1)];
		switch(chan_num) {
		case 1: {	ipp_cpp::ippiHistogramRange_C1R(IPPARG_IMG(*this),GetRegion(),pHist,pLevels[0],BinCount+1);
					// merge results
					ULONGLONG tot = 0;
					for(UINT i=0;i<BinCount;i++)tot += pHist[i];
					for(UINT i=0;i<BinCount;i++)pHistBins[0][i] = (float)(pHist[i]/(double)tot);
				} break;
		case 3: {	Ipp32s* pBins[3] = {&pHist[BinCount*0], &pHist[BinCount*1], &pHist[BinCount*2]};
					int BinCounts[3] = {(int)(BinCount+1), (int)(BinCount+1), (int)(BinCount+1)};
					ipp_cpp::ippiHistogramRange_C3R(IPPARG_IMG(*this),*this,(Ipp32s **)pBins,(const t_Level**)pLevels,(int*)BinCounts);

					// merge results
					ULONGLONG tot[3] = {0,0,0};
					for(UINT i=0;i<BinCount;i++)
					{	tot[0] += pHist[BinCount*0 + i];
						tot[1] += pHist[BinCount*1 + i];
						tot[2] += pHist[BinCount*2 + i];
					}
					for(UINT i=0;i<BinCount;i++)
					{
						pHistBins[0][i] = (float)(pHist[BinCount*0 + i]/(double)tot[0]);
						pHistBins[1][i] = (float)(pHist[BinCount*1 + i]/(double)tot[1]);
						pHistBins[2][i] = (float)(pHist[BinCount*2 + i]/(double)tot[2]);
					}
				} break;
		case 4: {	Ipp32s* pBins[4] = {&pHist[BinCount*0], &pHist[BinCount*1], &pHist[BinCount*2], &pHist[BinCount*3]};
					int BinCounts[4] = {(int)(BinCount+1), (int)(BinCount+1), (int)(BinCount+1), (int)(BinCount+1)};
					ipp_cpp::ippiHistogramRange_C4R(IPPARG_IMG(*this),*this,(Ipp32s **)pBins,(const t_Level**)pLevels,(int*)BinCounts);

					// merge results
					ULONGLONG tot[4] = {0,0,0,0};
					for(UINT i=0;i<BinCount;i++)
					{	tot[0] += pHist[BinCount*0 + i];
						tot[1] += pHist[BinCount*1 + i];
						tot[2] += pHist[BinCount*2 + i];
						tot[3] += pHist[BinCount*3 + i];
					}
					for(UINT i=0;i<BinCount;i++)
					{
						pHistBins[0][i] = (float)(pHist[BinCount*0 + i]/(double)tot[0]);
						pHistBins[1][i] = (float)(pHist[BinCount*1 + i]/(double)tot[1]);
						pHistBins[2][i] = (float)(pHist[BinCount*2 + i]/(double)tot[2]);
						pHistBins[3][i] = (float)(pHist[BinCount*3 + i]/(double)tot[3]);
					}
				} break;
		}
	}
	FORCEINL void BGR2Lab(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ASSERT(0); break;
			case 3: IPPCALL(::ippiBGRToLab_8u_C3R)(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ASSERT(0); break;
	}	}
	FORCEINL void Lab2BGR(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: IPPCALL(::ippiLabToBGR_8u_C3R)(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ASSERT(0); break;
	}	}
	FORCEINL void RGB2LUV(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ASSERT(0); break;
			case 3: ipp_cpp::ippiRGBToLUV_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiRGBToLUV_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void LUV2RGB(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ASSERT(0);  break;
			case 3: ipp_cpp::ippiLUVToRGB_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiLUVToRGB_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void RGB2HLS(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ASSERT(0);  break;
			case 3: ipp_cpp::ippiRGBToHLS_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiRGBToHLS_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void HLS2RGB(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ASSERT(0);  break;
			case 3: ipp_cpp::ippiHLSToRGB_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiHLSToRGB_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void RGB2Lab(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ASSERT(0);  break;
			case 3: ipp_cpp::ippiBGRToLab_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
            case 4: ASSERT(0); break;
	}	}
	FORCEINL void Lab2RGB(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
			case 1: ASSERT(0);  break;
			case 3: ipp_cpp::ippiLabToBGR_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ASSERT(0); break;
	}	}
	FORCEINL void FilterGauss(Image_Ref& dst, IppiMaskSize ms = ippMskSize5x5) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
		case 1: ipp_cpp::ippiFilterGauss_C1R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this, ms); break;
		case 3: ipp_cpp::ippiFilterGauss_C3R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this, ms); break;
		case 4: ipp_cpp::ippiFilterGauss_AC4R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this, ms); break;
		}
	}
	FORCEINL void FilterLaplace(Image_Ref& dst, IppiMaskSize ms = ippMskSize5x5) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
		case 1: ipp_cpp::ippiFilterLaplace_C1R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this, ms); break;
		case 3: ipp_cpp::ippiFilterLaplace_C3R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this, ms); break;
		case 4: ipp_cpp::ippiFilterLaplace_AC4R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this, ms); break;
		}
	}
	FORCEINL void FilterHipass(Image_Ref& dst, IppiMaskSize ms = ippMskSize5x5) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
		case 1: ipp_cpp::ippiFilterHipass_C1R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this, ms); break;
		case 3: ipp_cpp::ippiFilterHipass_C3R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this, ms); break;
		case 4: ipp_cpp::ippiFilterHipass_AC4R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this, ms); break;
		}
	}
	FORCEINL void FilterSharpen(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		switch(chan_num) {
		case 1: ipp_cpp::ippiFilterSharpen_C1R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this); break;
		case 3: ipp_cpp::ippiFilterSharpen_C3R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this); break;
		case 4: ipp_cpp::ippiFilterSharpen_AC4R(IPPARG_IMG(*this), IPPARG_IMG(dst),*this); break;
		}
	}
	//FORCEINL void Filter_LowerMask(const Image_Ref& kernel, ValueType ValidValueMin)
	//{	ASSERT(kernel.GetWidth()&1);
	//	ASSERT(kernel.GetHeight()&1);
	//	int knl_half_width = kernel.GetWidth()/2;
	//	int knl_half_height = kernel.GetHeight()/2;
	//	t_ImageObj temp;
	//	temp.SetSize( GetWidth()+kernel.GetWidth()-1, GetHeight()+kernel.GetHeight()-1 );
	//	{	//Clear to invalid value
	//		t_Val invalid;
	//		invalid.Set( rt::TypeTraits<ValueType>::MinVal() );
	//		temp.Set(invalid);
	//	}
	//	CopyTo(temp.GetSub(knl_half_width,knl_half_height,GetWidth(),GetHeight()));

	//	rt::TypeTraits<t_Val>::t_Accum accum,wei;

	//	for(int dy=0;dy<GetHeight();dy++)
	//	for(int dx=0;dx<GetWidth();dx++)
	//	{
	//		accum.Set(0);
	//		wei.Set(0);
	//		
	//		t_Val& dest = GetPixel(dx,dy);
	//		if(dest.x>=ValidValueMin)
	//		{
	//			Image_Ref & win = temp.GetSub( dx, dy, kernel.GetWidth(), kernel.GetHeight() );

	//			for(int y=0;y<kernel.GetHeight();y++)
	//			for(int x=0;x<kernel.GetWidth();x++)
	//			{
	//				t_Val& px = win.GetPixel(x,y);
	//				if( px.x>=ValidValueMin )
	//				{	accum = accum + px*kernel.GetPixel(x,y);
	//					wei = wei + kernel.GetPixel(x,y);
	//				}
	//			}

	//			dest = accum/wei;
	//		}
	//	}
	//}

	//FORCEINL void Filter_UpperMask(const Image_Ref& kernel, ValueType ValidValueMax)
	//{	ASSERT(kernel.GetWidth()&1);
	//	ASSERT(kernel.GetHeight()&1);
	//	int knl_half_width = kernel.GetWidth()/2;
	//	int knl_half_height = kernel.GetHeight()/2;
	//	t_ImageObj temp;
	//	temp.SetSize( GetWidth()+kernel.GetWidth()-1, GetHeight()+kernel.GetHeight()-1 );
	//	{	//Clear to invalid value
	//		t_Val invalid;
	//		invalid.Set( rt::TypeTraits<ValueType>::MinVal() );
	//		temp.Set(invalid);
	//	}
	//	CopyTo(temp.GetSub(knl_half_width,knl_half_height,GetWidth(),GetHeight()));

	//	for(int dy=0;dy<GetHeight();dy++)
	//	for(int dx=0;dx<GetWidth();dx++)
	//	{
	//		rt::TypeTraits<t_Val>::t_Accum accum,wei;
	//		
	//		accum.Set(0);
	//		wei.Set(0);
	//		
	//		t_Val& dest = GetPixel(dx,dy);
	//		if(dest.x<=ValidValueMax)
	//		{
	//			Image_Ref & win = temp.GetSub( dx, dy, kernel.GetWidth(), kernel.GetHeight() );

	//			for(int y=0;y<kernel.GetHeight();y++)
	//			for(int x=0;x<kernel.GetWidth();x++)
	//			{
	//				t_Val& px = win.GetPixel(x,y);
	//				if( px.x<=ValidValueMax )
	//				{	accum = accum + px*kernel.GetPixel(x,y);  
	//					wei = wei + kernel.GetPixel(x,y);
	//				}
	//			}

	//			dest = accum/wei;
	//		}
	//	}
	//}

	/*
	FORCEINL void Lowpass_3x3(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst); 
		IppiSize size;
		size.Width = Width-2;
		size.Height = Height-2;
		switch(chan_num) {	
			case 1: ipp_cpp::ippiFilterLowpass_C1R(GetValueAddress(1,1),GetStep(),dst.GetValueAddress(1,1),dst.GetStep(),size,ippMskSize3x3); break;
			case 3: ipp_cpp::ippiFilterLowpass_C3R(GetValueAddress(1,1),GetStep(),dst.GetValueAddress(1,1),dst.GetStep(),size,ippMskSize3x3); break;
			case 4: ipp_cpp::ippiFilterLowpass_AC4R(GetValueAddress(1,1),GetStep(),dst.GetValueAddress(1,1),dst.GetStep(),size,ippMskSize3x3); break;
	}	}
	FORCEINL void Lowpass_5x5(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst); 
		IppiSize size;
		size.Width = Width-4;
		size.Height = Height-4;
		switch(chan_num) {	
			case 1: ipp_cpp::ippiFilterLowpass_C1R(GetValueAddress(2,2),GetStep(),dst.GetValueAddress(2,2),dst.GetStep(),size,ippMskSize5x5); break;
			case 3: ipp_cpp::ippiFilterLowpass_C3R(GetValueAddress(2,2),GetStep(),dst.GetValueAddress(2,2),dst.GetStep(),size,ippMskSize5x5); break;
			case 4: ipp_cpp::ippiFilterLowpass_AC4R(GetValueAddress(2,2),GetStep(),dst.GetValueAddress(2,2),dst.GetStep(),size,ippMskSize5x5); break;
	}	}
	FORCEINL void Gauss_3x3(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst); 
		IppiSize size;
		size.Width = Width-2;
		size.Height = Height-2;
		switch(chan_num) {	
			case 1: ipp_cpp::ippiFilterGauss_C1R(GetValueAddress(1,1),GetStep(),dst.GetValueAddress(1,1),dst.GetStep(),size,ippMskSize3x3); break;
			case 3: ipp_cpp::ippiFilterGauss_C3R(GetValueAddress(1,1),GetStep(),dst.GetValueAddress(1,1),dst.GetStep(),size,ippMskSize3x3); break;
			case 4: ipp_cpp::ippiFilterGauss_AC4R(GetValueAddress(1,1),GetStep(),dst.GetValueAddress(1,1),dst.GetStep(),size,ippMskSize3x3); break;
	}	}
	FORCEINL void Gauss_5x5(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst); 
		IppiSize size;
		size.Width = Width-4;
		size.Height = Height-4;
		switch(chan_num) {	
			case 1: ipp_cpp::ippiFilterGauss_C1R(GetValueAddress(2,2),GetStep(),dst.GetValueAddress(2,2),dst.GetStep(),size,ippMskSize5x5); break;
			case 3: ipp_cpp::ippiFilterGauss_C3R(GetValueAddress(2,2),GetStep(),dst.GetValueAddress(2,2),dst.GetStep(),size,ippMskSize5x5); break;
			case 4: ipp_cpp::ippiFilterGauss_AC4R(GetValueAddress(2,2),GetStep(),dst.GetValueAddress(2,2),dst.GetStep(),size,ippMskSize5x5); break;
	}	}
	//Performs a full convolution of two images.
	//input img is one image
	//this is another
	//this will be replace by the convoluion result
	FORCEINL void ConvFull(const Image_Ref& img)
	{
		int FilterHeight = img.GetHeight();
		int FilterWidth = img.GetWidth();
		ASSERT(GetBPV()==16 || GetBPV()==32); //HDR only 

		CImage<ValueType, chan_num>  workspace;
		int archorY = (int)((FilterHeight>>1) + 0.6);
		int archorX = (int)((FilterWidth>>1) + 0.6);
		//increase to 2, MirrorTo seems have problem with size = 1
		archorY = max(2, archorY); archorX = max(2, archorX);
		workspace.SetSize(GetRegion().AddBorder(archorX, archorY));
		Image_Ref& dstwin = workspace.GetSub_Inside((UINT)archorX, (UINT)archorY);

		//expand border
		CopyTo(dstwin);
		GetSub(0,0,GetWidth(),archorY).MirrorTo(workspace.GetSub(archorX,0,GetWidth(),archorY),AxisHorizontal);
		GetSub(0,GetHeight()-archorY-1,GetWidth(),archorY).MirrorTo(workspace.GetSub(archorX,GetHeight()+archorY,GetWidth(),archorY),AxisHorizontal);
		GetSub(0,0,archorX,GetHeight()).MirrorTo(workspace.GetSub(0,archorY,archorX,GetHeight()),AxisVertical);
		GetSub(GetWidth()-archorX-1,0,archorX,GetHeight()).MirrorTo(workspace.GetSub(GetWidth()+archorX,archorY,archorX,GetHeight()),AxisVertical);
		GetSub(0,0,archorX,archorY).MirrorTo(workspace.GetSub(0,0,archorX, archorY), AxisBoth);
		GetSub(0,GetHeight()-archorY-1,archorX,archorY).MirrorTo(workspace.GetSub(0,GetHeight()+archorY,archorX, archorY), AxisBoth);
		GetSub(GetWidth()-archorX-1,0,archorX,archorY).MirrorTo(workspace.GetSub(GetWidth()+archorX,0,archorX, archorY), AxisBoth);
		GetSub(GetWidth()-archorX-1,GetHeight()-archorY-1,archorX,archorY).MirrorTo(workspace.GetSub(GetWidth()+archorX,GetHeight()+archorY,archorX, archorY), AxisBoth);

		//workspace.Save("abc.pfm");
		CImage<ValueType, chan_num>  tar;
		tar.SetSize(GetRegion().AddBorder(archorX*2, archorY*2));

        switch(chan_num) {
		case 1:
			ipp_cpp::ippiConvValid_C1R(IPPARG_IMG(workspace), workspace, IPPARG_IMG(img), img, IPPARG_IMG(tar));
			break;
		case 3: 
    		ipp_cpp::ippiConvFull_C3R(IPPARG_IMG(workspace), workspace, IPPARG_IMG(img), img, IPPARG_IMG(tar));
			break;
		case 4:
			ipp_cpp::ippiConvFull_AC4R(IPPARG_IMG(workspace), workspace, IPPARG_IMG(img), img, IPPARG_IMG(tar));
			break;
		}
		CopyFrom(tar.GetSub_Inside((UINT)archorX*2, (UINT)archorY*2));
	}

	FORCEINL void Gauss_WxH(UINT FilterWidth, UINT FilterHeight, float SmoothRatio = 4.0f)
	{
		num::Tabulated_Function<float> filter;
		CImage<ValueType, chan_num>  tmp;

		if (FilterWidth > 1)
		{
			FilterWidth = min(FilterWidth, GetWidth());
			FilterWidth |= 1;	

			filter.SetSize(FilterWidth);
			filter.GaussKernel(SmoothRatio);

			tmp.SetSize(GetRegion().AddBorder((FilterWidth>>1), 0));
			SeparableFilter_Row(filter, FilterWidth, tmp);
		}
		if (FilterHeight > 1)
		{
			FilterHeight = min(FilterHeight, GetHeight());
			FilterHeight |= 1;
			
			filter.SetSize(FilterHeight);
			filter.GaussKernel(SmoothRatio);

			tmp.SetSize(GetRegion().AddBorder(0, (FilterHeight>>1)));
			SeparableFilter_Col(filter, FilterHeight, tmp);
		}
	}
	FORCEINL void SeparableFilter_Col(LPCFLOAT pKernel_1D,UINT KernelSize,Image_Ref& workspace)
	{	ASSERT(KernelSize&1); //must be odd
		ASSERT_ARRAY(pKernel_1D,KernelSize);

		int archor = (int)KernelSize>>1;
		ASSERT_SIZE(*this,workspace.GetSub_Inside(0, (UINT)archor)); //workspace must be initialized

		Image_Ref& dstwin = workspace.GetSub_Inside(0, (UINT)archor);
		switch(chan_num) {
			case 1: //copy border along y and filter
					CopyTo(dstwin);
					GetSub(0,0,GetWidth(),archor).MirrorTo(workspace.GetSub(0,0,GetWidth(),archor),AxisHorizontal);
					GetSub(0,GetHeight()-archor-1,GetWidth(),archor).MirrorTo(workspace.GetSub(0,GetHeight()+archor,GetWidth(),archor),AxisHorizontal);
					ipp_cpp::ippiFilterColumn32f_C1R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);
					break;
			case 3: 
					CopyTo(dstwin);
					GetSub(0,0,GetWidth(),archor).MirrorTo(workspace.GetSub(0,0,GetWidth(),archor),AxisHorizontal);
					GetSub(0,GetHeight()-archor-1,GetWidth(),archor).MirrorTo(workspace.GetSub(0,GetHeight()+archor,GetWidth(),archor),AxisHorizontal);
					ipp_cpp::ippiFilterColumn32f_C3R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);
					break;
			case 4: 
					CopyTo(dstwin);
					GetSub(0,0,GetWidth(),archor).MirrorTo(workspace.GetSub(0,0,GetWidth(),archor),AxisHorizontal);
					GetSub(0,GetHeight()-archor-1,GetWidth(),archor).MirrorTo(workspace.GetSub(0,GetHeight()+archor,GetWidth(),archor),AxisHorizontal);
					ipp_cpp::ippiFilterColumn32f_C4R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);
					break;
	}	}
	FORCEINL void SeparableFilter_Row(LPCFLOAT pKernel_1D,UINT KernelSize,Image_Ref& workspace)
	{	ASSERT(KernelSize&1); //must be odd
		ASSERT_ARRAY(pKernel_1D,KernelSize);

		int archor = (int)KernelSize>>1;
		ASSERT_SIZE(*this,workspace.GetSub_Inside((UINT)archor, 0)); //workspace must be initialized

		Image_Ref& dstwin = workspace.GetSub_Inside((UINT)archor, 0);
		switch(chan_num) {
			case 1: //copy with border along x and filter
					CopyTo(dstwin);
					GetSub(0,0,archor,GetHeight()).MirrorTo(workspace.GetSub(0,0,archor,GetHeight()),AxisVertical);
					GetSub(GetWidth()-archor-1,0,archor,GetHeight()).MirrorTo(workspace.GetSub(GetWidth()+archor,0,archor,GetHeight()),AxisVertical);

					ipp_cpp::ippiFilterRow32f_C1R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);
					break;
			case 3: 
					CopyTo(dstwin);
					GetSub(0,0,archor,GetHeight()).MirrorTo(workspace.GetSub(0,0,archor,GetHeight()),AxisVertical);
					GetSub(GetWidth()-archor-1,0,archor,GetHeight()).MirrorTo(workspace.GetSub(GetWidth()+archor,0,archor,GetHeight()),AxisVertical);
					ipp_cpp::ippiFilterRow32f_C3R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);
					break;
			case 4: 
					CopyTo(dstwin);
					GetSub(0,0,archor,GetHeight()).MirrorTo(workspace.GetSub(0,0,archor,GetHeight()),AxisVertical);
					GetSub(GetWidth()-archor-1,0,archor,GetHeight()).MirrorTo(workspace.GetSub(GetWidth()+archor,0,archor,GetHeight()),AxisVertical);
					ipp_cpp::ippiFilterRow32f_C4R(IPPARG_IMG(dstwin),IPPARG_IMG(*this),*this,pKernel_1D,(int)KernelSize,archor);
					break;
	}	}
	FORCEINL void RGB2YUV(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		ASSERT(GetBPV()==8); //LDR only 
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: ipp_cpp::ippiRGBToYUV_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiRGBToYUV_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void YUV2RGB(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		ASSERT(GetBPV()==8); //LDR only 
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: ipp_cpp::ippiYUVToRGB_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiYUVToRGB_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void RGB2HSV(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		ASSERT(GetBPV()==8 || GetBPV()==8); //LDR only 
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: ipp_cpp::ippiRGBToHSV_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiRGBToHSV_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void HSV2RGB(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		ASSERT(GetBPV()==8 || GetBPV()==8); //LDR only 
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: ipp_cpp::ippiHSVToRGB_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ipp_cpp::ippiHSVToRGB_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
	}	}
	FORCEINL void BGR2Lab(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		ASSERT(GetBPV()==8); //LDR only 
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: ipp_cpp::ippiBGRToLab_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ASSERT(0);
	}	}
	FORCEINL void Lab2BGR(Image_Ref& dst) const
	{	ASSERT_SIZE(*this,dst);
		ASSERT(GetBPV()==8 || GetBPV()==16); //LDR only 
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: ipp_cpp::ippiLabToBGR_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this); break;
			case 4: ASSERT(0);
	}	}
	FORCEINL void SwapChannels_8u(LPCSTR pOrder = "\2\0\0\0\1\0\0\0\0\0\0\0\3\0\0\0")  //RGB<-->BGR, BGRA<-->RGBA by default
	{	ASSERT(GetBPV()==8);
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: IPPCALL(ippiSwapChannels_8u_C3IR)(IPPARG_IMG(*this),*this,(const int *)pOrder); break;
			case 4: IPPCALL(ippiSwapChannels_8u_C4IR)(IPPARG_IMG(*this),*this,(const int *)pOrder); break;
	}	}
	FORCEINL void SwapChannelsTo(Image_Ref& dst, Vec3i& Order = Vec3u(2,1,0))  //RGB<-->BGR by default
	{	ASSERT_SIZE(*this,dst);
		ASSERT(chan_num == 3 || chan_num == 4);
		switch(chan_num) {
			case 1: ASSERT(0);
			case 3: ipp_cpp::ippiSwapChannels_C3R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this,(const int *)&Order); break;
			case 4: ipp_cpp::ippiSwapChannels_AC4R(IPPARG_IMG(*this),IPPARG_IMG(dst),*this,(const int *)&Order); break;
	}	}
	FORCEINL void AlphaBlend(const Image_Ref& src1,const Image_Ref& src2,float alpha_1, float alpha_2=1.0f,int AlphaType = BlendMethod_AlphaOver)
	{	ASSERT_SIZE(*this,src1);
		ASSERT_SIZE(*this,src2);
		ASSERT(GetBPV()==8 || GetBPV()==16); //LDR only 
		ASSERT(alpha_1>=0.0f && alpha_1<=1.0f);
		ASSERT(alpha_2>=0.0f && alpha_2<=1.0f);
		Vec2i a;
		a.x = ((int)rt::TypeTraits<ValueType>::MaxVal())*alpha_1;
		a.y = ((int)rt::TypeTraits<ValueType>::MaxVal())*alpha_2;

		switch(chan_num) {
			case 1: ipp_cpp::ippiAlphaComp_C1R(	IPPARG_IMG(src1),a.x,
										IPPARG_IMG(src2),a.y,
										IPPARG_IMG(*this),*this,(IppiAlphaType)BlendMethod_AlphaOver); 
					break;
			case 3: ipp_cpp::ippiAlphaComp_C3R(	IPPARG_IMG(src1),a.x,
										IPPARG_IMG(src2),a.y,
										IPPARG_IMG(*this),*this,(IppiAlphaType)BlendMethod_AlphaOver); 
					break;
			case 4: ipp_cpp::ippiAlphaComp_C4R(	IPPARG_IMG(src1),a.x,
										IPPARG_IMG(src2),a.y,
										IPPARG_IMG(*this),*this,(IppiAlphaType)BlendMethod_AlphaOver); 
					break;
	}	}
	FORCEINL void AlphaBlend(const Image_Ref& src1,const Image_Ref& src2,int AlphaType = BlendMethod_AlphaOver)
	{	ASSERT_SIZE(*this,src1);
		ASSERT_SIZE(*this,src2);
		ASSERT(GetBPV()==8 || GetBPV()==16); //LDR only 
		ASSERT(chan_num==4);
		ipp_cpp::ippiAlphaComp_AC4R(IPPARG_IMG(src1),IPPARG_IMG(src2),IPPARG_IMG(*this),*this,(IppiAlphaType)AlphaType);
	}
	FORCEINL void Bitwise_And(const t_Val & p)
	{	switch(chan_num) {
			case 1: ipp_cpp::ippiAnd_C1R(p.x,IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
			case 3: ipp_cpp::ippiAnd_C3R(p,IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
			case 4: ipp_cpp::ippiAnd_C4R(p,IPPARG_IMG(src),IPPARG_IMG(*this),*this); break;
	}	}
	FORCEINL void Threshold_LessThenSet(const t_Val& Threshold,const t_Val& Val)
	{	switch(chan_num) {
		case 1: ipp_cpp::ippiThreshold_LTVal_C1R(IPPARG_IMG(*this),*this,Threshold[0],Val[0]); break;
		case 3: ipp_cpp::ippiThreshold_LTVal_C3R(IPPARG_IMG(*this),*this,Threshold,Val); break;
		case 4: ipp_cpp::ippiThreshold_LTVal_C4R(IPPARG_IMG(*this),*this,Threshold,Val); break;
	}	}
	FORCEINL void ThresholdTo_LessThenSet(const t_Val& Threshold,const t_Val& Val,CImage& x) const
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {
		case 1: ipp_cpp::ippiThreshold_LTVal_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),x,Threshold[0],Val[0]); break;
		case 3: ipp_cpp::ippiThreshold_LTVal_C3R(IPPARG_IMG(*this),IPPARG_IMG(x),x,Threshold,Val); break;
		case 4: ipp_cpp::ippiThreshold_LTVal_C4R(IPPARG_IMG(*this),IPPARG_IMG(x),x,Threshold,Val); break;
	}	}
	FORCEINL void Threshold_GreatThenSet(const t_Val& Threshold,const t_Val& Val)
	{	switch(chan_num) {
		case 1: ipp_cpp::ippiThreshold_GTVal_C1R(IPPARG_IMG(*this),*this,Threshold[0],Val[0]); break;
		case 3: ipp_cpp::ippiThreshold_GTVal_C3R(IPPARG_IMG(*this),*this,Threshold,Val); break;
		case 4: ipp_cpp::ippiThreshold_GTVal_C4R(IPPARG_IMG(*this),*this,Threshold,Val); break;
	}	}
	FORCEINL void ThresholdTo_GreatThenSet(const t_Val& Threshold,const t_Val& Val,CImage& x) const
	{	ASSERT_SIZE(*this,x);
		switch(chan_num) {
		case 1: ipp_cpp::ippiThreshold_GTVal_C1R(IPPARG_IMG(*this),IPPARG_IMG(x),x,Threshold[0],Val[0]); break;
		case 3: ipp_cpp::ippiThreshold_GTVal_C3R(IPPARG_IMG(*this),IPPARG_IMG(x),x,Threshold,Val); break;
		case 4: ipp_cpp::ippiThreshold_GTVal_C4R(IPPARG_IMG(*this),IPPARG_IMG(x),x,Threshold,Val); break;
	}	}
	*/
#endif // #ifdef PLATFORM_INTEL_IPP_SUPPORT
};



template<typename t_Value,int Channel>
class Image:public Image_Ref<t_Value, Channel>
{
    typedef rt::Vec<t_Value, Channel>	t_Val;
	typedef Image_Ref<t_Value, Channel>	Ref;
    typedef Image_Ref<t_Value, Channel>	_SC;

	void Attach(LPCVOID p = nullptr, UINT w = 0, UINT h = 0, UINT step = 0){ ASSERT(0); }
	void Attach(const Image_Ref<t_Value, Channel>& x){ ASSERT(0); }

	FORCEINL void	__SafeFree()
	{	
#ifdef PLATFORM_INTEL_IPP_SUPPORT
		if(Ref::lpData){ IPPCALL(ippiFree)(Ref::lpData); Ref::lpData = nullptr; }	
#else
		_SafeFree32AL(Ref::lpData);
#endif	// #ifdef PLATFORM_INTEL_IPP_SUPPORT
	}
public:
	FORCEINL Image(){}
	FORCEINL ~Image(){ __SafeFree(); }
	FORCEINL Image(const Image<t_Value, Channel>& x){ if(!x.IsEmpty()){ VERIFY(SetSize(x)); Ref::CopyFrom(x); } }
    FORCEINL Image(UINT w, UINT h){ SetSize(w,h); }
    
    template<typename T, int ch>
    FORCEINL Image(const Image<T, ch>& x){ if(!x.IsEmpty()){ VERIFY(SetSize(x)); Ref::CopyFrom(x); } }
	template<typename T, int ch>
    FORCEINL Image(const Image_Ref<T, ch>& x){ VERIFY(SetSize(x)); Ref::CopyFrom(x); }
    
	template<typename T,UINT ch>
	FORCEINL const Image_Ref<t_Value, Channel>& operator = (const Image_Ref<T,ch> & x){ Ref::CopyFrom(x); return *this; }
	FORCEINL const Ref& operator = (const Ref& x){ Ref::CopyFrom(x); return *this; }
    
	template<typename t_Value2,UINT Channel2>
	FORCEINL bool	SetSize(const Image_Ref<t_Value2, Channel2>& x){ return SetSize(x.GetWidth(), x.GetHeight()); }
	FORCEINL bool	SetSize(const IppiSize& x){ return SetSize(x.width, x.height); }
	INLFUNC  bool	SetSize(UINT w, UINT h)
	{	if(w==Ref::Width && h==Ref::Height){ return true; }
		else
		{	__SafeFree();
			int step_size = 0;
			if(w&&h)
			{	
#ifdef PLATFORM_INTEL_IPP_SUPPORT
				switch(sizeof(t_Value)*8*Channel)
				{
				case 8:  // 1c8u
					Ref::lpData = (typename Ref::t_Val*)IPPCALL(ippiMalloc_8u_C1)(w,h,&step_size); break;
				case 16: // 2c8u,1c16u
					Ref::lpData = (typename Ref::t_Val*)IPPCALL(ippiMalloc_16u_C1)(w,h,&step_size); break;
				case 24: // 3c8u
					Ref::lpData = (typename Ref::t_Val*)IPPCALL(ippiMalloc_8u_C3)(w,h,&step_size); break;
				case 32: // 4c8u,2c16u,1c32f
					Ref::lpData = (typename Ref::t_Val*)IPPCALL(ippiMalloc_8u_C4)(w,h,&step_size); break;
				case 48: // 3c16u
					Ref::lpData = (typename Ref::t_Val*)IPPCALL(ippiMalloc_16u_C3)(w,h,&step_size); break;
				case 64: // 4c16u,2c32f,1c64f
					Ref::lpData = (typename Ref::t_Val*)IPPCALL(ippiMalloc_16u_C4)(w,h,&step_size); break;
				case 96: // 3c32f
					Ref::lpData = (typename Ref::t_Val*)IPPCALL(ippiMalloc_32s_C3)(w,h,&step_size); break;
				case 128:// 4c32f,2c64f
					Ref::lpData = (typename Ref::t_Val*)IPPCALL(ippiMalloc_32s_C4)(w,h,&step_size); break;
				default:
					ASSERT(0); //unsupported pixel format
				}
#else	// PLATFORM_INTEL_IPP_SUPPORT
				step_size = (int)_EnlargeTo32AL(sizeof(t_Value)*Channel*w);
				Ref::lpData = (typename Ref::t_Val*)_Malloc32AL(BYTE, step_size*h,);
#endif  // PLATFORM_INTEL_IPP_SUPPORT
				if(Ref::lpData == nullptr)
				{	Ref::Width=Ref::Height=0;
					return false;
				}
			}
			
			Ref::Width=w; Ref::Height=h; 
			Ref::Step_Bytes = (UINT)step_size;
			return true;
		}
	}
	FORCEINL bool	Load(LPCSTR fn)
	{	
		os::FileRead<BYTE> f(fn);
		_tagImageCodec ic = ImageDecoder::DecodeFormat(f, (UINT)f.GetSize());
		if(ic == ImageCodec_PFM)
		{	if(!rt::IsTypeSame<float, t_Value>::Result)return false;
			image_codec::_PFM_Header h;
			return	image_codec::_Open_PFM(fn, &h) && 
					SetSize(h.width, h.height) &&
                    image_codec::_Read_PFM(&h, (LPFLOAT)_SC::lpData, Channel, _SC::GetStep());
		}
		else if(ic >= ImageCodec_EXR && ic < ImageCodec_EXR_END)
		{
			Imf::InputFile file (fn);
			Imath::Box2i dw = file.header().dataWindow();
			if(!SetSize(dw.max.x - dw.min.x + 1, dw.max.y - dw.min.y + 1))return false;

			Imf::PixelType pt;
			if(rt::IsTypeSame<float, t_Value>::Result)
			{	pt = Imf::FLOAT;
			}
			else if(rt::IsTypeSame<UINT, t_Value>::Result)
			{	pt = Imf::UINT;
			}
			else
			{	
				Image<float, Channel> img_temp;
				if(img_temp.Load(fn))
				{	this->CopyFrom(img_temp);
					return true;
				}
				else 
					return false;
			}

			Imf::FrameBuffer frameBuffer;
			switch(Channel)
			{	case 1: frameBuffer.insert ("G", Imf::Slice(pt,	(char *)_SC::lpData, sizeof(t_Val),	_SC::GetStep()));
						break;
				case 2:	frameBuffer.insert ("G", Imf::Slice(pt,	(char *)_SC::lpData					  , sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("Z", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value), sizeof(t_Val),	_SC::GetStep()));
						break;
				case 3: frameBuffer.insert ("R", Imf::Slice(pt,	(char *)_SC::lpData					    , sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("G", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value)  , sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("B", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value)*2, sizeof(t_Val),	_SC::GetStep()));
						break;
				case 4:	frameBuffer.insert ("R", Imf::Slice(pt,	(char *)_SC::lpData					    , sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("G", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value)  , sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("B", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value)*2, sizeof(t_Val),	_SC::GetStep()));
						frameBuffer.insert ("A", Imf::Slice(pt,	((char *)_SC::lpData) + sizeof(t_Value)*3, sizeof(t_Val),	_SC::GetStep()));
						break;
				default: return false;
			}

			try{
				file.setFrameBuffer (frameBuffer);
				file.readPixels (dw.min.y, dw.max.y);
				return true;
			}
			catch(...)
			{	return false;
			}
		}
		else return Load(f,(UINT)f.GetSize());
	}
	FORCEINL bool	Load(LPCVOID data, UINT data_len, UINT* pOriginalChannel = nullptr)
	{	ImageDecoder	dec;
		if(	data_len &&
			dec.Decode(data,data_len) &&
			SetSize(dec.GetImageWidth(), dec.GetImageHeight())
		)
		{	if(pOriginalChannel)*pOriginalChannel = dec.GetImageChannel();
			switch(dec.GetImageChannel())
			{	case 1:	*((Ref*)this) = Image_Ref<BYTE,1>(dec.GetOutput(), dec.GetImageWidth(), dec.GetImageHeight(), dec.GetImageStep()); return true;
				case 2: *((Ref*)this) = Image_Ref<BYTE,2>(dec.GetOutput(), dec.GetImageWidth(), dec.GetImageHeight(), dec.GetImageStep()); return true;
				case 3: *((Ref*)this) = Image_Ref<BYTE,3>(dec.GetOutput(), dec.GetImageWidth(), dec.GetImageHeight(), dec.GetImageStep()); return true;
				case 4: *((Ref*)this) = Image_Ref<BYTE,4>(dec.GetOutput(), dec.GetImageWidth(), dec.GetImageHeight(), dec.GetImageStep()); return true;
				default: return false;
			}
		}
		return false;
	}
	FORCEINL void	Empty(){ __SafeFree(); }
};

typedef Image_Ref<float,1>	ImageRef_1c32f;
typedef Image_Ref<float,3>	ImageRef_3c32f;
typedef Image_Ref<float,4>	ImageRef_4c32f;

typedef Image_Ref<BYTE,1>	ImageRef_1c8u;
typedef Image_Ref<BYTE,3>	ImageRef_3c8u;
typedef Image_Ref<BYTE,4>	ImageRef_4c8u;

typedef Image<float,1>	Image_1c32f;
typedef Image<float,3>	Image_3c32f;
typedef Image<float,4>	Image_4c32f;

typedef Image<BYTE,1>	Image_1c8u;
typedef Image<BYTE,3>	Image_3c8u;
typedef Image<BYTE,4>	Image_4c8u;


} // namespace ipp



