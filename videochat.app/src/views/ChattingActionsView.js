"use strict";

const CompositeView = require("yunos/ui/view/CompositeView");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");
const ImageView = require("yunos/ui/view/ImageView");
const ViewUtil = require("../utils/ViewUtil");
const ICON_WIDTH = 72;
const ICON_HEIGHT = 72;

const TAG = "ChattingActionsView: KONTAR";
class ChattingActionsView extends CompositeView {
    constructor(pView) {
        super();
        this.width = pView.width;
        this.height = ICON_HEIGHT;
        this.mSoundOn = true;
        this.mVoiceOn = true;
        this.createLayout();
        this.createView();
    }

    createLayout() {
        this.mRootLayout = new RelativeLayout();
        this.layout = this.mRootLayout;
        this.mRootLayout.setLayoutParam(0, "align", {
            center: "parent",
            middle: "parent"
        });

        this.mRootLayout.setLayoutParam(1, "align", {
            right: {
                target: 0,
                side: "left"
            },
            middle: "parent"
        });
        this.mRootLayout.setLayoutParam(1, "margin", {
            right: 30
        });

        this.mRootLayout.setLayoutParam(2, "align", {
            left: {
                target: 0,
                side: "right"
            },
            middle: "parent"
        });
        this.mRootLayout.setLayoutParam(2, "margin", {
            left: 30
        });
    }

    createView() {
        let hangupIcon = this.mHangup = new ImageView();
        hangupIcon.id = "hangup";
        hangupIcon.width = ICON_WIDTH;
        hangupIcon.height = ICON_HEIGHT;
        hangupIcon.src = global.getImageSrc("icon_hangup.png");
        this.addChild(hangupIcon);
        ViewUtil.addTapListener(hangupIcon, () => {
            log.D(TAG, "hangUpIcon taped ");
            this.onTapListener(hangupIcon);
        });

        let soundIcon = this.mSound = new ImageView();
        soundIcon.id = "sound";
        soundIcon.width = ICON_WIDTH;
        soundIcon.height = ICON_HEIGHT;
        soundIcon.src = global.getImageSrc("icon_sound_on.png");
        this.addChild(soundIcon);
        ViewUtil.addTapListener(soundIcon, () => {
            log.D(TAG, "soundIcon taped");
            this.onTapListener(soundIcon);
        });

        let voiceIcon = this.mVoice = new ImageView();
        voiceIcon.id = "voice";
        voiceIcon.width = ICON_WIDTH;
        voiceIcon.height = ICON_HEIGHT;
        voiceIcon.src = global.getImageSrc("icon_voice_on.png");
        this.addChild(voiceIcon);
        ViewUtil.addTapListener(voiceIcon, () => {
            log.D(TAG, "voiceIcon taped");
            this.onTapListener(voiceIcon);
        });
    }

    onTapListener(view) {
        switch (view.id) {
            case "hangup":
                global.homeController.emit("hangup");
                break;
            case "sound":
                if (this.mSoundOn) {
                    global.homeController.mChatMgr.pauseAudio((err) => {
                        if (err) {
                            log.E(TAG, "pause Audio failed error:", err);
                        }
                    });
                    view.src = global.getImageSrc("icon_sound_off.png");
                    this.mSoundOn = false;
                } else {
                    global.homeController.mChatMgr.playAudio((err) => {
                        if (err) {
                            log.E(TAG, "play Audio failed error:", err);
                        }
                    });
                    view.src = global.getImageSrc("icon_sound_on.png");
                    this.mSoundOn = true;
                }
                break;
            case "voice":
                if (this.mVoiceOn) {
                    global.homeController.mChatMgr.mute((err) => {
                        if (err) {
                            log.E(TAG, "mute Audio failed error:", err);
                        }
                    });
                    view.src = global.getImageSrc("icon_voice_off.png");
                    this.mVoiceOn = false;
                } else {
                    global.homeController.mChatMgr.umute((err) => {
                        if (err) {
                            log.E(TAG, "umute Audio failed error:", err);
                        }
                    });
                    view.src = global.getImageSrc("icon_voice_on.png");
                    this.mVoiceOn = true;
                }
                break;
            default:
                break;
        }
    }

    hangup() {
        this.onUpdate();
    }

    onUpdate() {
        this.mSound.src = global.getImageSrc("icon_sound_on.png");
        this.mSoundOn = true;
        this.mVoice.src = global.getImageSrc("icon_voice_on.png");
        this.mVoiceOn = true;
    }
}
module.exports = ChattingActionsView;
