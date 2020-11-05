"use strict";

const CompositeView = require("yunos/ui/view/CompositeView");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");
const SurfaceView = require("yunos/ui/view/SurfaceView");
const TAG = "VideoController: KONTAR ";
class VideoController extends CompositeView {
    constructor(pView) {
        super();
        this.mHomeController = pView;
        this.mCamId = 0;
        this.width = pView.width;
        this.height = pView.height;
        this.createLayout();
        this.initChatSurface();
    }

    initChatSurface() {
        // init surface
        let localSurface = this.mLocalSurface = this.createSurface({
            width: 1280,
            height: 720,
            type: SurfaceView.SurfaceType.Local
        });
        let preview = this.mPreview = this.createSurface({
            width: 640,
            height: 480,
            visible: false,
            type: SurfaceView.SurfaceType.Nested
        });
        localSurface.on("ready", () => {
            log.I(TAG, "ready localSurface address: " + localSurface.getSurfaceAddress());
            this.mLocalSrufaceReady = true;
            this.startChatVideo();
        });
        preview.on("ready", () => {
            log.I(TAG, "ready preview token: " + preview.getClientToken());
            this.mPreviewSrufaceReady = true;
            this.startChatVideo();
        });
        this.addChild(localSurface);
        this.addChild(preview);
    }

    startChatVideo() {
        if (this.mLocalSrufaceReady && this.mPreviewSrufaceReady) {
            global.homeController.emit("surface_ready", {
                localSurface: this.mLocalSurface,
                remoteSurface: this.mPreview
            });
            this.mLocalSurface.setZOrderOnTop(false);
            this.mPreview.setZOrderOnTop(false);
        }
    }

    static getInstance(pView) {
        if (!this.instance) {
            this.instance = new VideoController(pView);
        }
        return this.instance;
    }

    // destroy() {}

    createLayout() {
        this.mRootLayout = new RelativeLayout();
        this.layout = this.mRootLayout;
        this.mRootLayout.setLayoutParam(0, "align", {
            center: "parent",
            top: "parent"
        });
        this.mRootLayout.setLayoutParam(0, "margin", {
            top: 200
        });
        this.mRootLayout.setLayoutParam(1, "align", {
            right: "parent",
            bottom: "parent"
        });
        this.mRootLayout.setLayoutParam(1, "margin", {
            right: 24,
            bottom: 24
        });
    }

    createSurface(config) {
        try {
            let surfaceView = new SurfaceView();
            surfaceView.width = config.width;
            surfaceView.height = config.height;
            surfaceView.surfaceType = config.type;
            // this.addChild(surfaceView);
            return surfaceView;
        } catch (e) {
            log.E(TAG, "createSurface: excption: " + e.message);
            return null;
        }
    }

    joinChatRoom() {
        this.mRootLayout.setLayoutParam(0, "margin", {
            top: 200
        });
    }

    leaveChatRoom() {
        this.mRootLayout.setLayoutParam(0, "margin", {
            top: -800
        });
    }
}
module.exports = VideoController;
