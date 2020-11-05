"use strict";
process.env.CAFUI2 = true;

const Dialog = require("yunos/ui/widget/Dialog");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");
const ColumnLayout = require("yunos/ui/layout/ColumnLayout");
const CompositeView = require("yunos/ui/view/CompositeView");
const View = require("yunos/ui/view/View");
const TapRecognizer = require("yunos/ui/gesture/TapRecognizer");
const TextField = require("yunos/ui/view/TextField");
const Button = require("yunos/ui/widget/Button");
const TAG = "INCodeVerifyDialog: KONTAR ";
class INCodeVerifyDialog extends Dialog {
    constructor(pView) {
        super();
        this.width = 300;
        this.height = 134;
        this.showingMask = false;
        this.initView();
    }

    onBackkey() {
        if (!this._closeOnBackkey) {
            return;
        }
        this.emit("result", 0);
        this.close();
    }

    initView() {
        let layout = new ColumnLayout();
        let layoutView = new CompositeView();
        layoutView.width = 300;
        layoutView.height = 134;
        layoutView.layout = layout;

        let underline = new View();
        underline.height = 1;
        underline.width = 300;
        underline.background = "#efefef";
        layoutView.addChild(underline);

        let inputView = this.inputView();
        layoutView.addChild(inputView);

        let underlineB = new View();
        underlineB.height = 1;
        underlineB.width = 300;
        underlineB.background = "#efefef";
        layoutView.addChild(underlineB);

        let buttonView = this.buttonView();
        layoutView.addChild(buttonView);
        this.addChild(layoutView);
    }

    inputView() {
        let self = this;
        let cv = new CompositeView();
        cv.width = 300;
        cv.height = 82;

        let invatationCode = this.mInvatationCode = new TextField();
        invatationCode.left = 24;
        invatationCode.top = 16;
        invatationCode.width = 252;
        invatationCode.height = 50;
        invatationCode.maxLength = 50;
        invatationCode.placeholder = global.getString("inptu_invatation_code");
        invatationCode.color = "#333333";
        invatationCode.underLine = false;
        invatationCode.clearable = false;
        cv.addChild(invatationCode);
        return cv;
    }

    buttonView() {
        let self = this;
        let layoutA = new RelativeLayout();
        layoutA.setLayoutParam(0, "align", {
            left: "parent"
        });

        layoutA.setLayoutParam(1, "align", {
            left: {
                target: 0,
                side: "right"
            }
        });

        layoutA.setLayoutParam(2, "align", {
            left: {
                target: 1,
                side: "right"
            }
        });
        let buttonView = new CompositeView();
        buttonView.top = 160;
        buttonView.left = 0;
        buttonView.width = 300;
        buttonView.height = 50;
        buttonView.layout = layoutA;

        let cancel = new Button();
        cancel.width = 150;
        cancel.height = 50;
        cancel.buttonColor = "#ffffff";
        cancel.styleType = Button.StyleType.None;
        cancel.text = global.getString("button_cancel");
        cancel.color = "#000000aa";
        cancel.addGestureRecognizer(new TapRecognizer({
            event: "tap"
        }));

        cancel.on("tap", function() {
            self.emit("result", 0);
            self.close();
        });

        cancel.on("click", function() {
            self.emit("result", 0);
            self.close();
        });

        buttonView.addChild(cancel);

        let middleLine = new View();
        middleLine.height = 50;
        middleLine.width = 1;
        middleLine.background = "#efefef";
        buttonView.addChild(middleLine);

        let confirm = new Button();
        confirm.width = 150;
        confirm.height = 50;
        confirm.text = global.getString("button_join");
        confirm.buttonColor = "#ffffff";
        confirm.styleType = Button.StyleType.None;
        confirm.color = "#02c57e";
        confirm.addGestureRecognizer(new TapRecognizer({
            event: "tap"
        }));

        confirm.on("tap", () => {
            self.emit("result", 1, this.mInvatationCode.text);
            self.close();
        });

        confirm.on("click", () => {
            self.emit("result", 1, this.mInvatationCode.text);
            self.close();
        });

        buttonView.addChild(confirm);
        return buttonView;
    }
}
module.exports = INCodeVerifyDialog;
