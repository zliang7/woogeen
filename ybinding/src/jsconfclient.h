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
#include <unordered_map>

#include <woogeen/conference/remotemixedstream.h>
#include <woogeen/conference/conferenceclient.h>

#include <jsnipp.h>

#include "jsstream.h"

using namespace woogeen;
using namespace jsnipp;

class ConferenceClient : conference::ConferenceClientObserver {
public:
    ConferenceClient(JSObject, JSArray args);
    virtual ~ConferenceClient() = default;

    JSValue join(JSObject, JSArray args);
    JSValue publish(JSObject, JSArray args);
    JSValue subscribe(JSObject, JSArray args);
    JSValue unpublish(JSObject, JSArray args);
    JSValue unsubscribe(JSObject, JSArray args);
    JSValue send(JSObject, JSArray args);
    JSValue playAudio(JSObject, JSArray args);
    JSValue pauseAudio(JSObject, JSArray args);
    JSValue playVideo(JSObject, JSArray args);
    JSValue pauseVideo(JSObject, JSArray args);
    JSValue leave(JSObject, JSArray args);
    JSValue getRegion(JSObject, JSArray args);
    JSValue setRegion(JSObject, JSArray args);

    JSValue addEventListener(JSObject, JSArray args);
    JSValue removeEventListener(JSObject, JSArray args);
    JSValue dispatchEvent(JSObject, JSArray args);

    static void setup(JSObject cls);

private:
    void OnStreamAdded(std::shared_ptr<conference::RemoteCameraStream> stream) override;
    void OnStreamAdded(std::shared_ptr<conference::RemoteScreenStream> stream) override;
    void OnStreamAdded(std::shared_ptr<conference::RemoteMixedStream> stream) override;
    void OnStreamRemoved(std::shared_ptr<conference::RemoteCameraStream> stream) override;
    void OnStreamRemoved(std::shared_ptr<conference::RemoteScreenStream> stream) override;
    void OnStreamRemoved(std::shared_ptr<conference::RemoteMixedStream> stream) override;
    void OnMessageReceived(std::string& sender_id, std::string& message) override;
    void OnUserJoined(std::shared_ptr<const conference::User> user) override;
    void OnUserLeft(std::shared_ptr<const conference::User> user) override;
    void OnServerDisconnected() override;

    template <class C, typename... Ts>
    void dispatchEvent(const std::string& type, Ts... args);
    std::unordered_multimap<std::string, std::unique_ptr<JSCallbackBase>> listeners_;

    std::shared_ptr<conference::ConferenceClient> client_;
};
