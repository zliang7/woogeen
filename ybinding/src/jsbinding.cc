/*
 * Copyright © 2016 Intel Corporation. All Rights Reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <string>
#include <vector>

#include <woogeen/base/deviceutils.h>
#include <woogeen/base/globalconfiguration.h>

#include <Log.h>
#define LOG_TAG "woogeen"
#include <jsnipp.h>

#include "jsstream.h"
#include "jsconfclient.h"
#include "jsfilegenerator.h"

using namespace woogeen;
using namespace jsnipp;

JSValue getVideoCaptureDevices(JSObject, JSArray) {
    JSObject result;
    std::vector<std::string> devices = base::DeviceUtils::VideoCapturerIds();
    for (auto dev: devices) {
        auto reslist = base::DeviceUtils::VideoCapturerSupportedResolutions(dev);
        JSArray jsarr(reslist.size());
        size_t i = 0;
        for (auto res: reslist)
            jsarr.setElement(i++, JSResolution(res));
        result.setProperty(dev, jsarr);
    }
    return result;
}

JSValue setCustomAudioInput(JSObject, JSArray args) {
    JSNativeObject<base::AudioFrameGeneratorInterface> jsobj(args[0]);
    auto ptr = jsobj.native();
    auto generator = std::unique_ptr<base::AudioFrameGeneratorInterface>(ptr);
    jsobj.reset(nullptr);
    base::GlobalConfiguration::SetCustomizedAudioInputEnabled(true, std::move(generator));
    return JSUndefined();
}

JSValue setCustomVideoDecoder(JSObject, JSArray args) {
    // TODO
    return JSUndefined();
}

JSValue setCustomVideoEncoder(JSObject, JSArray args) {
    // TODO
    return JSUndefined();
}

//////////////////////////////////////////////////////////////////////
// for jsnipp feature testing

/* for test raw jsni 1.1 interface
// jsni version:
void SayHello(JSNIEnv* env, CallbackInfo info) {
    JsValue js_str = env->NewStringFromUtf8("Hello, World!", 14);
    env->SetReturnValue(info, js_str);
}*/
// jsnipp version:
JSValue SayHello(JSObject, JSArray args) {
    return JSString("Hello, world");
}

class Echo {
public:
    // constuctor
    Echo(JSObject, JSArray args) {
        if (args.length() > 0)
            prefix_ = JSString(args[0]);
        else
            prefix_ = "???: ";
    }
    // method
    JSValue echo(JSObject, JSArray args) {
        std::string str;
        if (args.length() > 0)
            str = JSString(args[0]);
        else
            str = "";
        return JSString(prefix_ + str);
    }
    // getter
    JSValue prefix() {
        return JSString(prefix_);
    }
    // initializer
    static void setup(JSObject cls) {
        cls.setProperty("echo", JSNativeMethod<Echo, &Echo::echo>());
        cls.defineProperty("prefix", JSNativeGetter<Echo, &Echo::prefix>());
    }
private:
    std::string prefix_;
};

// JSNI Entry point
__attribute__ ((visibility("default")))
int JSNI_Init(JSNIEnv* env, JsValue exports) {
    YUNOS_LOG(Info, LOG_TAG, "Woogeen JSNI module is loaded");
    JSValue::setup(env);
    JSObject jsobj(exports);
    jsobj.setProperty("ConferenceClient", JSNativeConstructor<ConferenceClient>(&ConferenceClient::setup));
    jsobj.setProperty("LocalCameraStream", JSNativeConstructor<LocalCameraStream>(&LocalCameraStream::setup));
    jsobj.setProperty("LocalCustomStream", JSNativeConstructor<LocalCustomStream>(&LocalCustomStream::setup));
    jsobj.setProperty("RemoteStream", JSNativeConstructor<RemoteStream>(&RemoteStream::setup));
    jsobj.setProperty("FileAudioFrameGenerator", JSNativeConstructor<FileAudioFrameGenerator>());
    jsobj.setProperty("FileVideoFrameGenerator", JSNativeConstructor<FileVideoFrameGenerator>());
    jsobj.setProperty("getVideoCaptureDevices", JSNativeFunction<getVideoCaptureDevices>());
    jsobj.setProperty("setCustomAudioInput", JSNativeFunction<setCustomAudioInput>());
    jsobj.setProperty("setCustomVideoDecoder", JSNativeFunction<setCustomVideoDecoder>());
    jsobj.setProperty("setCustomVideoEncoder", JSNativeFunction<setCustomVideoEncoder>());

    // for jsnipp feature testing
    jsobj.setProperty("Echo", JSNativeConstructor<Echo>(&Echo::setup));
    jsobj.setProperty("sayHello", JSNativeFunction<SayHello>());
    //env->RegisterMethod(exports, "sayHello", SayHello);
    return JSNI_VERSION_1_1;
}
