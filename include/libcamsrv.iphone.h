
#ifndef _LIBCAMSRV_IPHONE_H_
#define _LIBCAMSRV_IPHONE_H_


#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#include "libcamsrv.h"
typedef void (*_NotifyFrame)();


@interface CCamSrvIphone : NSObject
{
	NSTimer *m_tmrSendFrame;
	CFRunLoopRef m_runLoop;
	NSThread *m_threadRunLoop;
}
- (void) SendFrame;
- (bool) StartCamsrv:(const char *)cmdline ;
- (void) StopCamsrv ;
- (bool) OpenCamera:(int) index;
- (void) CloseCamera:(int) index;
- (bool) OpenAudio;
- (void) CloseAudio;
- (bool) IsStarted;
- (void) SetCamID:(const char *)camID;
- (UIImage *) GetCurrentImage;
- (void) ReceiveFrame:(BOOL)bReceive;
- (void) SetFrameNotify:(_NotifyFrame) notify;
- (void) SetDirectConnectionNotify:(_NotifyDirectConnectionClosed) notify;
- (BOOL) AddDirectConnection:(int) soc;
@end


#endif