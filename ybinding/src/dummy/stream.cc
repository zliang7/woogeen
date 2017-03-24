#include <woogeen/base/stream.h>

namespace woogeen {
namespace base {

Stream::Stream(): id_("") {
}
Stream::Stream(const std::string& id): id_(id) {
}
Stream::~Stream() {
}

std::string Stream::Id() const {
    return id_;
}
void Stream::Id(const std::string& id) {
    id_ = id;
}

void Stream::DisableAudio() {
}
void Stream::EnableAudio() {
}
void Stream::DisableVideo() {
}
void Stream::EnableVideo() {
}
void Stream::AttachVideoRenderer(VideoRendererARGBInterface& renderer){
}
void Stream::DetachVideoRenderer() {
}

LocalStream::LocalStream() {
}
LocalStream::~LocalStream() {
}

std::shared_ptr<LocalCameraStream> LocalCameraStream::Create(
	const LocalCameraStreamParameters& parameters,
	int& error_code) {
    error_code = 0;
    std::shared_ptr<LocalCameraStream> stream(
         new LocalCameraStream(parameters, error_code));
    return error_code ? stream : nullptr;
}
LocalCameraStream::LocalCameraStream(
	const LocalCameraStreamParameters& parameters,
	int& error_code) {
    Id(parameters.CameraId());
}
LocalCameraStream::~LocalCameraStream() {
}

void LocalCameraStream::Close() {
}


RemoteStream::RemoteStream(std::string& id, std::string& from)
    : Stream(id), remote_user_id_(from) {
}

std::string RemoteStream::From() {
    return remote_user_id_;
}

RemoteCameraStream::RemoteCameraStream(std::string& id, std::string& from)
    : RemoteStream(id, from) {
}

RemoteScreenStream::RemoteScreenStream(std::string& id, std::string& from)
    : RemoteStream(id, from) {
}

}
}
