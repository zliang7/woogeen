"use strict";

const CompositeView = require("yunos/ui/view/CompositeView");
const View = require("yunos/ui/view/View");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");
const AlertDialog = require("yunos/ui/widget/AlertDialog");
const VideoController = require("./VideoController");
const CallActionsView = require("../views/CallActionsView");
const ChatController = require("./ChatController");
const ChatManagerment = require("../chatservices/ChatManagerment");
const ContactController = require("./ContactController");
const SettingsDialog = require("../views/SettingsDialog");
const ViewUtil = require("../utils/ViewUtil");
const StoreUtil = require("../utils/StoreUtil");

const config = require("../config.json");
global.config = config;
global.config.mcu = "192.168.1.114:3001";
global.config.user = "kontar";

const EVT_SETTING_SERVER_CHANGED = "SettingServerChanged";
const EVT_HANGUP = "hangup";
const EVT_JOIN_CHAT = "join_chat";
const EVT_START_INVITE = "start_invite";
const EVT_SURFACE_READY = "surface_ready";
const EVT_CHAT_INVITE = "chat-invite";
const EVT_CHAT_ACCEPTED = "chat-accepted";
const EVT_CHAT_REFUSED = "chat-refused";
const EVT_CHAT_ERROR = "chat-error";
const EVT_SETTINGS_CHANGED = "settingsChanged";
const EVT_SERV_DISCONNECTED = "server-disconnected";
const EVT_STREAM_SUBSCRIBED = "stream_subscribed";

const TAG = "HomeController: KONTAR ";
class HomeController extends CompositeView {
    constructor(homePage) {
        super();
        this.mHomePage = homePage;
        this.pView = homePage.window;
        this.width = this.pView.width;
        this.height = this.pView.height;
        this.mOutsideViews = new Set();
        global.homeController = this;
        this.init();
        this.showSettings();
    }

    init() {
        this.registerListener();
        this.createLayout();
        this.initView();
    }

    showSettings() {
        log.I(TAG, "showSettings");
        StoreUtil.getData("userInfo", (err, info) => {
            log.I(TAG, "getData", err, info);
            if (!err && info) {
                let userInfo = JSON.parse(info);
                global.config.user = userInfo.user;
                global.config.mcu = userInfo.mcu;
                this.initVideController();
            } else {
                let settingsDialog = new SettingsDialog(this);
                settingsDialog.type = 1003;
                settingsDialog.show();
                settingsDialog.userNameField.focus();
                settingsDialog.userNameField.showSoftKeyboard();
                this.on(EVT_SETTINGS_CHANGED, (username, ipAddress) => {
                    this._started = true;
                    if (ipAddress) {
                        global.config.mcu = ipAddress;
                    }
                    if (username) {
                        global.config.user = username;
                    }
                });
                settingsDialog.on("close", () => {
                    log.I(TAG, "showSettingsDialog closed", global.config.mcu, global.config.user);
                    if (this._started) {
                        this.initVideController();
                    } else {
                        this.mHomePage.stopPage();
                    }
                });
            }
        });
    }

    createLayout() {
        this.mRootLayout = new RelativeLayout();
        this.layout = this.mRootLayout;
        this.mRootLayout.setLayoutParam(0, "align", {
            center: "parent",
            bottom: "parent"
        });
        this.mRootLayout.setLayoutParam(0, "margin", {
            bottom: 10
        });
        this.mRootLayout.setLayoutParam(1, "align", {
            top: "parent",
            right: "parent"
        });
        this.mRootLayout.setLayoutParam(1, "margin", {
            top: 10,
            right: 10
        });
    }

    initView() {
        let callActionsView = this.mCallActions = new CallActionsView(this);
        this.addChild(callActionsView);

        let contactController = this.mContactController = new ContactController(this);
        this.addChild(contactController);
    }

    initVideController() {
        let videoController = this.mVideoCtroller = VideoController.getInstance(this);
        this.addChild(videoController);
        this.sendChildToBack(videoController);
    }

    onBackKey() {
        let viewArr = Array.from(this.mOutsideViews);
        let viewNum = viewArr.length;
        if (viewNum > 0) {
            viewArr[viewNum - 1].hide();
            return true;
        }
        if (global.isInChatting) {
            this.stopChat();
            return true;
        }
        return false;
    }

    showOutsideView(show, view) {
        log.I(TAG, "showOutsideView ", show, view.id);
        if (show) {
            if (this.mOutsideViews.has(view)) {
                this.mOutsideViews.delete(view);
            }
            this.mOutsideViews.add(view);
        } else {
            this.mOutsideViews.delete(view);
        }
    }

    registerListener() {
        log.I(TAG, "registerListener");
        this.on(EVT_SURFACE_READY, (surfaces) => {
            this.initChatManagerment(surfaces);
        });
        this.on(EVT_HANGUP, () => {
            this.stopChat();
        });
        this.on(EVT_START_INVITE, (inviteList) => {
            this.startInvite(inviteList);
        });
        this.on(EVT_JOIN_CHAT, (code) => {
            if (code) {
                this.joinChatByInviteCode(code);
            } else {
                this.joinChatRoom();
            }
        });
        this.on(EVT_CHAT_INVITE, (sender) => {
            this.reciveCall(sender);
        });
        this.on(EVT_CHAT_ACCEPTED, (sender) => {
            ViewUtil.showToast(sender.name + "接受了您的邀请");
        });
        this.on(EVT_CHAT_REFUSED, (sender) => {
            ViewUtil.showToast(sender.name + "拒绝了您的邀请");
        });
        this.on(EVT_CHAT_ERROR, (data) => {
            log.E(TAG, EVT_CHAT_ERROR, data);
            ViewUtil.showToast("邀请失败");
        });
        this.on(EVT_SERV_DISCONNECTED, (user) => {
            ViewUtil.showToast(user + "断开了连接");
        });
        this.on(EVT_SETTING_SERVER_CHANGED, (username, ipAddress) => {
            this.onServerChanged(username, ipAddress);
        });

        this.on(EVT_STREAM_SUBSCRIBED, () => {
            setTimeout(() => {
                this.mVideoCtroller.joinChatRoom();
            }, 1500);
        });
    }

    onServerChanged(username, ipAddress) {
        log.I(TAG, "onServerChanged ipAddress", ipAddress);
        // crash block
        // this.mChatMgr.deinit();
    }

    initChatManagerment(surfaces) {
        log.I(TAG, "initChatManagerment ");
        this.mChatMgr = new ChatManagerment();
        this.mChatMgr.init(this, error => {
            log.I(TAG, "init start chat", error);
            if (!error) {
                this.mChatMgr.initWebrtc(surfaces.remoteSurface, surfaces.localSurface);
                global.myInfo = this.mChatMgr.getMyself();
            } else {
                log.E(TAG, "init start chat, error:" + error);
                ViewUtil.showToast("请重新设置服务器地址");
            }
        });
    }

    reciveCall(sender) {
        log.I(TAG, "receive invitation from ", sender);
        let beingCalledDialog = new AlertDialog();
        beingCalledDialog.message = global.getString("being_called", {
            name: sender.name
        });
        beingCalledDialog.buttons = [global.getString("hangup"), global.getString("join_meeting")];
        beingCalledDialog.show();
        beingCalledDialog.on("result", (index) => {
            if (index === 1) {
                this.mReceived = true;
            }
        });
        beingCalledDialog.on("close", () => {
            if (this.mReceived) {
                this.mChatMgr.acceptInvitation((err) => {
                    if (err) {
                        log.E(TAG, "acceptInvitation failed error:", err);
                    } else {
                        log.I(TAG, "acceptInvitation succeed:");
                        this.joinChatRoom();
                    }
                });
            } else {
                this.mChatMgr.refuseInvitation((err) => {
                    if (err) {
                        log.E(TAG, "refuseInvitation failed error:", err);
                    } else {
                        log.I(TAG, "refuseInvitation succeed:");
                    }
                });
            }
            this.mReceived = false;
        });
    }

    startInvite(contacts) {
        log.I(TAG, "startInvite", contacts);
        let invite = (err) => {
            if (err) {
                log.E(TAG, "invite contact failed error:", err);
            } else {
                log.I(TAG, "invite contact succeed");
                if (global.isInChatting) {
                    this.mChatController.mChatRoomController.mParticipantsView.refreshParticipantList();
                }
                this.joinChatRoom();
            }
        };
        for (let i = 0, len = contacts.length; i < len; i++) {
            let contact = contacts[i];
            this.mChatMgr.inviteContact(contact, invite.bind(this));
        }
    }

    joinChatRoom() {
        log.I(TAG, "joinChatRoom global.isInChatting:", global.isInChatting);
        if (global.isInChatting) {
            return;
        }
        // this.mVideoCtroller.joinChatRoom();
        this.mCallActions.visibility = View.Visibility.None;
        this.mContactController.switchToChatting(true);
        if (!this.mChatController) {
            this.mChatController = new ChatController(this);
            this.addChild(this.mChatController);
            this.bringChildToFront(this.mContactController);
        } else {
            this.mChatController.visibility = View.Visibility.Visible;
            this.mChatController.onUpdate();
        }
    }

    joinChatByInviteCode(code) {
        log.I(TAG, "joinChatByInviteCode code :", code);
        this.mChatMgr.joinChatByInviteCode(code, (err) => {
            if (err) {
                log.E(TAG, "joinChatRoom failed error:", err);
                this.mCallActions.visibility = View.Visibility.Visible;
            } else {
                log.I(TAG, "acceptInvitation succeed:");
                // join chat room
            }
            this.joinChatRoom();
        });
    }

    onStop() {
        this.mChatMgr.deinit();
    }

    stopChat() {
        log.I(TAG, "stopChat");
        this.mVideoCtroller.leaveChatRoom();
        this.mCallActions.visibility = View.Visibility.Visible;
        this.mContactController.switchToChatting(false);
        this.mChatController.hangup();
        this.mChatMgr.leaveChatRoom((err) => {
            if (err) {
                log.E(TAG, "leaveChatRoom error ", err);
            }
        });
    }

}
module.exports = HomeController;
