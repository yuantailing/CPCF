#include "gl_texture.h"


namespace gl
{

void TextureBase::_GenTextureName()
{
	if(!_TexName)glGenTextures(1, &_TexName);
	_LogGLError;
	ASSERT(_TexName);
}

void TextureBase::Destroy()
{
	if(_TexName)
	{
		glDeleteTextures(1,&_TexName);
		_LogGLError;
		_TexName = 0;
	}
}

Texture2D::Texture2D()
{
	_TextureFormat = TEXFMT_None;
	TextureWrapMode();
	TextureFilterSet();
}


void Texture2D::DefineTexture(UINT TextureFormat, UINT cx,UINT cy, LPCVOID pData, UINT image_step_byte)
{
	_TextureFormat = TextureFormat;
	switch(_TextureFormat)
	{
	case TEXFMT_None:	ASSERT(0); break;
	case TEXFMT_1c8u:	((Texture1c8u*) this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_2c8u:	((Texture2c8u*) this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_3c8u: 	((Texture3c8u*) this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_4c8u: 	((Texture4c8u*) this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_1c16f: 	((Texture1c16f*)this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_2c16f: 	((Texture2c16f*)this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_3c16f: 	((Texture3c16f*)this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_4c16f: 	((Texture4c16f*)this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_1c32f: 	((Texture1c32f*)this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_2c32f: 	((Texture2c32f*)this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_3c32f: 	((Texture3c32f*)this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	case TEXFMT_4c32f:	((Texture4c32f*)this)->DefineTexture(cx,cy,pData,image_step_byte); break;
	default: ASSERT(0);
	}
}

void Texture2D::DefineTextureMipmapped(UINT TextureFormat, UINT cx,UINT cy, LPCVOID pData, UINT image_step_byte)
{
	_TextureFormat = TextureFormat;
	switch(_TextureFormat)
	{
	case TEXFMT_None:	ASSERT(0); break;
	case TEXFMT_1c8u:	((Texture1c8u*) this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_2c8u:	((Texture2c8u*) this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_3c8u: 	((Texture3c8u*) this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_4c8u: 	((Texture4c8u*) this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_1c16f: 	((Texture1c16f*)this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_2c16f: 	((Texture2c16f*)this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_3c16f: 	((Texture3c16f*)this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_4c16f: 	((Texture4c16f*)this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_1c32f: 	((Texture1c32f*)this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_2c32f: 	((Texture2c32f*)this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_3c32f: 	((Texture3c32f*)this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	case TEXFMT_4c32f:	((Texture4c32f*)this)->DefineTextureMipmapped(cx,cy,pData,image_step_byte); break;
	default: ASSERT(0);
	}
}


} // namespace gl

