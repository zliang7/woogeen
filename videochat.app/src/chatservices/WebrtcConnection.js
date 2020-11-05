"use strict";

const Woogeen = require("../utils/woogeen");

const TAG = "<WebrtcConnection>";

const EVT_STREAM_ADDED = "stream-added";
const EVT_STREAM_REMOVED = "stream-removed";
const EVT_STREAM_SUBSCRIBED = "stream_subscribed";

class WebrtcConnection {
    init(client, preview, rmtSurface) {
        log.I(TAG, "init======");
        this.client = client;
        this.preview = preview;
        this.rmtSurface = rmtSurface;

        // LocalCustomStream configuration
        const localCustomStreamConfig = {
            cameraId: 0,
            // surface: preview.getClientToken(),
            surface: "hidden",
            videoEnabled: true,
            audioEnabled: true,
            width: this.preview.width,
            height: this.preview.height,
            fps: 25,
            bitrate: 1024
        };

        // ConferenceClient
        this.localStream = new Woogeen.LocalCustomStream(localCustomStreamConfig);
    }

    listenStreamEvents() {
        log.I(TAG, "listenStreamEvents set listeners!");
        this.woogeenClient = this.client.getWoogeenClient();

        this.woogeenClient.addEventListener(EVT_STREAM_ADDED, (stream) => {
            log.I(TAG, "listenStreamEvents event " + EVT_STREAM_ADDED + " received: from=" + stream.from + ", id=" + stream.id);
            if (stream.id === this.localStream.id) {
                log.I(TAG, "Ignore local stream: " + stream.id);
                return;
            }
            if (this.remoteStream) {
                log.I(TAG, "listenStreamEvents remote stream exists: " + stream.id);
                return;
            }
            if (this.rmtSurface) {
                if (stream.from) {
                    log.I(TAG, "listenStreamEvents do not handle user stream: " + stream.id);
                    return;
                }
                log.I(TAG, "listenStreamEvents stream really added: " + stream.id);
                this.remoteStream = stream;
                this.subscribeStream(this.rmtSurface, stream);
            } else {
                let listener = this.client.getListener();
                listener.emit(EVT_STREAM_ADDED, stream);
            }
        });

        this.woogeenClient.addEventListener(EVT_STREAM_REMOVED, (stream) => {
            log.I(TAG, "listenStreamEvents event " + EVT_STREAM_REMOVED + " received: from=" + stream.from + ", id=" + stream.id);
            if (!this.rmtSurface) {
                let listener = this.client.getListener();
                listener.emit(EVT_STREAM_REMOVED, stream);
            }
        });
    }

    subscribeStream(rmtSurface, stream, callback) {
        const subscribeOptions = {
            surface: rmtSurface,
            resolution: {
                width: rmtSurface.width,
                height: rmtSurface.height
            }
        };
        log.I(TAG, "subscribeStream subscribe stream: " + stream.id);
        this.woogeenClient.subscribe(stream, subscribeOptions)
        .then(() => {
            log.I(TAG, "subscribeStream subscribed to stream: " + stream.id);
            let listener = this.client.getListener();
            listener.emit(EVT_STREAM_SUBSCRIBED);
            if (callback) {
                callback(null);
            }
        }, (err) => {
            log.E(TAG, "subscribeStream failed to subscribe: " + err);
            if (callback) {
                callback(err);
            }
        });
    }

    connect(callback) {
        log.I(TAG, "connecting...");
        if (this.connected) {
            log.E(TAG, "connect already existed!");
            if (callback) {
                callback("Connection already existed!");
            }
            return;
        }
        this.connected = true;




        // if (!this.remoteStream && this.client.mixedStream) {
        //     log.I(TAG, "connect Stream really added: " + this.client.mixedStream.id);
        //     this.remoteStream = this.client.mixedStream;
        //
        //     log.I(TAG, "connect remote surface:", this.rmtSurface);
        //     const subscribeOptions = {
        //         surface: this.rmtSurface,
        //         resolution: {
        //             width: this.rmtSurface.width,
        //             height: this.rmtSurface.height
        //         }
        //     };
        //     this.woogeenClient.subscribe(this.remoteStream, subscribeOptions)
        //     .then(() => {
        //         log.I(TAG, "listenStreamEvents Subscribed to stream: " + this.remoteStream.id);
        //     }, (err) => {
        //         log.E(TAG, "listenStreamEvents Failed to subscribe: " + err);
        //     });
        // }


        this.woogeenClient.publish(this.localStream)
        .then(() => {
            log.I(TAG, "connect published successfully, stream: " + this.localStream.id);
            if (callback) {
                callback(null);
            }
        })
        .catch((error) => {
            log.E(TAG, "connect failed to publish: " + error);
            if (callback) {
                callback(error);
            }
        });
    }

    disconnect(callback) {
        // Zhenyu
        if (!this.remoteStream) {
            if (callback) {
                callback(null);
            }
            return;
        }
        log.I(TAG, "disconnect ...." + this.remoteStream.id);
        this.woogeenClient.unsubscribe(this.remoteStream)
        .then(() => {
            log.I(TAG, "disconnect unsubscribed stream: " + this.remoteStream.id);
            this.remoteStream = null;

            return this.woogeenClient.unpublish(this.localStream);
        })
        .catch((error) => {
            log.E(TAG, "disconnect failed to unsubscribed remote stream: " + error);
            if (callback) {
                callback(error);
            }
        })
        .then(() => {
            log.I(TAG, "disconnect unpublished stream: " + this.localStream.id);
            this.connected = false;

            if (callback) {
                callback(null);
            }
        })
        .catch((error) => {
            log.E(TAG, "disconnect failed to unpublished local stream: " + error);
            if (callback) {
                callback(error);
            }
        });
    }

    mute(callback) {
        log.I(TAG, "mute woogeenClient:", this.woogeenClient);
        if (this.localStream) {
            this.localStream.disableAudio();
        }
        if (callback) {
            callback(null);
        }
    }

    umute(callback) {
        log.I(TAG, "umute woogeenClient:", this.woogeenClient);
        if (this.localStream) {
            this.localStream.enableAudio();
        }
        if (callback) {
            callback(null);
        }
    }

    playAudio(callback) {
        if (!this.connected) {
            return;
        }
        log.I(TAG, "playAudio woogeenClient:", this.woogeenClient);
        // this.woogeenClient.playAudio(this.localStream)
        this.woogeenClient.playAudio(this.remoteStream)
        .then(() => {
            log.I(TAG, "playAudio stream id: " + this.remoteStream.id);
            // log.I(TAG, "playAudio stream id: " + this.localStream.id);
            if (callback) {
                callback();
            }
        })
        .catch((error) => {
            log.E(TAG, "playAudio failed to play audio: " + error);
            if (callback) {
                callback(error);
            }
        });
        log.I(TAG, "playAudio woogeenClient2:", this.woogeenClient);
    }

    pauseAudio(callback) {
        if (!this.connected) {
            return;
        }

        log.I(TAG, "pauseAudio woogeenClient:", this.woogeenClient);
        // this.woogeenClient.pauseAudio(this.localStream)
        this.woogeenClient.pauseAudio(this.remoteStream)
        .then(() => {
            // log.I(TAG, "pauseAudio stream id: " + this.localStream.id);
            log.I(TAG, "pauseAudio stream id: " + this.remoteStream.id);
            if (callback) {
                callback();
            }
        })
        .catch((error) => {
            log.E(TAG, "pauseAudio failed to pause audio: " + error);
            if (callback) {
                callback(error);
            }
        });
        log.I(TAG, "pauseAudio woogeenClient2:", this.woogeenClient);
    }

    deinit() {
        this.disconnect();

        this.localStream.close();
        this.localStream = null;

        this.woogeenClient = null;
        this.client = null;
    }
}

module.exports = WebrtcConnection;
