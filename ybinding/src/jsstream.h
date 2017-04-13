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
#pragma once

#include <string>

#include <woogeen/base/globalconfiguration.h>
#include <woogeen/base/stream.h>

#include <jsnipp.h>

#include "yunosaudioframegenerator.h"

using namespace woogeen;
using namespace jsnipp;


class JSResolution : public JSObject {
public:
    JSResolution(const JSValue& jsval): JSObject(jsval) {}
    JSResolution(const base::Resolution& res): JSObject({
        { "width", JSNumber(res.width) },
        { "height", JSNumber(res.height) }
    }){}

    operator base::Resolution() const {
        base::Resolution res;
        res.width = JSNumber(getProperty("width"));
        res.height = JSNumber(getProperty("height"));
        return res;
    }
};


class Stream {
    JSValue id() {
        return JSString(stream_->Id());
    }

    JSValue disableAudio(JSObject, JSArray) {
        stream_->DisableAudio();
        return JSUndefined();
    }
    JSValue enableAudio(JSObject, JSArray) {
        stream_->EnableAudio();
        return JSUndefined();
    }
    JSValue disableVideo(JSObject, JSArray) {
        stream_->DisableVideo();
        return JSUndefined();
    }
    JSValue enableVideo(JSObject, JSArray) {
        stream_->EnableVideo();
        return JSUndefined();
    }
    JSValue attachVideoRenderer(JSObject, JSArray) {
        //TODO
        return JSUndefined();
    }
    JSValue detachVideoRenderer(JSObject, JSArray) {
        stream_->DetachVideoRenderer();
        return JSUndefined();
    }
    JSValue attachAudioPlayer(JSObject, JSArray) {
        //TODO
        return JSUndefined();
    }
    JSValue detachAudioPlayer(JSObject, JSArray) {
        stream_->DetachAudioPlayer();
        return JSUndefined();
    }

public:
    Stream() = delete;

    static void setup(JSObject cls) {
        cls.defineProperty("id", JSNativeGetter<Stream, &Stream::id>());
        cls.setProperty("disableAudio", JSNativeMethod<Stream, &Stream::disableAudio>());
        cls.setProperty("enableAudio", JSNativeMethod<Stream, &Stream::enableAudio>());
        cls.setProperty("disableVideo", JSNativeMethod<Stream, &Stream::disableVideo>());
        cls.setProperty("enableVideo", JSNativeMethod<Stream, &Stream::enableVideo>());
        cls.setProperty("attachVideoRenderer", JSNativeMethod<Stream, &Stream::attachVideoRenderer>());
        cls.setProperty("detachVideoRenderer", JSNativeMethod<Stream, &Stream::detachVideoRenderer>());
        cls.setProperty("attachAudioPlayer", JSNativeMethod<Stream, &Stream::attachAudioPlayer>());
        cls.setProperty("detachAudioPlayer", JSNativeMethod<Stream, &Stream::detachAudioPlayer>());
    }

protected:
    Stream(std::shared_ptr<base::Stream> stream) :
        stream_(stream) {}

    std::shared_ptr<base::Stream> stream_;
};

class RemoteStream : public Stream {
public:
    RemoteStream(JSObject obj, JSArray args) : Stream(nullptr) {
        // TODO: not constructible in script, throw an exception
    }

    std::shared_ptr<base::RemoteStream> operator->() const {
        return std::static_pointer_cast<base::RemoteStream>(stream_);
    }

    static JSObject wrap(std::shared_ptr<base::RemoteStream> stream) {
        RemoteStream *binding = new RemoteStream(stream);
        return JSNativeConstructor<RemoteStream>::adopt(binding);
    }

    static void setup(JSObject cls) {
        Stream::setup(cls);
        cls.defineProperty("from", JSNativeGetter<RemoteStream, &RemoteStream::from>());
    }

private:
    RemoteStream(std::shared_ptr<base::RemoteStream> stream) :
        Stream(stream) {}

    JSValue from() {
        return JSString((*this)->From());
    }
};

class LocalCameraStream : public Stream {
public:
    LocalCameraStream(JSObject, JSArray args): Stream(nullptr) {
        JSParameters param(args[0]);
        int error = 0;
        std::unique_ptr<base::AudioFrameGeneratorInterface> audio_generator(YunOSAudioFrameGenerator::Create());
        base::GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
        base::GlobalConfiguration::SetCustomizedAudioInputEnabled(true, std::move(audio_generator));
        stream_ = base::LocalCameraStream::Create(param, error);
    }

    JSValue close(JSObject, JSArray) {
        (*this)->Close();
        return JSUndefined();
    }

    std::shared_ptr<base::LocalCameraStream> operator->() const {
        return std::static_pointer_cast<base::LocalCameraStream>(stream_);
    }

    static void setup(JSObject cls) {
        Stream::setup(cls);
        cls.setProperty("close", JSNativeMethod<LocalCameraStream, &LocalCameraStream::close>());
    }

private:
    class JSParameters : JSObject {
    public:
        JSParameters(const JSValue& jsval): JSObject(jsval){}
        operator base::LocalCameraStreamParameters() const {
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
    };
};

class LocalCustomStream: public Stream {
public:
    LocalCustomStream(JSObject, JSArray args) : Stream(nullptr) {
        JSParameters param(args[0]);
        stream_ = std::make_shared<base::LocalCustomizedStream>(param);
    }

private:
    class JSParameters : JSObject {
    public:
        JSParameters(const JSValue& jsval): JSObject(jsval) {}
        operator base::LocalCustomizedStreamParameters() const {
            JSBoolean video(getProperty("videoEnabled"));
            JSBoolean audio(getProperty("audioEnabled"));
            return base::LocalCustomizedStreamParameters(video, audio);
        }
    };
};
