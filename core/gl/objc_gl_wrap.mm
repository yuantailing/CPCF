//////////////////////////////////////////////////////////////////////
// Cicada Project Shared APIs
//
// (C) Copyright Cicada DevTeam 2012.
//
// Permission to copy, use, modify, sell and distribute this software
// is granted provided this copyright notice appears in all copies.
// This software is provided "as is" without express or implied
// warranty, and with no claim as to its suitability for any purpose.
//
// Absolutely no warranty!!
//////////////////////////////////////////////////////////////////////

#include "predefines.h"

//////////////////////////////////////////////////////
// Mac Stuffs
#if defined(PLATFORM_MAC)
#import <Cocoa/Cocoa.h>

extern "C" LPVOID _objc_opengl_rendercontext_get(LPVOID pNSOpenGLView)
{
	return [(NSOpenGLView*)pNSOpenGLView openGLContext];
}

extern "C" LPVOID _objc_opengl_rendercontext_current()
{
	return [NSOpenGLContext currentContext];
}

extern "C" BOOL _objc_opengl_rendercontext_makecurrent(LPVOID pNSOpenGLContext)
{
	if(pNSOpenGLContext)
	{	[(NSOpenGLContext*)pNSOpenGLContext makeCurrentContext];
		return TRUE;
	}
	else return FALSE;
}

extern "C" void _objc_opengl_rendercontext_swapbuffer(LPVOID pNSOpenGLContext)
{
	if(pNSOpenGLContext)
	{	[(NSOpenGLContext*)pNSOpenGLContext flushBuffer];
	}
}

extern "C" void _objc_opengl_swapinterval(LPVOID pNSOpenGLContext, GLint v)
{
	[(NSOpenGLContext*)pNSOpenGLContext setValues:&v forParameter:NSOpenGLCPSwapInterval];
}

#endif


//////////////////////////////////////////////////////
// iOS Stuffs
#if defined(PLATFORM_IOS)
#import <UIKit/UIDevice.h>
#import <OpenGLES/EAGL.h>
#import <OpenGLES/EAGLDrawable.h>
#import <QuartzCore/QuartzCore.h>

extern "C" BOOL		_objc_opengl_rendercontext_makecurrent(LPVOID pEAGLContext)
{	if(pEAGLContext)
	{	return [EAGLContext setCurrentContext:(__bridge EAGLContext*)pEAGLContext];
	}else return FALSE;
}

extern "C" LPVOID	_objc_opengl_rendercontext_current() // return EAGLContext*
{	return (__bridge LPVOID)[EAGLContext currentContext];
}

extern "C" double _objc_opengl_get_screen_scale()
{
	if([[UIScreen mainScreen] respondsToSelector:@selector(displayLinkWithTarget:selector:)])
	{
		return [UIScreen mainScreen].scale;
	}
	else
	{	return 1.0;	}
}

extern "C" LPVOID	_objc_opengl_rendercontext_create(LPVOID pUIView, GLuint* pRenderingBuffers)  // EAGLContext*
{	EAGLRenderingAPI api = kEAGLRenderingAPIOpenGLES2;
	EAGLContext* rc;
    if(	(rc = [[EAGLContext alloc] initWithAPI:api]) &&
		[EAGLContext setCurrentContext:rc]
	)
	{	UIView* view = (__bridge UIView*)pUIView;
		CAEAGLLayer* layer = (CAEAGLLayer*)(view.layer);
		layer.opaque = TRUE;
		float sscale = _objc_opengl_get_screen_scale();
		
		GLuint framebuffer,_colorRenderBuffer,_depthRenderBuffer;
		glGenRenderbuffers(1, &_depthRenderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _depthRenderBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, (int)view.frame.size.width*sscale, (int)view.frame.size.height*sscale);
		
		//NSLog(@"Frame: %d %d", (int)view.frame.size.width*sscale, (int)view.frame.size.height*sscale);
		
		glGenRenderbuffers(1, &_colorRenderBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, _colorRenderBuffer);
		[rc renderbufferStorage:GL_RENDERBUFFER fromDrawable:layer];
		
		glGenFramebuffers(1, &framebuffer);
		glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _colorRenderBuffer);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _depthRenderBuffer);
			
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
            NSLog(@"Failed to make complete framebuffer object %x", glCheckFramebufferStatus(GL_FRAMEBUFFER));
		
		return (__bridge LPVOID)rc;
	}
	return NULL;
}

extern "C" void		_objc_opengl_rendercontext_destroy(LPVOID pEAGLContext)  // EAGLContext*
{
#if !__has_feature(objc_arc)
    if(pEAGLContext)
		[(__bridge EAGLContext*)pEAGLContext dealloc];
#endif
}

extern "C" void _objc_opengl_setup_disaply_link(LPVOID pUIView)
{
	CADisplayLink* displayLink = [CADisplayLink displayLinkWithTarget:((__bridge UIView*)pUIView) selector:@selector(__vframe)];
	[displayLink addToRunLoop:[NSRunLoop currentRunLoop] forMode:NSDefaultRunLoopMode];
}

extern "C" void _objc_opengl_rendercontext_swapbuffer(LPVOID pEAGLContext)
{
	if(pEAGLContext)
	{	[(__bridge EAGLContext*)pEAGLContext presentRenderbuffer:GL_RENDERBUFFER];
	}
}


#endif
