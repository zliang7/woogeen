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

#include <set>
#include <string>
#include <vector>
#include <map>

#include <Log.h>
#include <JsniStub.h>
#include <native.h>
#define LOG_TAG "woogeen"

#include <woogeen/base/deviceutils.h>
#include <woogeen/conference/remotemixedstream.h>
#include <woogeen/conference/conferenceclient.h>
#include <woogeen/base/framegeneratorinterface.h>
#include <woogeen/base/globalconfiguration.h>

#include "fileaudioframegenerator.h"
#include "filevideoframegenerator.h"
#include "yunosaudioframegenerator.h"

using namespace jsni;
using namespace std;
using namespace woogeen;


template<typename T, typename U = T>
T JsValueTo(JSNIEnv *env, const JsValue& value) {
    return T(value);
}
template<>
bool JsValueTo(JSNIEnv *env, const JsValue& value) {
    assert(env && value.IsBoolean(env));
    return value.ToBool(env);
}
template<>
int JsValueTo(JSNIEnv *env, const JsValue& value) {
    assert(env && value.IsNumber(env));
    return value.ToInt(env);
}
template<>
std::string JsValueTo(JSNIEnv *env, const JsValue& value) {
    assert(env && value.IsString(env));
    JsString jsstr = value.AsString(env);
    char *utf8str = jsstr.GetUtf8Chars(env);
    std::string str(utf8str, jsstr.Utf8Length(env));
    JsString::ReleaseUtf8Chars(env, utf8str);
    return str;
}
template<>
std::wstring JsValueTo(JSNIEnv *env, const JsValue& value) {
    assert(env && value.IsString(env));
    JsString jsstr = value.AsString(env);
    uint16_t *utf16str = jsstr.GetChars(env);
    std::wstring str((wchar_t*)utf16str, jsstr.Length(env));
    JsString::ReleaseChars(env, utf16str);
    return str;
}

JsValue JsValueFrom(JSNIEnv *env, const conference::Permission &permission) {
    JsObject obj = JsObject::New(env);
    obj.SetProperty(env, "canPublish", JsBoolean(env, permission.CanPublish()));
    obj.SetProperty(env, "canRecord", JsBoolean(env, permission.CanRecord()));
    obj.SetProperty(env, "canSubscribe", JsBoolean(env, permission.CanSubscribe()));
    return obj;
}
JsValue JsValueFrom(JSNIEnv *env, const conference::User &user) {
    JsObject obj = JsObject::New(env);
    obj.SetProperty(env, "id", JsString(env, user.Id().c_str()));
    obj.SetProperty(env, "name", JsString(env, user.Name().c_str()));
    obj.SetProperty(env, "role", JsString(env, user.Role().c_str()));
    obj.SetProperty(env, "permissions", JsValueFrom(env, user.Permissions()));
    return obj;
}
JsValue JsValueFrom(JSNIEnv *env, const base::Resolution &res) {
    JsObject obj = JsObject::New(env);
    obj.SetProperty(env, "width", JsNumber(env, res.width));
    obj.SetProperty(env, "height", JsNumber(env, res.height));
    return obj;
}
/*
template <template <typename, typename...> class V, typename... Args>
V<Args...> JsValueTo(JSNIEnv *env, const JsValue& value) {
    return V<Args...>();
}
template <template <typename> class V, typename U>
V<U> JsValueTo(JSNIEnv *env, const JsValue& value) {
    return V<U>();
}*/

template <typename T>
std::vector<T> JsValueToArrayOf(JSNIEnv *env, const JsValue& value) {
    assert(env && value.IsArray(env));
    std::vector<T> vector;
    JsObject array = value.AsObject(env);
    int length = array.GetProperty(env, "length").ToInt(env);
    for (int i = 0; i < length; ++i) {
        JsValue val = array.GetProperty(env, std::to_string(i).c_str());
        vector.push_back(JsValueTo<T>(env, val));
    }
    return vector;
}

template<>
base::IceServer JsValueTo(JSNIEnv* env, const JsValue& value) {
    assert(env && value.IsObject(env));

    base::IceServer server;
    JsValue val;
    JsObject obj = value.AsObject(env);
    val = obj.GetProperty(env, "username");
    if (val.IsString(env))
        server.username = JsValueTo<std::string>(env, val);
    val = obj.GetProperty(env, "password");
    if (val.IsString(env))
        server.password = JsValueTo<std::string>(env, val);
    val = obj.GetProperty(env, "urls");
    if (val.IsArray(env)) {
        server.urls = JsValueToArrayOf<std::string>(env, val);
    }
    return server;
}
template<>
base::MediaCodec JsValueTo(JSNIEnv* env, const JsValue& value) {
    assert(env && value.IsObject(env));

    static std::array<std::string, 9> allCodecs = {
        "OPUS", "ISAC", "G722", "PCMU", "PCMA",
        "VP8", "H264", "VP9", "H265"
    };

    base::MediaCodec codec;
    JsObject obj = value.AsObject(env);
    JsValue val;
    val = obj.GetProperty(env, "audioCodec");
    if (val.IsString(env)) {
        std::string name = JsValueTo<std::string>(env, val);
        for (int i = 0; i < 5; ++i) {
            if (name == allCodecs[i])
                codec.audio_codec = base::MediaCodec::AudioCodec(i + 1);
        }
    }
    val = obj.GetProperty(env, "videoCodec");
    if (val.IsString(env)) {
        std::string name = JsValueTo<std::string>(env, val);
        for (size_t i = 5; i < allCodecs.size(); ++i) {
            if (name == allCodecs[i])
                codec.video_codec = base::MediaCodec::VideoCodec(i - 4);
        }
    }
    return codec;
}
template<>
base::ClientConfiguration JsValueTo(JSNIEnv* env, const JsValue& value) {
    assert(env && value.IsObject(env));

    base::ClientConfiguration conf;
    JsObject obj = value.AsObject(env);
    JsValue val;
    val = obj.GetProperty(env, "maxAudioBandwidth");
    if (val.IsNumber(env))
        conf.max_audio_bandwidth = val.ToInt(env);
    val = obj.GetProperty(env, "maxVideoBandwidth");
    if (val.IsNumber(env))
        conf.max_video_bandwidth = val.ToInt(env);
    val = obj.GetProperty(env, "mediaCodec");
    if (val.IsObject(env)) {
        conf.media_codec = JsValueTo<base::MediaCodec>(env, val);
    }
    val = obj.GetProperty(env, "iceServers");
    if (val.IsArray(env)) {
        obj = val.AsObject(env);
        int len = obj.GetProperty(env, "length").ToInt(env);
        for (int i = 0; i < len; ++i) {
            val = obj.GetProperty(env, std::to_string(i).c_str());
            if (val.IsObject(env)) {
                base::IceServer server = JsValueTo<base::IceServer>(env, val);
                conf.ice_servers.push_back(std::move(server));
            }
        }
    } else if (val.IsObject(env)) {
        base::IceServer server = JsValueTo<base::IceServer>(env, val);
        conf.ice_servers.push_back(std::move(server));
    }
    return conf;
}

template<>
base::Resolution JsValueTo(JSNIEnv* env, const JsValue& value) {
    assert(env && value.IsObject(env));

    base::Resolution resolution;
    JsObject obj = value.AsObject(env);
    JsValue val;
    val = obj.GetProperty(env, "width");
    if (val.IsNumber(env))
        resolution.width = val.ToInt(env);
    val = obj.GetProperty(env, "height");
    if (val.IsNumber(env))
        resolution.height = val.ToInt(env);
    return resolution;
}

template<>
conference::SubscribeOptions JsValueTo(JSNIEnv* env, const JsValue& value) {
    assert(env && value.IsObject(env));

    conference::SubscribeOptions options;
    JsObject obj = value.AsObject(env);
    JsValue val;
    val = obj.GetProperty(env, "resolution");
    if (val.IsObject(env)) {
        options.resolution = JsValueTo<base::Resolution>(env, val);
    }
    return options;
}

template<>
base::LocalCameraStreamParameters JsValueTo(JSNIEnv* env, const JsValue& value) {
    assert(env && value.IsObject(env));

    JsObject obj = value.AsObject(env);
    JsValue val;
    bool video, audio;
    val = obj.GetProperty(env, "videoEnabled");
    video = val.IsBoolean(env) ? val.ToBool(env) : true;
    val = obj.GetProperty(env, "audioEnabled");
    audio = val.IsBoolean(env) ? val.ToBool(env) : true;

    base::LocalCameraStreamParameters param(video, audio);
    val = obj.GetProperty(env, "cameraID");
    if (val.IsString(env))
        param.CameraId(JsValueTo<std::string>(env, val));
    val = obj.GetProperty(env, "streamName");
    if (val.IsString(env))
        param.StreamName(JsValueTo<std::string>(env, val));
    val = obj.GetProperty(env, "fps");
    if (val.IsNumber(env))
        param.Fps(val.ToInt(env));
    val = obj.GetProperty(env, "resolution");
    if (val.IsObject(env)) {
        base::Resolution res = JsValueTo<base::Resolution>(env, val);
        if (res.width | res.height)
            param.Resolution(res.width, res.height);
    }
    return param;
}

template<>
base::LocalCustomizedStreamParameters JsValueTo(JSNIEnv* env, const JsValue& value) {
    assert(env && value.IsObject(env));

    JsObject obj = value.AsObject(env);
    JsValue val;
    bool video, audio;
    val = obj.GetProperty(env, "videoEnabled");
    video = val.IsBoolean(env) ? val.ToBool(env) : true;
    val = obj.GetProperty(env, "audioEnabled");
    audio = val.IsBoolean(env) ? val.ToBool(env) : true;
    return base::LocalCustomizedStreamParameters(video, audio);
}


class RemoteStream: public JsniStub {
  JsniClass(RemoteStream) {
      JsniClassReadOnlyAccessor("id", Getter);
      JsniMethod(disableAudio);
      JsniMethod(enableAudio);
      JsniMethod(disableVideo);
      JsniMethod(enableVideo);
      JsniMethod(attachVideoRenderer);
      JsniMethod(detachVideoRenderer);
  }

public:
  static void New(JSNIEnv *env, const jsni::NativeCallbackInfo &arguments) {
    // RemoteStream is not constructible by script
  }

  void id() {
    JsClassGtter(WTF);
    JsReturnValue(This->stream_->Id().c_str());
  }

  void disableAudio() {
    JsMemberFunc(disableAudio);
    This->stream_->DisableAudio();
  }
  void enableAudio() {
    JsMemberFunc(enableAudio);
    This->stream_->EnableAudio();
  }

  void disableVideo() {
    JsMemberFunc(disableVideo);
    This->stream_->DisableVideo();
  }
  void enableVideo() {
    JsMemberFunc(enableVideo);
    This->stream_->EnableVideo();
  }

  void attachVideoRenderer() {
    JsMemberFunc(attachVideoRenderer);
    // TODO
  }
  void detachVideoRenderer() {
    JsMemberFunc(detachVideoRenderer);
    // TODO
  }

  void attachAudioPlayer() {
    JsMemberFunc(attachAudioPlayer);
    // TODO
  }
  void detachAudioPlayer() {
    JsMemberFunc(detachAudioPlayer);
    // TODO
  }

private:
  friend class ConferenceClient;
  friend JsValue JsValueFrom(JSNIEnv *env, std::shared_ptr<base::RemoteStream> stream);

  RemoteStream(std::shared_ptr<base::RemoteStream> stream) :
      stream_(stream) {
  }

  std::shared_ptr<base::RemoteStream> stream_;
};
InitClass(RemoteStream);

JsValue JsValueFrom(JSNIEnv *env, std::shared_ptr<base::RemoteStream> stream) {
    JsObject obj = RemoteStream::globalRefObj.ToLocal(env).AsObject(env);
    RemoteStream *wrapper = new RemoteStream(stream);
    wrapper->Wrap(env, obj);
    return obj;
}

class FileAudioFrameGenerator: public JsniStub {
  JsniClass(FileAudioFrameGenerator) {
  }

public:
  FileAudioFrameGenerator(JSNIEnv* env, const jsni::NativeCallbackInfo& arguments) {
    JsCtor(FileAudioFrameGenerator);
    JsValue value = arguments.Get(env, 0);
    if (value.IsString(env)) {
        std::string filename = JsValueTo<std::string>(env, value);
        auto generator = internal::FileAudioFrameGenerator::Create(filename);
        generator_ = std::shared_ptr<base::AudioFrameGeneratorInterface>(generator);
    }
  }

private:
  std::shared_ptr<base::AudioFrameGeneratorInterface> generator_;
};
InitClass(FileAudioFrameGenerator);

class FileVideoFrameGenerator: public JsniStub {
  JsniClass(FileVideoFrameGenerator) {
  }

public:
  FileVideoFrameGenerator(JSNIEnv* env, const jsni::NativeCallbackInfo& arguments) {
    JsCtor(FileVideoFrameGenerator);
    JsValue value = arguments.Get(env, 0);
    if (value.IsString(env)) {
        std::string filename = JsValueTo<std::string>(env, value);
        auto generator = internal::FileVideoFrameGenerator::Create(filename);
        generator_ = std::shared_ptr<base::VideoFrameGeneratorInterface>(generator);
    }
  }

private:
  std::shared_ptr<base::VideoFrameGeneratorInterface> generator_;
};
InitClass(FileVideoFrameGenerator);

class LocalCameraStream: public JsniStub {
  JsniClass(LocalCameraStream) {
      JsniClassReadOnlyAccessor("id", Getter);
      JsniMethod(disableAudio);
      JsniMethod(enableAudio);
      JsniMethod(disableVideo);
      JsniMethod(enableVideo);
      JsniMethod(attachVideoRenderer);
      JsniMethod(detachVideoRenderer);
      JsniMethod(close);
  }

public:
  LocalCameraStream(JSNIEnv* env, const jsni::NativeCallbackInfo& arguments) {
    JsCtor(LocalCameraStream);
    JsValue value = arguments.Get(env, 0);
    if (value.IsObject(env)) {
        auto param = JsValueTo<base::LocalCameraStreamParameters>(env, value);
        int error = 0;
        std::unique_ptr<base::AudioFrameGeneratorInterface> audio_generator(YunOSAudioFrameGenerator::Create());
        base::GlobalConfiguration::SetEncodedVideoFrameEnabled(true);
        base::GlobalConfiguration::SetCustomizedAudioInputEnabled(true, std::move(audio_generator));
        stream_ = base::LocalCameraStream::Create(param, error);
    }
  }

  void id() {
    JsClassGtter(WTF);
    JsReturnValue(This->stream_->Id().c_str());
  }

  void disableAudio() {
    JsMemberFunc(disableAudio);
    This->stream_->DisableAudio();
  }
  void enableAudio() {
    JsMemberFunc(enableAudio);
    This->stream_->EnableAudio();
  }

  void disableVideo() {
    JsMemberFunc(disableVideo);
    This->stream_->DisableVideo();
  }
  void enableVideo() {
    JsMemberFunc(enableVideo);
    This->stream_->EnableVideo();
  }

  void attachVideoRenderer() {
    JsMemberFunc(attachVideoRenderer);
    // TODO
  }
  void detachVideoRenderer() {
    JsMemberFunc(detachVideoRenderer);
    // TODO
  }

  void attachAudioPlayer() {
    JsMemberFunc(attachAudioPlayer);
    // TODO
  }
  void detachAudioPlayer() {
    JsMemberFunc(detachAudioPlayer);
    // TODO
  }

  void close() {
    JsMemberFunc(close);
    This->stream_->Close();
  }

private:
  friend class ConferenceClient;
  std::shared_ptr<base::LocalCameraStream> stream_;
};
InitClass(LocalCameraStream);


class LocalCustomStream: public JsniStub {
  JsniClass(LocalCustomStream) {
      JsniClassReadOnlyAccessor("id", Getter);
      JsniMethod(disableAudio);
      JsniMethod(enableAudio);
      JsniMethod(disableVideo);
      JsniMethod(enableVideo);
      JsniMethod(attachVideoRenderer);
      JsniMethod(detachVideoRenderer);
  }

public:
  LocalCustomStream(JSNIEnv* env, const jsni::NativeCallbackInfo& arguments) {
    JsCtor(LocalCustomStream);
    base::LocalCustomizedStreamParameters param(true, true);
    JsValue value = arguments.Get(env, 0);
    if (value.IsObject(env)) {
        param = JsValueTo<base::LocalCustomizedStreamParameters>(env, value);
    }

    //stream_ = std::make_shared<base::LocalCustomizedStream>(param, framer);
    stream_ = std::make_shared<base::LocalCustomizedStream>(param);
  }

  void id() {
    JsClassGtter(WTF);
    JsReturnValue(This->stream_->Id().c_str());
  }

  void disableAudio() {
    JsMemberFunc(disableAudio);
    This->stream_->DisableAudio();
  }
  void enableAudio() {
    JsMemberFunc(enableAudio);
    This->stream_->EnableAudio();
  }

  void disableVideo() {
    JsMemberFunc(disableVideo);
    This->stream_->DisableVideo();
  }
  void enableVideo() {
    JsMemberFunc(enableVideo);
    This->stream_->EnableVideo();
  }

  void attachVideoRenderer() {
    JsMemberFunc(attachVideoRenderer);
    // TODO
  }
  void detachVideoRenderer() {
    JsMemberFunc(detachVideoRenderer);
    // TODO
  }

  void attachAudioPlayer() {
    JsMemberFunc(attachAudioPlayer);
    // TODO
  }
  void detachAudioPlayer() {
    JsMemberFunc(detachAudioPlayer);
    // TODO
  }

private:
  friend class ConferenceClient;
  std::shared_ptr<base::LocalCustomizedStream> stream_;
};
InitClass(LocalCustomStream);


class ConferenceClient : public JsniStub, conference::ConferenceClientObserver {
  JsniClass(ConferenceClient) {
    JsniMethod(join);
    JsniMethod(publish);
    JsniMethod(subscribe);
    JsniMethod(unpublish);
    JsniMethod(unsubscribe);
    JsniMethod(send);
    JsniMethod(playAudio);
    JsniMethod(pauseAudio);
    JsniMethod(playVideo);
    JsniMethod(pauseVideo);
    JsniMethod(leave);
    JsniMethod(getRegion);
    JsniMethod(setRegion);
    JsniMethod(addEventListener);
    JsniMethod(removeEventListener);
    JsniMethod(dispatchEvent);
  }

public:
  ConferenceClient(JSNIEnv *env, const jsni::NativeCallbackInfo &arguments)  {
    JsCtor(ConferenceClient);

    env_ = env;
    JsValue value = arguments.Get(env, 0);
    if (value.IsObject(env)) {
        auto conf = JsValueTo<base::ClientConfiguration>(env, value);
        conference::ConferenceClientConfiguration confconf;;
        memcpy(&confconf, &conf, sizeof(conf));
        client_ = std::make_shared<conference::ConferenceClient>(confconf);
        client_->AddObserver(*this);
    }
  }

  void join(const jsni::NativeCallbackInfo &arguments) {
    auto env = env_;
    JsValue value = arguments.Get(env, 0);
    if (!value.IsString(env)) return;
    std::string token = JsValueTo<std::string>(env, value);

    JsFunction resolve, reject;
    value = arguments.Get(env, 1);
    if (value.IsFunction(env))
        resolve = value.AsFunction(env);
    value = arguments.Get(env, 2);
    if (value.IsFunction(env))
        reject = value.AsFunction(env);

    client_->Join(token,
        [&resolve, env](std::shared_ptr<conference::User> user) {
            if (!resolve.IsEmpty(env)) {
                JsValue args = JsValueFrom(env, *user);
                resolve.Call(env, JsValue::Null(env), 1, &args);
            }
        },
        [&reject, env](std::unique_ptr<conference::ConferenceException> err) {
            if (!reject.IsEmpty(env)) {
                //JsValue args = JsString(env, err->Message().c_str()); FIXME!
                JsValue args = JsString(env, "error message");
                reject.Call(env, JsValue::Null(env), 1, &args);
            }
        });

    ///////////////////////////////////////////////////////
    JsMemberFunc(join);
    This->join(arguments);
  }

  void publish(const jsni::NativeCallbackInfo &arguments) {
    auto env = env_;
    JsValue value = arguments.Get(env, 0);
    if (!value.IsObject(env)) return;
    JsObject obj = value.AsObject(env);
    if (!obj.HasInternalField(env)) return;

    JsFunction resolve, reject;
    value = arguments.Get(env, 1);
    if (value.IsFunction(env))
        resolve = value.AsFunction(env);
    value = arguments.Get(env, 2);
    if (value.IsFunction(env))
        reject = value.AsFunction(env);

    // FIXME: how to valid the type of pointer?
    auto stream = reinterpret_cast<LocalCameraStream*>(obj.GetInternalField(env));
    client_->Publish(stream->stream_,
        [&resolve, env]() {
            if (!resolve.IsEmpty(env))
                resolve.Call(env, JsValue::Null(env), 0, nullptr);
        },
        [&reject, env](std::unique_ptr<conference::ConferenceException> err) {
            if (!reject.IsEmpty(env))
                reject.Call(env, JsValue::Null(env), 0, nullptr);
        });

    ///////////////////////////////////////////////////////
    JsMemberFunc(publish);
    This->publish(arguments);
  }

  void subscribe(const jsni::NativeCallbackInfo &arguments) {
    auto env = env_;
    JsValue value = arguments.Get(env, 0);
    if (!value.IsObject(env)) return;
    JsObject obj = value.AsObject(env);
    if (!obj.HasInternalField(env)) return;

    conference::SubscribeOptions options;
    value = arguments.Get(env, 1);
    if (value.IsObject(env))
        options = JsValueTo<conference::SubscribeOptions>(env, value);

    JsFunction resolve, reject;
    value = arguments.Get(env, 2);
    if (value.IsFunction(env))
        resolve = value.AsFunction(env);
    value = arguments.Get(env, 3);
    if (value.IsFunction(env))
        reject = value.AsFunction(env);

    // FIXME: how to valid the type of pointer?
    auto stream = reinterpret_cast<RemoteStream*>(obj.GetInternalField(env));
    client_->Subscribe(stream->stream_,
        [&resolve, env](std::shared_ptr<base::RemoteStream> stream) {
            if (!resolve.IsEmpty(env)) {
                // FIXME!!
                JsValue args = JsValueFrom(env, stream);
                resolve.Call(env, JsValue::Null(env), 1, &args);
            }
        },
        [&reject, env](std::unique_ptr<conference::ConferenceException> err) {
            if (!reject.IsEmpty(env))
                reject.Call(env, JsValue::Null(env), 0, nullptr);
        });

    ///////////////////////////////////////////////////////
    JsMemberFunc(subscribe);
    This->subscribe(arguments);
  }

  void unpublish(const jsni::NativeCallbackInfo &arguments) {
    auto env = env_;
    JsValue value = arguments.Get(env, 0);
    if (!value.IsObject(env)) return;
    JsObject obj = value.AsObject(env);
    if (!obj.HasInternalField(env)) return;

    JsFunction resolve, reject;
    value = arguments.Get(env, 1);
    if (value.IsFunction(env))
        resolve = value.AsFunction(env);
    value = arguments.Get(env, 2);
    if (value.IsFunction(env))
        reject = value.AsFunction(env);

    // FIXME: how to valid the type of pointer?
    auto stream = reinterpret_cast<LocalCameraStream*>(obj.GetInternalField(env));
    client_->Unpublish(stream->stream_,
        [&resolve, env]() {
            if (!resolve.IsEmpty(env))
                resolve.Call(env, JsValue::Null(env), 0, nullptr);
        },
        [&reject, env](std::unique_ptr<conference::ConferenceException> err) {
            if (!reject.IsEmpty(env))
                reject.Call(env, JsValue::Null(env), 0, nullptr);
        });

    ///////////////////////////////////////////////////////
    JsMemberFunc(unpublish);
    This->unpublish(arguments);
  }

  void unsubscribe(const jsni::NativeCallbackInfo &arguments) {
    auto env = env_;
    JsValue value = arguments.Get(env, 0);
    if (!value.IsObject(env)) return;
    JsObject obj = value.AsObject(env);
    if (!obj.HasInternalField(env)) return;

    JsFunction resolve, reject;
    value = arguments.Get(env, 1);
    if (value.IsFunction(env))
        resolve = value.AsFunction(env);
    value = arguments.Get(env, 2);
    if (value.IsFunction(env))
        reject = value.AsFunction(env);

    // FIXME: how to valid the type of pointer?
    auto stream = reinterpret_cast<RemoteStream*>(obj.GetInternalField(env));
    client_->Unsubscribe(stream->stream_,
        [&resolve, env]() {
            if (!resolve.IsEmpty(env))
                resolve.Call(env, JsValue::Null(env), 0, nullptr);
        },
        [&reject, env](std::unique_ptr<conference::ConferenceException> err) {
            if (!reject.IsEmpty(env))
                reject.Call(env, JsValue::Null(env), 0, nullptr);
        });

    ///////////////////////////////////////////////////////
    JsMemberFunc(unsubscribe);
    This->unsubscribe(arguments);
  }

  void send(const jsni::NativeCallbackInfo &arguments) {
    auto env = env_;
    JsValue value = arguments.Get(env, 0);
    if (!value.IsString(env)) return;
    std::string message = JsValueTo<std::string>(env, value);
    value = arguments.Get(env, 1);
    std::string receiver;
    if (value.IsString(env))
        receiver = JsValueTo<std::string>(env, value);

    JsFunction resolve, reject;
    value = arguments.Get(env, 2);
    if (value.IsFunction(env))
        resolve = value.AsFunction(env);
    value = arguments.Get(env, 3);
    if (value.IsFunction(env))
        reject = value.AsFunction(env);

    client_->Send(message, receiver,
        [&resolve, env](void) {
            if (!resolve.IsEmpty(env)) {
                resolve.Call(env, JsValue::Null(env), 0, nullptr);
            }
        },
        [&reject, env](std::unique_ptr<conference::ConferenceException> err) {
            if (!reject.IsEmpty(env))
                reject.Call(env, JsValue::Null(env), 0, nullptr);
        });

    ///////////////////////////////////////////////////////
    JsMemberFunc(send);
    This->send(arguments);
  }

  void playAudio(const jsni::NativeCallbackInfo &arguments) {
    JsMemberFunc(playAudio);
    This->playAudio(arguments);
  }
  void pauseAudio(const jsni::NativeCallbackInfo &arguments) {
    JsMemberFunc(pauseAudio);
    This->pauseAudio(arguments);
  }
  void playVideo(const jsni::NativeCallbackInfo &arguments) {
    JsMemberFunc(playVideo);
    This->playVideo(arguments);
  }
  void pauseVideo(const jsni::NativeCallbackInfo &arguments) {
    JsMemberFunc(pauseVideo);
    This->pauseVideo(arguments);
  }

  void leave(const jsni::NativeCallbackInfo &arguments) {
    auto env = env_;
    JsFunction resolve, reject;
    JsValue value = arguments.Get(env, 0);
    if (value.IsFunction(env))
        resolve = value.AsFunction(env);
    value = arguments.Get(env, 1);
    if (value.IsFunction(env))
        reject = value.AsFunction(env);

    client_->Leave(
        [&resolve, env](void) {
            if (!resolve.IsEmpty(env)) {
                resolve.Call(env, JsValue::Null(env), 0, nullptr);
            }
        },
        [&reject, env](std::unique_ptr<conference::ConferenceException> err) {
            if (!reject.IsEmpty(env))
                reject.Call(env, JsValue::Null(env), 0, nullptr);
        });

    ///////////////////////////////////////////////////////
    JsMemberFunc(leave);
    This->leave(arguments);
  }

  void getRegion(const jsni::NativeCallbackInfo &arguments) {
    auto env = env_;
    JsValue value = arguments.Get(env, 0);
    if (!value.IsObject(env)) return;
    JsObject obj = value.AsObject(env);
    if (!obj.HasInternalField(env)) return;

    JsFunction resolve, reject;
    value = arguments.Get(env, 1);
    if (value.IsFunction(env))
        resolve = value.AsFunction(env);
    value = arguments.Get(env, 2);
    if (value.IsFunction(env))
        reject = value.AsFunction(env);

    auto stream = reinterpret_cast<RemoteStream*>(obj.GetInternalField(env));
    client_->GetRegion(stream->stream_,
        [&resolve, env](std::string id) {
            if (!resolve.IsEmpty(env)) {
                JsValue args = JsString(env, id.c_str());
                resolve.Call(env, JsValue::Null(env), 1, &args);
            }
        },
        [&reject, env](std::unique_ptr<conference::ConferenceException> err) {
            if (!reject.IsEmpty(env))
                reject.Call(env, JsValue::Null(env), 0, nullptr);
        });

   ///////////////////////////////////////////////////////
    JsMemberFunc(getRegion);
    This->getRegion(arguments);
  }
  void setRegion(const jsni::NativeCallbackInfo &arguments) {
    auto env = env_;
    JsValue value = arguments.Get(env, 0);
    if (!value.IsObject(env)) return;
    JsObject obj = value.AsObject(env);
    if (!obj.HasInternalField(env)) return;

    std::string id;
    value = arguments.Get(env, 1);
    if (!value.IsString(env)) return;
    id = JsValueTo<std::string>(env, value);

    JsFunction resolve, reject;
    value = arguments.Get(env, 2);
    if (value.IsFunction(env))
        resolve = value.AsFunction(env);
    value = arguments.Get(env, 3);
    if (value.IsFunction(env))
        reject = value.AsFunction(env);

    auto stream = reinterpret_cast<RemoteStream*>(obj.GetInternalField(env));
    client_->SetRegion(stream->stream_, id,
        [&resolve, env](void) {
            if (!resolve.IsEmpty(env))
                resolve.Call(env, JsValue::Null(env), 0, nullptr);
        },
        [&reject, env](std::unique_ptr<conference::ConferenceException> err) {
            if (!reject.IsEmpty(env))
                reject.Call(env, JsValue::Null(env), 0, nullptr);
        });

   ///////////////////////////////////////////////////////
    JsMemberFunc(setRegion);
    This->setRegion(arguments);
  }

  void addEventListener(const jsni::NativeCallbackInfo &arguments) {
    static std::set<std::string> event_types = {
        "user-joined", "user-left", "stream-added", "stream-removed",
        "message-received", "server-disconnected"
    };

    JsValue value = arguments.Get(env_, 0);
    if (!value.IsString(env_)) return;
    std::string type = JsValueTo<std::string>(env_, value);
    if (event_types.find(type) == event_types.end()) return;
    value = arguments.Get(env_, 1);
    if (!value.IsFunction(env_)) return;
    JsFunction listener = value.AsFunction(env_);

    if (findListener(type, &listener) == listeners_.end()) {
        auto global = JsGlobalRef();
        global.Set(env_, listener);
        listeners_.insert(std::make_pair(type, std::move(global)));
    }

    ///////////////////////////////////////////////////////
    JsMemberFunc(addEventListener);
    This->addEventListener(arguments);
  }

  void removeEventListener(const jsni::NativeCallbackInfo &arguments) {
    JsValue value = arguments.Get(env_, 0);
    if (!value.IsString(env_)) return;
    std::string type = JsValueTo<std::string>(env_, value);
    value = arguments.Get(env_, 1);
    if (!value.IsFunction(env_)) return;
    JsFunction listener = value.AsFunction(env_);

    auto it = findListener(type, &listener);
    if (it != listeners_.end()) {
        it->second.Clear(env_);
        listeners_.erase(it);
    }

    ///////////////////////////////////////////////////////
    JsMemberFunc(removeEventListener);
    This->removeEventListener(arguments);
  }

  void dispatchEvent(std::string type, int argc, JsValue* argv) {
    YUNOS_LOG(Info, LOG_TAG, "%s event received", type.c_str());
    auto it = findListener(type);
    if (it == listeners_.end()) return;
    while (it != listeners_.end()) {
        JsFunction listener = it->second.ToLocal(env_).AsFunction(env_);
        listener.Call(env_, JsValue::Null(env_), argc, argv);
        ++it;
    }
    YUNOS_LOG(Info, LOG_TAG, "%s event dispatched", type.c_str());

    ///////////////////////////////////////////////////////
    // Another function which can be called from script (for testing purpose).
    JsMemberFunc(dispatchEvent);

    JsValue value =  arguments.Get(env, 0);
    if (!value.IsString(env)) return;
    std::string type = JsValueTo<std::string>(env, value);

    int argc = arguments.Length(env) - 1;
    JsValue args[argc];
    for (int i = 0; i < argc; ++i) {
        args[i] = arguments.Get(env, i + 1);
    }
    This->dispatchEvent(type, argc, args);
    JsReturnValue(true);
  }

private:

  void OnStreamAdded(std::shared_ptr<conference::RemoteCameraStream> stream) override {
      JsValue args = JsValueFrom(env_, stream);
      dispatchEvent("stream-added", 1, &args);
  }
  void OnStreamAdded(std::shared_ptr<conference::RemoteScreenStream> stream) override {
      JsValue args = JsValueFrom(env_, stream);
      dispatchEvent("stream-added", 1, &args);
  }
  void OnStreamAdded(std::shared_ptr<conference::RemoteMixedStream> stream) override {
      JsValue args = JsValueFrom(env_, stream);
      dispatchEvent("stream-added", 1, &args);
  }
  void OnStreamRemoved(std::shared_ptr<conference::RemoteCameraStream> stream) override {
      JsValue args = JsValueFrom(env_, stream);
      dispatchEvent("stream-removed", 1, &args);
  }
  void OnStreamRemoved(std::shared_ptr<conference::RemoteScreenStream> stream) override {
      JsValue args = JsValueFrom(env_, stream);
      dispatchEvent("stream-removed", 1, &args);
  }
  void OnStreamRemoved(std::shared_ptr<conference::RemoteMixedStream> stream) override {
      JsValue args = JsValueFrom(env_, stream);
      dispatchEvent("stream-removed", 1, &args);
  }
  void OnUserJoined(std::shared_ptr<const conference::User> user) override {
    JsValue args = JsValueFrom(env_, *user);
    dispatchEvent("user-joined", 1, &args);
  }
  void OnUserLeft(std::shared_ptr<const conference::User> user) override {
    JsValue args = JsValueFrom(env_, *user);
    dispatchEvent("user-left", 1, &args);
  }
  void OnMessageReceived(std::string& sender_id, std::string& message) override {
    JsValue args[2];
    args[0] = JsString(env_, sender_id.c_str());
    args[1] = JsString(env_, message.c_str());
    dispatchEvent("message-received", 2, args);
  }
  void OnServerDisconnected() override {
    dispatchEvent("server-disconnected", 0, nullptr);
  }

  typedef std::multimap<std::string, JsGlobalRef> ListenerBag;
  ListenerBag::iterator findListener(std::string type, JsFunction* listener = nullptr) {
      auto it = listeners_.find(type);
      if (listener) {
          while (it != listeners_.end()) {
              if (it->second.ToLocal(env_).Get() == listener->Get())
                  return it;
              ++it;
          }
      }
      return it;
  }
  JSNIEnv *env_;
  ListenerBag listeners_;
  std::shared_ptr<conference::ConferenceClient> client_;
};
InitClass(ConferenceClient);


void getVideoCaptureDevices(jsni::JSNIEnv *env, const jsni::NativeCallbackInfo &arguments) {
    JsObject result = JsObject::New(env);
    std::vector<std::string> devices = base::DeviceUtils::VideoCapturerIds();
    for (auto dev: devices) {
        JsObject devobj = JsObject::New(env);
        auto resolutions = base::DeviceUtils::VideoCapturerSupportedResolutions(dev);
        int i = 0;
        for (auto res: resolutions) {
            auto name = std::to_string(i++).c_str();
            devobj.SetProperty(env, name, JsValueFrom(env, res));
        }
        result.SetProperty(env, dev.c_str(), devobj);
    }
    JsReturnValue(result);
}


void setCustomAudioInput(JSNIEnv *env, const jsni::NativeCallbackInfo &arguments) {
    JsValue value = arguments.Get(env, 0);
    if (!value.IsObject(env)) return;
    JsObject obj = value.AsObject(env);
    if (!obj.HasInternalField(env)) return;

    auto ptr = reinterpret_cast<base::AudioFrameGeneratorInterface*>(
            obj.GetInternalField(env));
    auto generator = std::unique_ptr<base::AudioFrameGeneratorInterface>(ptr);
    obj.SetInternalField(env, nullptr);
    base::GlobalConfiguration::SetCustomizedAudioInputEnabled(true, std::move(generator));
}

void setVideoDecoder(JSNIEnv *env, const jsni::NativeCallbackInfo &arguments) {
    // TODO
}
void setCustomVideoEncoder(JSNIEnv *env, const jsni::NativeCallbackInfo &arguments) {
    // TODO
}

JSNI_INIT {
    JsRegisterClass(ConferenceClient);
    JsRegisterClass(LocalCameraStream);
    JsRegisterClass(LocalCustomStream);
    JsRegisterClass(RemoteStream);
    JsRegisterClass(FileVideoFrameGenerator);
    JsRegisterClass(FileAudioFrameGenerator);
    JsRegisterMethod(getVideoCaptureDevices);
    JsRegisterMethod(setCustomAudioInput);
    return 1;
}
