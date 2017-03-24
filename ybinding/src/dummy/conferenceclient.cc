#include <algorithm>
#include <woogeen/conference/conferenceclient.h>
#include <woogeen/conference/remotemixedstream.h>

#include <Log.h>
#define LOG_TAG "woogeen"

namespace webrtc {
class CriticalSectionWrapper
{
public:
    static CriticalSectionWrapper* CreateCriticalSection() {
        return new CriticalSectionWrapper();
    }
    CriticalSectionWrapper() {}
    virtual ~CriticalSectionWrapper() {}
    virtual void Enter() {}
    virtual void Leave() {}
};
}


namespace woogeen {
namespace conference {

ConferenceClient::ConferenceClient(const ConferenceClientConfiguration& configuration) :
    configuration_(configuration),
    crit_sect_(*webrtc::CriticalSectionWrapper::CreateCriticalSection()) {
}
ConferenceClient::~ConferenceClient() {
    delete &crit_sect_;
}

void ConferenceClient::AddObserver(ConferenceClientObserver& observer) {
    observers_.push_back(observer);
}
void ConferenceClient::RemoveObserver(ConferenceClientObserver& observer) {
    observers_.erase(std::find_if(
        observers_.begin(), observers_.end(),
        [&](std::reference_wrapper<ConferenceClientObserver> o) -> bool {
            return &observer == &(o.get());
        }));
}

void ConferenceClient::Join(
    const std::string& token,
    std::function<void(std::shared_ptr<User>)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    if (participants.find("self") == participants.end() && token != "badtoken") {
        auto permission = Permission(true, true, true);
        auto user = std::make_shared<User>("solar", "zhenyu", "engineer", permission);
        participants.insert(std::make_pair("self", user));
        on_success(user);

        signaling_channel_connected_ = true;
        for (auto it : observers_)
            it.get().OnUserJoined(user);

	std::string id = "remote_camera_stream_1";
	std::string from = "California";
        auto stream = std::make_shared<RemoteCameraStream>(id, from);
        for (auto it : observers_)
            it.get().OnStreamAdded(stream);
    } else {
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnkown, "test"));
        LOG("===== %s", e->Message().c_str());  // FIXME!!!
        on_failure(std::move(e));
    }
}

void ConferenceClient::Publish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    if (signaling_channel_connected_) {
        on_success();
    } else {
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnkown, "disconnected"));
        on_failure(std::move(e));
    }
}

void ConferenceClient::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    SubscribeOptions options;
    Subscribe(stream, options, on_success, on_failure);
}

void ConferenceClient::Subscribe(
    std::shared_ptr<RemoteStream> stream,
    const SubscribeOptions& options,
    std::function<void(std::shared_ptr<RemoteStream> stream)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    std::string key = stream->Id();
    if (signaling_channel_connected_ && added_streams_.find(key) == added_streams_.end()) {
        added_streams_.insert(std::make_pair(stream->Id(), stream));
        on_success(stream);
    } else {
        std::string msg = signaling_channel_connected_ ? "existed" : "disconnected";
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnkown, msg));
        on_failure(std::move(e));
    }
}

void ConferenceClient::Unpublish(
    std::shared_ptr<LocalStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    if (signaling_channel_connected_) {
        on_success();
    } else {
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnkown, "disconnected"));
        on_failure(std::move(e));
    }
}

void ConferenceClient::Unsubscribe(
    std::shared_ptr<RemoteStream> stream,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    auto it = added_streams_.find(stream->Id());
    if (signaling_channel_connected_ && it != added_streams_.end()) {
        added_streams_.erase(it);
        on_success();
    } else {
        std::string msg = signaling_channel_connected_ ? "unsubscribed" : "disconnected";
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnkown, msg));
        on_failure(std::move(e));
    }
}

void ConferenceClient::Send(
    const std::string& message,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    Send(message, "", on_success, on_failure);
}

void ConferenceClient::Send(
    const std::string& message,
    const std::string& receiver,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    if (signaling_channel_connected_) {
        on_success();
        if (receiver.empty() || receiver == "self") {
            // loopback
            std::string sender = "self", msg = message;
            for (auto it : observers_)
                it.get().OnMessageReceived(sender, msg);
        }
    } else {
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnkown, "disconnected"));
        on_failure(std::move(e));
    }
}

void ConferenceClient::Leave(
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    auto it = participants.find("self");
    if (it != participants.end()) {
        on_success();

        auto user = it->second;
        for (auto it : observers_)
            it.get().OnUserLeft(user);
        participants.erase(it);
        signaling_channel_connected_ = false;
    } else {
        std::unique_ptr<ConferenceException> e(
            new ConferenceException(ConferenceException::kUnkown, "unjoined"));
        on_failure(std::move(e));
    }
}

void ConferenceClient::GetRegion(
    std::shared_ptr<RemoteStream> stream,
    std::function<void(std::string)> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure){
    on_success("region id");
}

void ConferenceClient::SetRegion(
    std::shared_ptr<RemoteStream> stream,
    const std::string& region_id,
    std::function<void()> on_success,
    std::function<void(std::unique_ptr<ConferenceException>)> on_failure) {
    on_success();
}

void ConferenceClient::OnUserJoined(std::shared_ptr<sio::message> user) {}
void ConferenceClient::OnUserLeft(std::shared_ptr<sio::message> user) {}
void ConferenceClient::OnStreamAdded(std::shared_ptr<sio::message> stream) {}
void ConferenceClient::OnStreamRemoved(std::shared_ptr<sio::message> stream) {}
void ConferenceClient::OnStreamUpdated(std::shared_ptr<sio::message> stream) {}
void ConferenceClient::OnServerDisconnected() {}
void ConferenceClient::OnCustomMessage(std::string& from, std::string& message) {}
void ConferenceClient::OnSignalingMessage(std::shared_ptr<sio::message> message) {}
void ConferenceClient::OnStreamId(const std::string& id, const std::string& label) {}

}
}
