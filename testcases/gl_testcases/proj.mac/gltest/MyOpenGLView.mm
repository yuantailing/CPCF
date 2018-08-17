//
//  MyOpenGLView.m
//  gltest
//
//  Created by devteam on 4/24/13.
//  Copyright (c) 2013 devteam. All rights reserved.
//

#import "MyOpenGLView.h"
#import <QuartzCore/QuartzCore.h>

@implementation MyOpenGLView

- (id) initWithFrame:(NSRect)frameRect
{
	NSOpenGLPixelFormatAttribute attributes [] = {
        NSOpenGLPFAWindow,
        NSOpenGLPFADoubleBuffer,	// double buffered
		NSOpenGLPFAColorSize, 32,
		NSOpenGLPFAAlphaSize, 8,
		NSOpenGLPFADepthSize, 16,
        (NSOpenGLPixelFormatAttribute)nil
    };
    NSOpenGLPixelFormat* pixfmt = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes] autorelease];
	
	self = [super initWithFrame: frameRect pixelFormat: pixfmt];

	_Core.Init(self);
    return self;
}
	 
BIND_UIEVENTPUMP(_Core.GetPump())

@end
