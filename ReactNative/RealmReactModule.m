////////////////////////////////////////////////////////////////////////////
//
// Copyright 2015 Realm Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////

#import <objc/runtime.h>
#import "RealmReactModule.h"
#import "Base/RCTLog.h"
#import "Base/RCTBridge.h"

@import RealmJS;
@import JavaScriptCore;

@interface RCTJavaScriptContext : NSObject <RCTInvalidating>
@property (nonatomic, assign, readonly) JSGlobalContextRef ctx;
- (void)executeBlockOnJavaScriptQueue:(dispatch_block_t)block;
- (instancetype)initWithJSContext:(JSGlobalContextRef)context;
@end

RCT_EXTERN id<RCTJavaScriptExecutor> RCTGetLatestExecutor(void);

@implementation Realm

RCT_EXPORT_MODULE()

@synthesize bridge = _bridge;

-(void)setBridge:(RCTBridge *)bridge {
    _bridge = bridge;

    id contextExecutor = RCTGetLatestExecutor();
    [contextExecutor executeBlockOnJavaScriptQueue:^{
        JSContextRef ctx;
        if ([contextExecutor isKindOfClass:NSClassFromString(@"RCTWebViewExecutor")]) {
            Ivar ivar = class_getInstanceVariable([contextExecutor class], "_webView");
            UIWebView *webView = object_getIvar(contextExecutor, ivar);
            if (!webView) {
                webView = [[UIWebView alloc] init];
                object_setIvar(contextExecutor, ivar, webView);
            }
            JSContext *jsctx = [webView valueForKeyPath:@"documentView.webView.mainFrame.javaScriptContext"];
            ctx = jsctx.JSGlobalContextRef;
        }
        else {
            Ivar ivar = class_getInstanceVariable([contextExecutor class], "_context");
            RCTJavaScriptContext *rctJSContext = object_getIvar(contextExecutor, ivar);
            if (rctJSContext) {
                ctx = rctJSContext.ctx;
            }
            else {
                JSGlobalContextRef gctx = JSGlobalContextCreate(NULL);
                object_setIvar(contextExecutor, ivar, [[RCTJavaScriptContext alloc] initWithJSContext:gctx]);
                ctx = gctx;
            }
        }

        [RealmJS initializeContext:ctx];

        RCTLogInfo(@"Realm initialized");
    }];
}


@end

