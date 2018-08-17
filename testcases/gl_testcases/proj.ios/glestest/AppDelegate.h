//
//  AppDelegate.h
//  glestest
//
//  Created by devteam on 4/26/13.
//  Copyright (c) 2013 devteam. All rights reserved.
//

#import <UIKit/UIKit.h>
#import "glesView.h"

@interface AppDelegate : UIResponder <UIApplicationDelegate>
{
	glesView* _glView;
}

@property (strong, nonatomic) UIWindow *window;
@property (nonatomic, retain) IBOutlet glesView *view;

@end
