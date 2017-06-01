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

#include <algorithm>
#include <unordered_set>

#include <log/Log.h>
#define LOG_TAG "woogeen"

#include "jsconfclient.h"
#include "jsstream.h"

using namespace woogeen;
using namespace jsnipp;


class JSSubscribeOptions : JSObject {
public:
    JSSubscribeOptions(const JSValue& jsval): JSObject(jsval) {}
    operator conference::SubscribeOptions() const {
        conference::SubscribeOptions opt;
        opt.resolution = JSResolution(getProperty("resolution"));
        return opt;
    }
};

class JSUser : public JSObject {
public:
    JSUser(const conference::User& user);
};

class JSClientConfiguration : JSObject {
public:
    JSClientConfiguration(const JSValue& jsval): JSObject(jsval) {}
    operator base::ClientConfiguration() const;

private:
    class JSIceServer : JSObject {
    public:
        JSIceServer(const JSValue& jsval): JSObject(jsval){}
        operator base::IceServer() const;
    };
    class JSMediaCodec : JSObject {
    public:
        JSMediaCodec(const JSValue& jsval): JSObject(jsval){}
        operator base::MediaCodec() const;
    };
};


typedef std::shared_ptr<conference::ConferenceException> Exception;
class JSErrorCallback : public JSCallback<void, Exception> {
    using JSCallback::JSCallback;
    JSValue jsnify(Exception& err) const override {
        return JSString(err->Message());
    }
public:
    void operator()(conference::ConferenceException* error) {
        auto err = std::make_shared<conference::ConferenceException>(
                conference::ConferenceException::kUnkown, error->Message());
        JSCallback<void, Exception>::operator()(err);
    }
};
class JSUserCallback : public JSCallback<void, std::shared_ptr<conference::User>> {
    using JSCallback::JSCallback;
    JSValue jsnify(std::shared_ptr<conference::User>& user) const override {
        return JSUser(*user);
    }
};
class JSStreamCallback : public JSCallback<void, std::shared_ptr<base::RemoteStream>> {
    using JSCallback::JSCallback;
    JSValue jsnify(std::shared_ptr<base::RemoteStream>& stream) const override {
        return RemoteStream::wrap(stream);
    }
};
class JSRegionCallback : public JSCallback<void, std::string> {
    using JSCallback::JSCallback;
    JSValue jsnify(std::string& id) const override {
        return JSString(id);
    }
};
class JSMessageCallback : public JSCallback<void, std::string, std::string> {
    using JSCallback::JSCallback;
    JSValue jsnify(std::string& id, std::string& message) const override {
        return JSArray(2, id, message);
    }
};


ConferenceClient::ConferenceClient(JSObject, JSArray args) {
    static_assert(sizeof(base::ClientConfiguration) ==
                  sizeof(conference::ConferenceClientConfiguration), "");
    base::ClientConfiguration conf = JSClientConfiguration(args[0]);
    auto confconf = static_cast<conference::ConferenceClientConfiguration&>(conf);
    client_ = std::make_shared<conference::ConferenceClient>(confconf);
    client_->AddObserver(*this);
}

JSValue ConferenceClient::join(JSObject, JSArray args) {
    JSString token(args[0]);
    JSUserCallback* resolve = new JSUserCallback(args[1]);
    JSErrorCallback* reject = new JSErrorCallback(args[2]);

    client_->Join(token,
        [=](std::shared_ptr<conference::User> user) {
            (*resolve)(user);
            delete reject;
        },
        [=](std::unique_ptr<conference::ConferenceException> err) {
            (*reject)(err.get());
            delete resolve;
        });
    return JSUndefined();
}

JSValue ConferenceClient::publish(JSObject, JSArray args) {
    JSNativeObject<LocalCustomStream> jsobj(args[0]);
    JSCallback<void>* resolve = new JSCallback<void>(args[1]);
    JSErrorCallback* reject = new JSErrorCallback(args[2]);

    client_->Publish(jsobj->operator->(),
        [=]() {
            (*resolve)();
            delete reject;
        },
        [=](std::unique_ptr<conference::ConferenceException> err) {
            (*reject)(err.get());
            delete resolve;
    });
    return JSUndefined();
}

JSValue ConferenceClient::subscribe(JSObject, JSArray args) {
    JSNativeObject<RemoteStream> jsobj(args[0]);
    JSSubscribeOptions options(args[1]);
    JSStreamCallback* resolve = new JSStreamCallback(args[2]);
    JSErrorCallback* reject = new JSErrorCallback(args[3]);

    client_->Subscribe(jsobj->operator->(),
        [=](std::shared_ptr<base::RemoteStream> stream) {
            (*resolve)(stream);
            delete reject;
        },
        [=](std::unique_ptr<conference::ConferenceException> err) {
            (*reject)(err.get());
            delete resolve;
    });
    return JSUndefined();
}

JSValue ConferenceClient::unpublish(JSObject, JSArray args) {
    JSNativeObject<LocalCustomStream> jsobj(args[0]);
    JSCallback<void>* resolve = new JSCallback<void>(args[1]);
    JSErrorCallback* reject = new JSErrorCallback(args[2]);

    client_->Unpublish(jsobj->operator->(),
        [=]() {
            (*resolve)();
            delete reject;
        },
        [=](std::unique_ptr<conference::ConferenceException> err) {
            (*reject)(err.get());
            delete resolve;
        });
    return JSUndefined();
}

JSValue ConferenceClient::unsubscribe(JSObject, JSArray args) {
    JSNativeObject<RemoteStream> jsobj(args[0]);
    JSCallback<void>* resolve = new JSCallback<void>(args[1]);
    JSErrorCallback* reject = new JSErrorCallback(args[2]);

    client_->Unsubscribe(jsobj->operator->(),
        [=]() {
            (*resolve)();
            delete reject;
        },
        [=](std::unique_ptr<conference::ConferenceException> err) {
            (*reject)(err.get());
            delete resolve;
        });
    return JSUndefined();
}

JSValue ConferenceClient::send(JSObject, JSArray args) {
    JSString message(args[0]), receiver(args[1]);
    JSCallback<void>* resolve = new JSCallback<void>(args[2]);
    JSErrorCallback* reject = new JSErrorCallback(args[3]);

    client_->Send(message, receiver,
        [=](void) {
            (*resolve)();
            delete reject;
        },
        [=](std::unique_ptr<conference::ConferenceException> err) {
            (*reject)(err.get());
            delete resolve;
        });
    return JSUndefined();
}

// TODO
JSValue ConferenceClient::playAudio(JSObject, JSArray args) {
    return JSUndefined();
}
JSValue ConferenceClient::pauseAudio(JSObject, JSArray args) {
    return JSUndefined();
}
JSValue ConferenceClient::playVideo(JSObject, JSArray args) {
    return JSUndefined();
}
JSValue ConferenceClient::pauseVideo(JSObject, JSArray args) {
    return JSUndefined();
}

JSValue ConferenceClient::leave(JSObject, JSArray args) {
    JSCallback<void>* resolve = new JSCallback<void>(args[0]);
    JSErrorCallback* reject = new JSErrorCallback(args[1]);

    client_->Leave(
        [=](void) {
            (*resolve)();
            delete reject;
        },
        [=](std::unique_ptr<conference::ConferenceException> err) {
            (*reject)(err.get());
            delete resolve;
        });
    return JSUndefined();
}

JSValue ConferenceClient::getRegion(JSObject, JSArray args) {
    JSNativeObject<RemoteStream> jsobj(args[0]);
    JSRegionCallback* resolve = new JSRegionCallback(args[1]);
    JSErrorCallback* reject = new JSErrorCallback(args[2]);

    client_->GetRegion(jsobj->operator->(),
        [=](std::string id) {
            (*resolve)(id);
            delete reject;
        },
        [=](std::unique_ptr<conference::ConferenceException> err) {
            (*reject)(err.get());
            delete resolve;
        });
    return JSUndefined();
}

JSValue ConferenceClient::setRegion(JSObject, JSArray args) {
    JSNativeObject<RemoteStream> jsobj(args[0]);
    JSString id(args[1]);
    JSCallback<void>* resolve = new JSCallback<void>(args[2]);
    JSErrorCallback* reject = new JSErrorCallback(args[3]);

    client_->SetRegion(jsobj->operator->(), id,
        [=](void) {
            (*resolve)();
            delete reject;
        },
        [=](std::unique_ptr<conference::ConferenceException> err) {
            (*reject)(err.get());
            delete resolve;
        });
    return JSUndefined();
}

void ConferenceClient::setup(JSObject cls) {
    cls.setProperty("join", JSNativeMethod<ConferenceClient, &ConferenceClient::join>());
    cls.setProperty("publish", JSNativeMethod<ConferenceClient, &ConferenceClient::publish>());
    cls.setProperty("subscribe", JSNativeMethod<ConferenceClient, &ConferenceClient::subscribe>());
    cls.setProperty("unpublish", JSNativeMethod<ConferenceClient, &ConferenceClient::unpublish>());
    cls.setProperty("unsubscribe", JSNativeMethod<ConferenceClient, &ConferenceClient::unsubscribe>());
    cls.setProperty("send", JSNativeMethod<ConferenceClient, &ConferenceClient::send>());
    cls.setProperty("playAudio", JSNativeMethod<ConferenceClient, &ConferenceClient::playAudio>());
    cls.setProperty("pauseAudio", JSNativeMethod<ConferenceClient, &ConferenceClient::pauseAudio>());
    cls.setProperty("playVideo", JSNativeMethod<ConferenceClient, &ConferenceClient::playVideo>());
    cls.setProperty("pauseVideo", JSNativeMethod<ConferenceClient, &ConferenceClient::pauseVideo>());
    cls.setProperty("leave", JSNativeMethod<ConferenceClient, &ConferenceClient::leave>());
    cls.setProperty("getRegion", JSNativeMethod<ConferenceClient, &ConferenceClient::getRegion>());
    cls.setProperty("setRegion", JSNativeMethod<ConferenceClient, &ConferenceClient::setRegion>());
    cls.setProperty("addEventListener", JSNativeMethod<ConferenceClient, &ConferenceClient::addEventListener>());
    cls.setProperty("removeEventListener", JSNativeMethod<ConferenceClient, &ConferenceClient::removeEventListener>());
    cls.setProperty("dispatchEvent", JSNativeMethod<ConferenceClient, &ConferenceClient::dispatchEvent>());
}


JSValue ConferenceClient::addEventListener(JSObject, JSArray args) {
    static std::unordered_set<std::string> event_types = {
        "user-joined", "user-left", "stream-added", "stream-removed",
        "message-received", "server-disconnected"
    };

    std::string type = JSString(args[0]);
    if (event_types.find(type) == event_types.end())
        return false_js;

    JSFunction jsfunc(args[1]);
    /* TODO: check for duplication
    for (auto it = listeners_.find(type); it != listeners_.end(); ++it) {
        if (JSFunction(*it->second) == jsfunc) {
            return false_js;
        }
    }*/

    JSCallbackBase *callback;
    if (type.find("user-") == 0) {
        callback = new JSUserCallback(jsfunc, false);
    } else if (type.find("stream-") == 0){
        callback = new JSStreamCallback(jsfunc, false);
    } else if (type.find("message-") == 0){
        callback = new JSMessageCallback(jsfunc, false);
    } else {
        callback = new JSCallback<void>(jsfunc, false);
    }
    listeners_.emplace(type, std::unique_ptr<JSCallbackBase>(callback));
    return true_js;
}

JSValue ConferenceClient::removeEventListener(JSObject, JSArray args) {
    std::string type = JSString(args[0]);
    JSFunction jsfunc(args[1]);
    for (auto it = listeners_.find(type); it != listeners_.end(); ++it) {
        if (JSFunction(*it->second) == jsfunc) {
            listeners_.erase(it);
            return true_js;
        }
    }
    return false_js;
}

JSValue ConferenceClient::dispatchEvent(JSObject, JSArray args) {
    // This can be called from script for testing purpose.
    std::string type = JSString(args[0]);
    JSFunction shift(args.getProperty("shift"));
    shift.call(args);

    for (auto it = listeners_.find(type); it != listeners_.end(); ++it) {
        JSFunction jsfunc(*it->second);
        jsfunc.apply(nullptr, args);
    }
    return JSBoolean(true);
}

template <class C, typename... Ts>
void ConferenceClient::dispatchEvent(const std::string& type, Ts... args) {
    for (auto it = listeners_.find(type); it != listeners_.end(); ++it) {
        auto listener = static_cast<C*>(it->second.get());
        (*listener)(args...);
    }
    LOG_I("event \"%s\" dispatched", type.c_str());
}

void ConferenceClient::OnUserJoined(std::shared_ptr<const conference::User> user) {
    LOG_I("event \"user-joined\" received: id=%s, name=%s, role=%s",
          user->Id().c_str(), user->Name().c_str(), user->Role().c_str());
    auto arg = std::const_pointer_cast<conference::User>(user);
    dispatchEvent<JSUserCallback>("user-joined", arg);
}
void ConferenceClient::OnUserLeft(std::shared_ptr<const conference::User> user) {
    LOG_I("event \"user-left\" received: id=%s, name=%s, role=%s",
          user->Id().c_str(), user->Name().c_str(), user->Role().c_str());
    auto arg = std::const_pointer_cast<conference::User>(user);
    dispatchEvent<JSUserCallback>("user-left", arg);
}
void ConferenceClient::OnStreamAdded(std::shared_ptr<conference::RemoteCameraStream> stream) {
    auto arg = std::static_pointer_cast<base::RemoteStream>(stream);
    dispatchEvent<JSStreamCallback>("stream-added", arg);
}
void ConferenceClient::OnStreamAdded(std::shared_ptr<conference::RemoteScreenStream> stream) {
    auto arg = std::static_pointer_cast<base::RemoteStream>(stream);
    dispatchEvent<JSStreamCallback>("stream-added", arg);
}
void ConferenceClient::OnStreamAdded(std::shared_ptr<conference::RemoteMixedStream> stream) {
    auto arg = std::static_pointer_cast<base::RemoteStream>(stream);
    dispatchEvent<JSStreamCallback>("stream-added", arg);
}
void ConferenceClient::OnStreamRemoved(std::shared_ptr<conference::RemoteCameraStream> stream) {
    auto arg = std::static_pointer_cast<base::RemoteStream>(stream);
    dispatchEvent<JSStreamCallback>("stream-removed", arg);
}
void ConferenceClient::OnMessageReceived(std::string& id, std::string& message) {
    dispatchEvent<JSMessageCallback>("message-received", id, message);
}
void ConferenceClient::OnStreamRemoved(std::shared_ptr<conference::RemoteScreenStream> stream) {
    auto arg = std::static_pointer_cast<base::RemoteStream>(stream);
    dispatchEvent<JSStreamCallback>("stream-removed", arg);
}
void ConferenceClient::OnStreamRemoved(std::shared_ptr<conference::RemoteMixedStream> stream) {
    auto arg = std::static_pointer_cast<base::RemoteStream>(stream);
    dispatchEvent<JSStreamCallback>("stream-removed", arg);
}
void ConferenceClient::OnServerDisconnected() {
    dispatchEvent<JSCallback<void>>("server-disconnected");
}

///////////////////////////////////////////////////////////////////////////////

JSClientConfiguration::operator base::ClientConfiguration() const {
    base::ClientConfiguration conf;
    conf.max_audio_bandwidth = JSNumber(getProperty("maxAudioBandwidth"));
    conf.max_video_bandwidth = JSNumber(getProperty("maxVideoBandwidth"));
    conf.media_codec = JSMediaCodec(getProperty("mediaCodec"));
    JSArray servers = JSArray(getProperty("iceServers"));
    for (size_t i = 0; i < servers.length(); ++i) {
        base::IceServer server(JSIceServer(servers[i]));
        conf.ice_servers.push_back(server);
    }
    return conf;
}

JSClientConfiguration::JSIceServer::operator base::IceServer() const {
    base::IceServer server;
    server.username = JSString(getProperty("username"));
    server.password = JSString(getProperty("password"));
    JSArray urls(getProperty("urls"));
    for (size_t i = 0; i < urls.length(); ++i) {
        std::string url(JSString(urls[i]));
        server.urls.push_back(std::move(url));
    }
    return server;
}

JSClientConfiguration::JSMediaCodec::operator base::MediaCodec() const {
    base::MediaCodec codec;
    static const char* audioCodecs[] = {
        "OPUS", "ISAC", "G722", "PCMU", "PCMA", NULL };
    std::string audio = JSString(getProperty("audioCodec"));
    for (int i = 0; audioCodecs[i]; ++i) {
        if (audio == audioCodecs[i]) {
            codec.audio_codec = base::MediaCodec::AudioCodec(i + 1);
            break;
        }
    }

    static const char* videoCodecs[] = {
        "VP8", "H264", "VP9", "H265", NULL };
    std::string video = JSString(getProperty("videoCodec"));
    for (int i = 0; videoCodecs[i]; ++i) {
        if (video == videoCodecs[i]) {
            codec.video_codec = base::MediaCodec::VideoCodec(i + 1);
            break;
        }
    }
    return codec;
}

JSUser::JSUser(const conference::User& user): JSObject({
    { "id",          JSString(user.Id()) },
    { "name",        JSString(user.Name()) },
    { "role",        JSString(user.Role()) },
    { "permissions", JSObject({
        { "canPublish",   JSBoolean(user.Permissions().CanPublish()) },
        { "canRecord",    JSBoolean(user.Permissions().CanRecord()) },
        { "canSubscribe", JSBoolean(user.Permissions().CanSubscribe()) }
    })}
}){}
