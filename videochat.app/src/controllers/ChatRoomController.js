"use strict";

const CompositeView = require("yunos/ui/view/CompositeView");
const ColumnLayout = require("yunos/ui/layout/ColumnLayout");
const RowLayout = require("yunos/ui/layout/RowLayout");
const ImageView = require("yunos/ui/view/ImageView");
const Resource = require("yunos/content/resource/Resource");
const resource = Resource.getInstance();
const DialogueView = require("../views/DialogueView");
const ParticipantsView = require("../views/ParticipantsView");
const ViewUtil = require("../utils/ViewUtil");

const TAG = "ChatRoomController: KONTAR ";
class ChatRoomController extends CompositeView {
    constructor(pView) {
        super();
        this.width = 430;
        this.height = pView.height;
        this.left = -396;
        this.background = "transparent";
        this.isShowing = false;
        this.id = "ChatRoomController";
        this.createLayout();
        this.initView();
    }

    createLayout() {
        let layout = this.mRootLayout = new RowLayout();
        layout.defaultLayoutParam = {
            align: RowLayout.Align.Middle
        };
        this.layout = layout;
    }

    initView() {
        this.initLeftView();
        this.initDragView();
    }

    onUpdate() {
        this.updateParticipants();
        this.mDialogueView.updateChatRecords();
    }

    show() {
        if (!this.isShowing) {
            this.left = 0;
            this.isShowing = true;
            global.homeController.showOutsideView(true, this);
        }
    }

    hide() {
        if (this.isShowing) {
            this.left = -this.width + 30;
            this.isShowing = false;
            global.homeController.showOutsideView(false, this);
        }
    }

    hangup() {
        this.mParticipantsView.hangup();
        this.mDialogueView.hangup();
        this.hide();
    }

    initDragView() {
        let dragView = new ImageView();
        dragView.width = 33;
        dragView.height = 68;
        dragView.src = resource.getImageSrc("icon_pull.png");
        this.addChild(dragView);

        ViewUtil.addTapListener(dragView, () => {
            log.I(TAG, "drag icon taped this.isShowing:", this.isShowing);
            if (this.isShowing) {
                this.hide();
            } else {
                this.show();
            }
        });
    }

    initLeftView() {
        let container = this._rootContainer = new CompositeView();
        container.width = 396;
        container.height = this.height;
        container.background = "black";
        container.opacity = 0.92;
        this.addChild(container);

        let layout = new ColumnLayout();
        layout.defaultLayoutParam = {
            align: ColumnLayout.Align.FillWidth
        };
        container.layout = layout;

        let participantsView = this.mParticipantsView = new ParticipantsView(container);
        container.addChild(participantsView);

        let dialogueView = this.mDialogueView = new DialogueView(container);
        container.addChild(dialogueView);
    }

    updateParticipants(data) {
        this.mParticipantsView.refreshParticipantList(data);
    }
}
module.exports = ChatRoomController;
