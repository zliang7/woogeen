//NOTE: you must run `surfaceserver` before starting this app

"use strict";
process.env.CAFUI2 = true;
const Page = require("yunos/page/Page");
const SurfaceView = require("yunos/ui/view/SurfaceView");
const TextView = require("yunos/ui/view/TextView");
const Toast = require("yunos/ui/widget/Toast");
const Woogeen = require('./woogeen.js');
const config = require('./config.json');

const TAG = "woogeen";
const BACKGROUND = "#e5e6e8";

class Main extends Page {
    constructor(props) {
        super(props);
        this.client = null;
        this.localStream = null;
        this.remoteStream = null;
    }

    onStart() {
        let self = this;

        let surface = new SurfaceView();
        surface.width = 1280;
        surface.height = 720;
        surface.surfaceType = SurfaceView.SurfaceType.Local;
        self.window.addChild(surface);

        let preview = new SurfaceView();
        preview.width = 320;
        preview.height = 240;
        preview.surfaceType = SurfaceView.SurfaceType.Nested;
        self.window.addChild(preview);

        surface.on("ready", () => {
            console.log("surface address: " + surface.getSurfaceAddress());
            preview.on("ready", () => {
                console.log("surface token: " + preview.getClientToken());
                self.window.bringChildToFront(surface);
                self.window.sendChildToBack(preview);
                this.connect(surface, preview);
            });
        });
    }

    onStop() {
        this.client.unpublish(this.localStream)
        .then(() => {
            console.log(TAG, "Unpublished stream: " + this.localStream.id);
            this.localStream.close();
            if (this.remoteStream) {
                return this.client.unsubscribe(this.remoteStream)
            } else {
                return Promise.resolve();
            }
        }).then(() => {
            this.client.leave();
        }).then(() => {
            console.log(TAG, "User left.");
            this.client = null;
        });
    }

    connect(surface, preview) {
        // Signaling server
        const iceServer = {
            urls: ["stun:" + config.stun],
            username: "",
            password: ""
        };
        // Media codecs
        const mediaCodec = {
            videoCodec: "H264",
            audioCodec: "OPUS"
        };
        // ConferenceClient configuration
        const clientConfiguration = {
            iceServers: [iceServer],
            mediaCodec: mediaCodec
        };
        // LocalCustomStream configuration
        const localCustomStreamConfig = {
            cameraId: 0,
            surface: preview.getClientToken(),
            videoEnabled: true,
            audioEnabled: true,
            width: 640,
            height: 480,
            fps: 25,
            bitrate: 1024000
        };

        // ConferenceClient
        this.client = new Woogeen.ConferenceClient(clientConfiguration);
        this.localStream = new Woogeen.LocalCustomStream(localCustomStreamConfig);

        this.client.addEventListener('stream-removed', (stream) => {
            console.log(TAG, "stream is removed: " + stream.id);
            if (this.remoteStream && stream.id == this.remoteStream.id)
                this.remoteStream = null;
        });

        this.client.addEventListener('stream-added', (stream) => {
            if (stream.id == this.localStream.id) {
                console.log(TAG, "Ignore local stream: " + stream.id);
                return;
            }
            if (this.remoteStream || stream.from.length > 0) {
                return;
            }
            this.remoteStream = stream;

            console.log(TAG, "Stream was added: " + stream.id);
            const subscribeOptions = {
                surface: surface,
                resolution: {
                    width: surface ? surface.width : 1280,
                    height: surface ? surface.height : 720
                }
            };
            this.client.subscribe(stream, subscribeOptions)
            .then(() => {
                console.log(TAG, "Subscribed to stream: " + stream.id);
            }, (err) => {
                console.log(TAG, "Failed to subscribe: " + err);
            });
        });

        this.client.addEventListener('message-received', (id, msg) => {
            console.log(TAG, "Message received: id="+ id + ", msg=" + msg);
        });

        Woogeen.getToken(config.mcu)
        .then((token) => {
            console.log(TAG, "token: " + token);
            return this.client.join(token);
        })
        .catch((error) => {
            console.log(TAG, "Failed to get token from server: " + error);
            // warning toast
            let tokenWarning = new Toast();
            tokenWarning.text = "Could not connect to server " + config.mcu;
            tokenWarning.show();
        })
        .then((user) => {
            console.log(TAG, "Joined successfully. user: " + user.id);
            setTimeout(()=>{this.client.send("Hello!");}, 5000);
            setTimeout(()=>{this.client.dispatchEvent("message-received", "loopback", "test");}, 6000);
            return this.client.publish(this.localStream);
        })
        .catch((error) => {
            console.log(TAG, "Failed to join: " + error);
        })
        .then(() => {
            console.log(TAG, "Published successfully. stream: " + this.localStream.id);
        })
        .catch((error) => {
            console.log(TAG, "Failed to publish: " + error);
        })
    }
}
module.exports = Main;
