"use strict";

const CompositeView = require("yunos/ui/view/CompositeView");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");
const TextView = require("yunos/ui/view/TextView");
const TimeUtil = require("../utils/TimeUtil");

const TAG = "ChattingHeaderView: KONTAR ";
class ChattingHeaderView extends CompositeView {
    constructor(pView) {
        super();
        this.width = pView.width;
        this.height = 48;
        this.clipBound = false;
        this.mTime = 0;
        this.createLayout();
        this.initView();
    }

    createLayout() {
        this.mRootLayout = new RelativeLayout();
        this.layout = this.mRootLayout;
        this.mRootLayout.setLayoutParam(0, "align", {
            middle: "parent",
            left: "parent"
        });
        this.mRootLayout.setLayoutParam(0, "margin", {
            left: 24
        });

        this.mRootLayout.setLayoutParam(1, "align", {
            middle: "parent",
            right: "parent"
        });
        this.mRootLayout.setLayoutParam(1, "margin", {
            right: 70
        });
    }

    initView() {
        let participant = this.mParticipantNum = new TextView();
        participant.fontSize = 16;
        participant.color = "#FFFFFF";
        participant.text = global.getString("participant_number", {
            number: 0
        });
        this.addChild(participant);

        let chattingTime = this.mChattingTime = new TextView();
        chattingTime.fontSize = 16;
        chattingTime.color = "#FFFFFF";
        chattingTime.text = "00:00:00";
        this.addChild(chattingTime);
        this.startTimer();
    }

    onUpdate() {
        this.updateParticipantNumber(2);
        this.startTimer();
    }

    hangup() {
        this.endTimer();
        this.updateParticipantNumber(0);
    }

    updateParticipantNumber(number) {
        log.I(TAG, "updateParticipantNumber number", number);
        this.mParticipantNum.text = global.getString("participant_number", {
            number: number
        });
    }

    startTimer() {
        this.mTimer = setInterval(() => {
            this.mTime++;
            this.mChattingTime.text = TimeUtil.formatChattingTime(this.mTime);
        }, 1000);
    }

    endTimer() {
        clearInterval(this.mTimer);
        this.mTimer = null;
        this.mTime = 0;
        this.mChattingTime.text = "00:00:00";
    }
}
module.exports = ChattingHeaderView;
