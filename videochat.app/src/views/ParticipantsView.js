"use strict";

const CompositeView = require("yunos/ui/view/CompositeView");
const ColumnLayout = require("yunos/ui/layout/ColumnLayout");
const TextView = require("yunos/ui/view/TextView");
const ListView = require("yunos/ui/view/ListView");
const ParticipantAdapter = require("../adapters/ParticipantAdapter");
const ViewUtil = require("../utils/ViewUtil");

const TAG = "ParticipantsView: KONTAR ";
class ParticipantsView extends CompositeView {
    constructor(pView) {
        super();
        this._pView = pView;
        this.width = pView.width;
        this.height = 260;
        this.background = "transparent";
        this.createLayout();
        this.initView();
    }

    createLayout() {
        let layout = new ColumnLayout();
        layout.defaultLayoutParam = {
            align: ColumnLayout.Align.FillWidth
        };
        this.layout = layout;
    }

    initView() {
        let title = new TextView();
        title.width = this.width;
        title.height = 68;
        title.fontSize = 16;
        title.paddingLeft = 24;
        title.verticalAlign = TextView.VerticalAlign.Middle;
        title.color = "#FFFFFF";
        title.text = global.getString("participant_title");
        this.addChild(title);
        this.addChild(ViewUtil.getSplitLine({
            width: this.width
        }));

        let participantList = this.mParticipants = new ListView();
        participantList.width = this.width;
        participantList.height = 232;
        participantList.background = "transparent";
        let adapter = new ParticipantAdapter();
        this.mParticipants.adapter = adapter;
        this.addChild(participantList);
    }

    hangup() {
        this.mParticipants.adapter.data = [];
    }

    refreshParticipantList(data) {
        log.I(TAG, "refreshParticipantList", global.isInChatting);
        if (!global.isInChatting) {
            return;
        }
        if (data) {
            this.mParticipants.adapter.data = data;
        } else {
            global.homeController.mChatMgr.getParticipants((err, participants) => {
                log.I(TAG, "getParticipants", err, participants);
                if (!err) {
                    this.mParticipants.adapter.data = participants;
                } else {
                    this.mParticipants.adapter.data = [];
                }
            });
        }
    }
}
module.exports = ParticipantsView;
