//
//  main.m
//  gltest
//
//  Created by devteam on 4/24/13.
//  Copyright (c) 2013 devteam. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "AppDelegate.h"
#include "../../../../shared_api/os/user_inputs.h"

int main(int argc, char *argv[])
{
	//return NSApplicationMain(argc, (const char **)argv);
	
	[NSApplication sharedApplication];
    [NSApp setDelegate:[[AppDelegate alloc] init]];
    [NSBundle loadNibNamed:@"MainMenu" owner:[NSApp delegate]];
    [NSApp finishLaunching];
	
	os::UserInputEventPump::Get()->MainLoop();
}
