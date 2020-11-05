"use strict";

const CompositeView = require("yunos/ui/view/CompositeView");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");
const ColumnLayout = require("yunos/ui/layout/ColumnLayout");
const TextView = require("yunos/ui/view/TextView");
const TextArea = require("yunos/ui/view/TextArea");
const Button = require("yunos/ui/widget/Button");
const ListView = require("yunos/ui/view/ListView");
const ChatListAdapter = require("../adapters/ChatListAdapter");
const ViewUtil = require("../utils/ViewUtil");
const EVT_MSG_RECVED = "message-received";

const TAG = "DialogueView: KONTAR ";
class DialogueView extends CompositeView {
    constructor(pView) {
        super();
        this.width = pView.width;
        this.height = 450;
        this.background = "transparent";
        this.mMessages = [];
        this.createLayout();
        this.initView();
        this.registerListener();

    }

    createLayout() {
        let layout = new ColumnLayout();
        layout.defaultLayoutParam = {
            align: ColumnLayout.Align.FillWidth
        };
        this.layout = layout;
    }

    initView() {
        this.initChatRecordView();
        this.initChatInputView();
    }

    initChatRecordView() {
        this.addChild(ViewUtil.getSplitLine({
            width: this.width
        }));
        let title = new TextView();
        title.width = this.width;
        title.height = 68;
        title.fontSize = 16;
        title.paddingLeft = 24;
        title.color = "#FFFFFF";
        title.text = global.getString("dialogue_title");
        title.verticalAlign = TextView.VerticalAlign.Middle;
        this.addChild(title);
        this.addChild(ViewUtil.getSplitLine({
            width: this.width
        }));

        let recordContainer = new CompositeView();
        recordContainer.width = this.width;
        recordContainer.height = 300;
        this.addChild(recordContainer);

        let layout = new RelativeLayout();
        recordContainer.layout = layout;
        layout.setLayoutParam(0, "align", {
            left: "parent",
            top: "parent"
        });

        layout.setLayoutParam(0, "margin", {
            left: 24,
            top: 16
        });

        layout.setLayoutParam(1, "align", {
            left: "parent",
            top: {
                target: 0,
                side: "bottom"
            },
            bottom: "parent"
        });

        layout.setLayoutParam(1, "margin", {
            right: 24,
            top: 16,
            bottom: 66
        });

        let time = this.mChatRecordTime = new TextView();
        time.width = this.width;
        time.height = 14;
        time.fontSize = 14;
        time.color = "#999999";
        time.text = "2017.12.12 星期五";
        time.verticalAlign = TextView.VerticalAlign.Middle;
        recordContainer.addChild(time);

        let chatList = this.mChatList = new ListView();
        chatList.width = this.width;
        chatList.height = 260;
        chatList.dividerHeight = 10;
        chatList.background = "transparent";
        let adapter = new ChatListAdapter();
        chatList.adapter = adapter;
        recordContainer.addChild(chatList);
        this.addChild(ViewUtil.getSplitLine({
            width: this.width
        }));
    }

    registerListener() {
        log.I(TAG, "registerListener");
        global.homeController.on(EVT_MSG_RECVED, (sender, msg) => {
            log.I(TAG, "receive message :", msg);
            this.updateChatRecords({
                id: sender.id,
                name: sender.name,
                message: msg
            });
        });
    }

    updateChatRecords(msg) {
        // Zhenyu
        if (msg) {
            this.mMessages.push(msg);
        }
        this.refreshChatList();
    }

    refreshChatList() {
        this.mChatList.adapter.data = this.mMessages;
        if (this.mChatList.adapter.getCount() > 0) {
            this.mChatList.arriveAt(this.mChatList.adapter.getCount() - 1);
        }
    }

    initChatInputView() {
        let container = new CompositeView();
        container.width = this.width;
        container.height = 76;
        container.background = "transparent";
        this.addChild(container);

        let layout = this._chatInputLayout = new RelativeLayout();
        container.layout = layout;

        layout.setLayoutParam(0, "align", {
            left: "parent",
            middle: "parent"
        });

        layout.setLayoutParam(0, "margin", {
            left: 24,
            right: 5
        });

        layout.setLayoutParam(1, "align", {
            right: "parent",
            middle: "parent"
        });

        layout.setLayoutParam(1, "margin", {
            right: 12
        });

        let chatInput = this.mChatInput = new TextArea();
        chatInput.width = this.width - 100;
        chatInput.color = "#FFFFFF";
        chatInput.opacity = 0.6;
        chatInput.placeholder = global.getString("send_message");
        container.addChild(chatInput);

        let send = new Button();
        send.width = 48;
        send.height = 28;
        send.opacity = 0.4;
        send.text = global.getString("send");
        container.addChild(send);
        send.on("tap", () => {
            log.D(TAG, chatInput.text);
            // let msg = {
            //     name: "Me",
            //     message: chatInput.text
            // };
            global.homeController.mChatMgr.sendMsg(chatInput.text, undefined, (error) => {
                log.I(TAG, "sendMsg error:", error);
            });
            chatInput.text = "";
            // this.mMessages.push(msg);
            // this.updateChatRecords();
        });
    }

    hangup() {
        this.mChatRecordTime.text = "";
        this.mMessages.length = 0;
        this.mChatList.adapter.data = [];
        this.mChatInput.text = "";
    }
}
module.exports = DialogueView;
