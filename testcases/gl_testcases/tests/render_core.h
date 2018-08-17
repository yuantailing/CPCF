#pragma once

#include "../../../core/os/user_inputs.h"
#include "../../../core/gl/gl_basic.h"
#include "../../../core/gl/gl_animator.h"
#include "../../../core/gl/gl_shader.h"
#include "../../../core/gl/gl_texture.h"
#include "../../../core/gl/gl_gdi.h"
#include "../../../core/gl/gl_object.h"

#include "../../../core/ext/ipp/ipp_image.h"
#include "../../../core/inet/http_client.h"


class RenderCore: public os::UserInputSubscriber
{
protected:
	virtual void OnUserInputEvent(const os::UserInputEvent& x);

protected:
	os::UserInputEventPump	_Pump;
	gl::RenderContext		_RC;
	gl::FramerateEstimator	_FPS;
	gl::Camera				_Cam;	
	gl::aniArcball			_GlobalArcball;
	UINT					_GlobalArcball_StateVersion;
	//gl::FlyCamera			_Cam;

	gl::gdiCanvas			_DC;
	gl::gdiFont				_Font;
	ipp::Image_4c8u			_imgLogo;

	gl::ShaderProgram		_ShaderProgram;
	gl::ShaderProgram		_progTextured;

	gl::Texture2D			_Texture;

	gl::Object				_R8;

	gl::VertexAttributeBuffer	_PointCloud;
	UINT						_PointCount;
	

protected:
	void UpdateCamera();
	void OnRendering();
	bool PostInit();

public:
	INLFUNC os::UserInputEventPump& GetPump(){ return _Pump; }
	RenderCore();
#if	defined(PLATFORM_WIN)
	bool Init(HWND mainwnd);
#elif defined(PLATFORM_MAC)
	bool Init(LPVOID pNSOpenGLView);
#elif defined(PLATFORM_IOS)
	bool Init(LPVOID pUIView);
#else
	ASSERT_STATIC(0);
#endif
};

