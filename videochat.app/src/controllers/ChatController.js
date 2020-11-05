"use strict";

const CompositeView = require("yunos/ui/view/CompositeView");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");
const View = require("yunos/ui/view/View");
const ChattingActionsView = require("../views/ChattingActionsView");
const ChattingHeaderView = require("../views/ChattingHeaderView");
const ChatRoomController = require("./ChatRoomController");

const EVT_USER_JOINED = "user-joined";
const EVT_USER_LEFT = "user-left";
const EVT_SERV_DISCONNECTED = "server-disconnected";
const TAG = "ChatController: KONTAR ";
class ChatController extends CompositeView {
    constructor(pView) {
        super();
        this.mChatMgr = pView.mChatMgr;
        this.width = pView.width;
        this.height = pView.height;
        global.isInChatting = true;
        this.createLayout();
        this.initView();
        this.registerListener();
        global.myInfo = this.mChatMgr.getMyself();
    }

    createLayout() {
        this.mRootLayout = new RelativeLayout();
        this.layout = this.mRootLayout;

        this.mRootLayout.setLayoutParam(0, "align", {
            bottom: "parent",
            right: "parent"
        });
        this.mRootLayout.setLayoutParam(0, "margin", {
            bottom: 10
        });

        this.mRootLayout.setLayoutParam(1, "align", {
            top: "parent"
        });
        this.mRootLayout.setLayoutParam(1, "margin", {
            top: 10
        });
    }

    onUpdate() {
        global.isInChatting = true;
        this.mChattingActions.onUpdate();
        this.mChattingHeader.onUpdate();
        this.mChatRoomController.onUpdate();
    }

    hangup() {
        this.mChattingActions.hangup();
        this.mChattingHeader.hangup();
        this.mChatRoomController.hangup();
        this.visibility = View.Visibility.None;
        global.isInChatting = false;
    }

    initView() {
        let chattingActions = this.mChattingActions = new ChattingActionsView(this);
        this.addChild(chattingActions);

        let chattingHeader = this.mChattingHeader = new ChattingHeaderView(this);
        this.addChild(chattingHeader);

        let chatRoomController = this.mChatRoomController = new ChatRoomController(this);
        this.addChild(chatRoomController);

        this.updateParticipants();
    }

    registerListener() {
        global.homeController.on(EVT_USER_JOINED, (user) => {
            log.I(TAG, EVT_USER_JOINED, user);
            this.updateParticipants(user);
        });
        global.homeController.on(EVT_USER_LEFT, (user) => {
            log.I(TAG, EVT_USER_LEFT, user);
            this.updateParticipants(user);
        });
        global.homeController.on(EVT_SERV_DISCONNECTED, (user) => {
            log.I(TAG, EVT_SERV_DISCONNECTED, user);
        });
    }

    updateParticipants(user) {
        log.I(TAG, "updateParticipants user:", user);
        global.homeController.mChatMgr.getParticipants((err, participants) => {
            log.I(TAG, "getParticipants", err, participants);
            if (participants.length === 1 && user && user.id !== participants[0].id || err) {
                log.I(TAG, "auto hangup");
                participants = [];
                global.homeController.stopChat();
            } else {
                this.mChattingHeader.updateParticipantNumber(participants.length);
                this.mChatRoomController.updateParticipants(participants);
            }
        });
    }

}
module.exports = ChatController;
