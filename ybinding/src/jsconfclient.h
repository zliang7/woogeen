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

#include <set>
#include <string>
#include <map>

#include <Log.h>
#define LOG_TAG "woogeen"

#include <woogeen/conference/remotemixedstream.h>
#include <woogeen/conference/conferenceclient.h>

#include <jsni.h>
#include <jsnipp.h>

#include "jsstream.h"

using namespace woogeen;
using namespace jsnipp;

class JSClientConfiguration : JSObject {
public:
    JSClientConfiguration(const JSValue& jsval): JSObject(jsval) {}
    operator base::ClientConfiguration() const {
        base::ClientConfiguration conf;
        conf.max_audio_bandwidth = JSNumber(getProperty("maxAudioBandwidth"));
        conf.max_video_bandwidth = JSNumber(getProperty("maxAudioBandwidth"));
        conf.media_codec = JSMediaCodec(getProperty("maxAudioBandwidth"));
        JSArray servers = JSArray(getProperty("iceServers"));
        for (size_t i = 0; i < servers.length(); ++i) {
            base::IceServer server(JSIceServer(servers[i]));
            conf.ice_servers.push_back(std::move(server));
        }
        return conf;
    }

private:
    class JSIceServer : JSObject {
    public:
        JSIceServer(const JSValue& jsval): JSObject(jsval){}
        operator base::IceServer() const {
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
    };
    class JSMediaCodec : JSObject {
    public:
        JSMediaCodec(const JSValue& jsval): JSObject(jsval){}
        operator base::MediaCodec() const {
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
    };
};


class ConferenceClient : conference::ConferenceClientObserver {
public:
    ConferenceClient(JSObject, JSArray args) {
        base::ClientConfiguration conf  = JSClientConfiguration(args[0]);
        conference::ConferenceClientConfiguration confconf;
        memcpy(&confconf, (void*)&conf, sizeof(confconf));
        client_ = std::make_shared<conference::ConferenceClient>(confconf);
        client_->AddObserver(*this);
    }
    virtual ~ConferenceClient() = default;

    JSValue join(JSObject, JSArray args) {
        JSString token(args[0]);
        JSFunction resolve(args[1]), reject(args[2]);

        client_->Join(token,
            [=](std::shared_ptr<conference::User> user) {
                if (resolve)  resolve(nullptr, 1, JSUser(*user));
            },
            [=](std::unique_ptr<conference::ConferenceException> err) {
                if (reject)  reject(nullptr, 1, JSString(err->Message()));
            });
        return JSUndefined();
    }

    JSValue publish(JSObject, JSArray args) {
        JSNativeObject<LocalCameraStream> jsobj(args[0]);
        JSFunction resolve(args[1]), reject(args[2]);
        client_->Publish(jsobj->operator->(),
            [=]() {
                if (resolve)  resolve();
            },
            [=](std::unique_ptr<conference::ConferenceException> err) {
                if (reject)  reject();
        });
        return JSUndefined();
    }

    JSValue subscribe(JSObject, JSArray args) {
        JSNativeObject<RemoteStream> jsobj(args[0]);
        JSSubscribeOptions options(args[1]);
        JSFunction resolve(args[2]), reject(args[3]);
        client_->Subscribe(jsobj->operator->(),
            [=](std::shared_ptr<base::RemoteStream> stream) {
                if (resolve)  resolve(nullptr, 1, RemoteStream::wrap(stream));
            },
            [=](std::unique_ptr<conference::ConferenceException> err) {
                if (reject)  reject();
        });
        return JSUndefined();
    }

    JSValue unpublish(JSObject, JSArray args) {
        JSNativeObject<LocalCameraStream> jsobj(args[0]);
        JSFunction resolve(args[1]), reject(args[2]);
        client_->Unpublish(jsobj->operator->(),
            [=]() {
                if (resolve)  resolve();
            },
            [=](std::unique_ptr<conference::ConferenceException> err) {
                if (reject)  reject();
            });
        return JSUndefined();
    }

    JSValue unsubscribe(JSObject, JSArray args) {
        JSNativeObject<RemoteStream> jsobj(args[0]);
        JSFunction resolve(args[1]), reject(args[2]);
        client_->Unsubscribe(jsobj->operator->(),
            [=]() {
                if (resolve)  resolve();
            },
            [=](std::unique_ptr<conference::ConferenceException> err) {
                if (reject)  reject();
            });
        return JSUndefined();
    }

    JSValue send(JSObject, JSArray args) {
        JSString message(args[0]), receiver(args[1]);
        JSSubscribeOptions options(args[1]);
        JSFunction resolve(args[2]), reject(args[3]);

        client_->Send(message, receiver,
            [=](void) {
                if (resolve)  resolve();
            },
            [=](std::unique_ptr<conference::ConferenceException> err) {
                if (reject)  reject();
            });
        return JSUndefined();
    }

    // TODO
    JSValue playAudio(JSObject, JSArray args) {
        return JSUndefined();
    }
    JSValue pauseAudio(JSObject, JSArray args) {
        return JSUndefined();
    }
    JSValue playVideo(JSObject, JSArray args) {
        return JSUndefined();
    }
    JSValue pauseVideo(JSObject, JSArray args) {
        return JSUndefined();
    }

    JSValue leave(JSObject, JSArray args) {
        JSFunction resolve(args[0]), reject(args[1]);
        client_->Leave(
            [=](void) {
                if (resolve)  resolve();
            },
            [=](std::unique_ptr<conference::ConferenceException> err) {
                if (reject)  reject();
            });
        return JSUndefined();
    }

    JSValue getRegion(JSObject, JSArray args) {
        JSNativeObject<RemoteStream> jsobj(args[0]);
        JSFunction resolve(args[1]), reject(args[2]);
        client_->GetRegion(jsobj->operator->(),
            [=](std::string id) {
                if (resolve)  resolve(nullptr, 1, JSString(id));
            },
            [=](std::unique_ptr<conference::ConferenceException> err) {
                if (reject)  reject();
            });
        return JSUndefined();
    }
    JSValue setRegion(JSObject, JSArray args) {
        JSNativeObject<RemoteStream> jsobj(args[0]);
        JSString id(args[1]);
        JSFunction resolve(args[2]), reject(args[3]);
        client_->SetRegion(jsobj->operator->(), id,
            [=](void) {
                if (resolve)  resolve();
            },
            [=](std::unique_ptr<conference::ConferenceException> err) {
                if (reject)  reject();
            });
        return JSUndefined();
    }

    JSValue addEventListener(JSObject, JSArray args) {
        static std::set<std::string> event_types = {
            "user-joined", "user-left", "stream-added", "stream-removed",
            "message-received", "server-disconnected"
        };

        std::string type = JSString(args[0]);
        if (event_types.find(type) == event_types.end())
            return JSUndefined();

        JSGlobalValue listener(args[1]);
        if (findListener(type, listener) == listeners_.end()) {
            listeners_.insert(std::make_pair(type, std::move(listener)));
        }
        return JSUndefined();
    }

    JSValue removeEventListener(JSObject, JSArray args) {
        std::string type = JSString(args[0]);
        JSGlobalValue listener(args[1]);
        auto it = findListener(type, listener);
        if (it != listeners_.end()) {
            listeners_.erase(it);
        }
        return JSUndefined();
    }

    JSValue dispatchEvent(JSObject, JSArray args) {
        // This can be called from script for testing purpose.
        std::string type = JSString(args[0]);
        JSFunction shift(args.getProperty("shift"));
        shift(args, 0);
        dispatchEvent(type, args);
        return JSBoolean(true);
    }

    static void setup(JSObject cls) {
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

private:
    void dispatchEvent(std::string type, JSArray args) {
        YUNOS_LOG(Info, LOG_TAG, "%s event received", type.c_str());
        auto it = findListener(type, JSGlobalValue());
        if (it == listeners_.end())  return;
        while (it != listeners_.end()) {
            JSFunction listener(it->second.newLocalValue());
            listener(nullptr, args);
            ++it;
        }
        YUNOS_LOG(Info, LOG_TAG, "%s event dispatched", type.c_str());
    }

    void OnStreamAdded(std::shared_ptr<conference::RemoteCameraStream> stream) override {
        dispatchEvent("stream-added", JSArray(RemoteStream::wrap(stream)));
    }
    void OnStreamAdded(std::shared_ptr<conference::RemoteScreenStream> stream) override {
        dispatchEvent("stream-added", JSArray(RemoteStream::wrap(stream)));
    }
    void OnStreamAdded(std::shared_ptr<conference::RemoteMixedStream> stream) override {
        dispatchEvent("stream-added", JSArray(RemoteStream::wrap(stream)));
    }
    void OnStreamRemoved(std::shared_ptr<conference::RemoteCameraStream> stream) override {
        dispatchEvent("stream-removed", JSArray(RemoteStream::wrap(stream)));
    }
    void OnStreamRemoved(std::shared_ptr<conference::RemoteScreenStream> stream) override {
        dispatchEvent("stream-removed", JSArray(RemoteStream::wrap(stream)));
    }
    void OnStreamRemoved(std::shared_ptr<conference::RemoteMixedStream> stream) override {
        dispatchEvent("stream-removed", JSArray(RemoteStream::wrap(stream)));
    }
    void OnUserJoined(std::shared_ptr<const conference::User> user) override {
        dispatchEvent("user-joined", JSArray(JSUser(*user)));
    }
    void OnUserLeft(std::shared_ptr<const conference::User> user) override {
        dispatchEvent("user-left", JSArray(JSUser(*user)));
    }
    void OnMessageReceived(std::string& sender_id, std::string& message) override {
        dispatchEvent("message-received", JSArray({sender_id, message}));
    }
    void OnServerDisconnected() override {
        dispatchEvent("server-disconnected", JSArray());
    }

    typedef std::multimap<std::string, JSGlobalValue> ListenerBag;
    ListenerBag::iterator findListener(std::string type, const JSGlobalValue& listener) {
        auto it = listeners_.find(type);
        if (listener) {
            while (it != listeners_.end()) {
                if (it->second == listener)
                    return it;
                ++it;
            }
        }
        return it;
    }
    ListenerBag listeners_;
    std::shared_ptr<conference::ConferenceClient> client_;

private:
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
        JSUser(const conference::User& user): JSObject({
            { "id",          JSString(user.Id()) },
            { "name",        JSString(user.Name()) },
            { "role",        JSString(user.Role()) },
            { "permissions", JSObject({
                { "canPublish",   JSBoolean(user.Permissions().CanPublish()) },
                { "canRecord",    JSBoolean(user.Permissions().CanRecord()) },
                { "canSubscribe", JSBoolean(user.Permissions().CanSubscribe()) }
            })}
        }){}
    };
};
