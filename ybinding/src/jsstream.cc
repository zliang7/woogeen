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

#include "jsstream.h"

using namespace woogeen;
using namespace jsnipp;

void Stream::setup(JSObject cls) {
    cls.defineProperty("id", JSPropertyAccessor(JSNativeGetter<Stream, &Stream::id>()));
    cls.setProperty("disableAudio", JSNativeMethod<Stream, &Stream::disableAudio>());
    cls.setProperty("enableAudio", JSNativeMethod<Stream, &Stream::enableAudio>());
    cls.setProperty("disableVideo", JSNativeMethod<Stream, &Stream::disableVideo>());
    cls.setProperty("enableVideo", JSNativeMethod<Stream, &Stream::enableVideo>());
    cls.setProperty("attachVideoRenderer", JSNativeMethod<Stream, &Stream::attachVideoRenderer>());
    cls.setProperty("detachVideoRenderer", JSNativeMethod<Stream, &Stream::detachVideoRenderer>());
    cls.setProperty("attachAudioPlayer", JSNativeMethod<Stream, &Stream::attachAudioPlayer>());
    cls.setProperty("detachAudioPlayer", JSNativeMethod<Stream, &Stream::detachAudioPlayer>());
}

void RemoteStream::setup(JSObject cls) {
    Stream::setup(cls);
    cls.defineProperty("from", JSPropertyAccessor(JSNativeGetter<RemoteStream, &RemoteStream::from>()));
}

class JSParameters : JSObject {
public:
    JSParameters(const JSValue& jsval): JSObject(jsval) {}
    operator base::LocalCustomizedStreamParameters() const {
        JSBoolean video(getProperty("videoEnabled"));
        JSBoolean audio(getProperty("audioEnabled"));
        auto param = base::LocalCustomizedStreamParameters(video, audio);
        JSNumber width(getProperty("width"));
        JSNumber height(getProperty("height"));
        param.Resolution(width, height);
        return param;
    }
};

LocalCustomStream::LocalCustomStream(JSObject, JSArray args) : Stream(nullptr) {
    JSParameters param(args[0]);
    stream_ = std::make_shared<base::LocalCustomizedStream>(param);
}

#if ENABLE_LOCAL_CAMERA_STREAM
class JSLocalCameraStreamParameters : JSObject {
public:
    JSLocalCameraStreamParameters(const JSValue& jsval): JSObject(jsval){}
    operator base::LocalCameraStreamParameters() const;
};

LocalCameraStream::LocalCameraStream(JSObject, JSArray args): Stream(nullptr) {
    JSLocalCameraStreamParameters param(args[0]);
    int error = 0;
    stream_ = base::LocalCameraStream::Create(param, error);
}
void LocalCameraStream::setup(JSObject cls) {
    Stream::setup(cls);
    cls.setProperty("close", JSNativeMethod<LocalCameraStream, &LocalCameraStream::close>());
}
JSLocalCameraStreamParameters::operator base::LocalCameraStreamParameters() const {
    JSBoolean video(getProperty("videoEnabled"));
    JSBoolean audio(getProperty("audioEnabled"));
    base::LocalCameraStreamParameters param(video, audio);
    param.CameraId(JSString(getProperty("cameraID")));
    param.StreamName(JSString(getProperty("streamName")));
    if (hasProperty("fps"))
        param.Fps(JSNumber(getProperty("fps")));
    if (hasProperty("resolution")) {
        base::Resolution res = JSResolution(getProperty("resolution"));
        if (res.width | res.height)
            param.Resolution(res.width, res.height);
    }
    return param;
}
#endif
