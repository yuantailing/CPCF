//
//  glesView.m
//  glestest
//
//  Created by devteam on 4/26/13.
//  Copyright (c) 2013 devteam. All rights reserved.
//

#import "glesView.h"
#import <QuartzCore/QuartzCore.h>

@implementation glesView

+ (Class)layerClass
{
    return [CAEAGLLayer class];
}

- (id)initWithFrame:(CGRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
		[self setUserInteractionEnabled:TRUE];
        _Core.Init(self);
    }
    return self;
}

- (void)drawRect:(CGRect)rect
{
	os::UserInputEvent e;
	e.Device = os::UIDEV_VIEWPORT;
	e.Type = os::UIEVT_VIEWPORT_REDRAW;
	_Core.GetPump()._OnReceiveEventFromResponder(e);
}

- (void) __vframe
{
	os::UserInputEvent e;
	e.Device = os::UIDEV_VIEWPORT;
	e.Type = os::UIEVT_VIEWPORT_REDRAW;
	_Core.GetPump()._OnReceiveEventFromResponder(e);
}

- (void) touchesBegan:(NSSet *)touches withEvent:(UIEvent *)te
{
	os::UserInputEvent e;
	NSSet* tset = [te allTouches];
	if([tset count] == 1 || !_Core.GetPump().IsMultiTouchInterested())
	{
		e.Device = os::UIDEV_TOUCH;
		e.Type = os::UIEVT_MOUSE_DOWN;
		
		UITouch *touch = [tset anyObject];
		e.Position.copy([touch locationInView: self]);
		e.ButtonCode = os::BUTTON_TOUCH;
		_Core.GetPump()._OnReceiveEventFromResponder(e);
	}
}

- (void) touchesMoved:(NSSet *)touches withEvent:(UIEvent *)te
{
	os::UserInputEvent e;
	NSSet* tset = [te allTouches];
	if([tset count] == 1 || !_Core.GetPump().IsMultiTouchInterested())
	{
		e.Device = os::UIDEV_TOUCH;
		e.Type = os::UIEVT_TOUCH_DRAG;
		
		UITouch *touch = [tset anyObject];
		e.Position.copy([touch locationInView: self]);
		e.ButtonCode = os::BUTTON_TOUCH;
		_Core.GetPump()._OnReceiveEventFromResponder(e);
	}
}

- (void) touchesEnded:(NSSet *)touches withEvent:(UIEvent *)te
{
	os::UserInputEvent e;
	NSSet* tset = [te allTouches];
	if([tset count] == 1 || !_Core.GetPump().IsMultiTouchInterested())
	{
		e.Device = os::UIDEV_TOUCH;
		e.Type = os::UIEVT_MOUSE_UP;
		
		UITouch *touch = [tset anyObject];
		e.Position.copy([touch locationInView: self]);
		e.ButtonCode = os::BUTTON_TOUCH;
		_Core.GetPump()._OnReceiveEventFromResponder(e);
	}
}


@end
