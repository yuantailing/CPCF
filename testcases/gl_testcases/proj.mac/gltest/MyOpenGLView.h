//
//  MyOpenGLView.h
//  gltest
//
//  Created by devteam on 4/24/13.
//  Copyright (c) 2013 devteam. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "../../tests/render_core.h"


@interface MyOpenGLView : NSOpenGLView {
@protected
@public
	RenderCore _Core;
    id _localEventMonitor;
}
@end
