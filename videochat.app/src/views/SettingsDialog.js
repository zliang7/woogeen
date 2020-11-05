"use strict";

const Dialog = require("yunos/ui/widget/Dialog");
const CompositeView = require("yunos/ui/view/CompositeView");
const ColumnLayout = require("yunos/ui/layout/ColumnLayout");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");
const TextView = require("yunos/ui/view/TextView");
const View = require("yunos/ui/view/View");
const TextField = require("yunos/ui/view/TextField");
const Button = require("yunos/ui/widget/Button");
const TapRecognizer = require("yunos/ui/gesture/TapRecognizer");
const StoreUtil = require("../utils/StoreUtil");
const ViewUtil = require("../utils/ViewUtil");
const re = /^([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5]):\d{0,5}$/;
const TAG = "<SettingsDialog>: KONTAR";

const DLG_W = 400;
const DLG_H = 255;

const USERNAME = "username";
const IPADDRESS = "ipaddress";

const EVT_SETTINGS_CHANGED = "settingsChanged";

class SettingsDialog extends Dialog {
    constructor(handler) {
        super();
        this.width = DLG_W;
        this.height = DLG_H;
        this.radius = 99;
        this.handler = handler;
        this.initView();
    }

    initView() {
        let layout = new ColumnLayout();
        layout.defaultLayoutParam = {
            align: ColumnLayout.Align.FillWidth
        };

        let layoutView = new CompositeView();
        layoutView.width = DLG_W;
        layoutView.height = DLG_H;
        layoutView.layout = layout;
        let contentView = this.contentView();

        layoutView.addChild(contentView);
        layoutView.addChild(this.underline());

        let buttonView = this.buttonView();
        layoutView.addChild(buttonView);

        this.addChild(layoutView);
    }
    contentView() {
        let contentView = new CompositeView();
        contentView.width = DLG_W;
        contentView.height = DLG_H - 50;
        let contentViewLayout = new RelativeLayout();
        contentViewLayout.setLayoutParam(0, "align", {
            top: "parent"
        });
        contentViewLayout.setLayoutParam(0, "margin", {
            top: 40
        });
        contentViewLayout.setLayoutParam(1, "align", {
            top: {
                target: 0,
                side: "bottom"
            }
        });
        contentViewLayout.setLayoutParam(1, "margin", {
            top: 24
        });
        contentView.layout = contentViewLayout;
        let userEditView = this.inputView(USERNAME);
        contentView.addChild(userEditView);

        let addrEditView = this.inputView(IPADDRESS);
        contentView.addChild(addrEditView);

        return contentView;

    }

    underline() {
        let underline = new View();
        underline.height = 1;
        underline.width = DLG_W;
        underline.background = "#efefef";

        return underline;
    }

    inputView(textId) {
        let inputFieldView = new CompositeView();
        inputFieldView.width = DLG_W;
        inputFieldView.height = 50;
        let inputFieldViewLayout = new RelativeLayout();
        inputFieldViewLayout.setLayoutParam(0, "align", {
            middle: "parent",
            left: "parent"
        });
        inputFieldViewLayout.setLayoutParam(0, "margin", {
            left: 40
        });
        inputFieldViewLayout.setLayoutParam(1, "align", {
            middle: "parent",
            left: {
                target: 0,
                side: "right"
            }
        });
        inputFieldViewLayout.setLayoutParam(1, "margin", {
            left: 12
        });
        inputFieldView.layout = inputFieldViewLayout;

        let title = new TextView();
        title.color = "#333333";
        title.fontSize = 14;
        title.height = 50;
        inputFieldView.addChild(title);

        let inputNameField = new TextField();
        inputNameField.left = 10;
        inputNameField.top = 5;
        inputNameField.width = 280;
        inputNameField.height = 50;
        inputNameField.clearable = false;
        inputNameField.maxLength = 50;

        inputNameField.color = "#333333";
        inputNameField.underLine = false;
        if (textId === USERNAME) {
            title.text = "用户名";
            this.userNameField = inputNameField;
        } else if (textId === IPADDRESS) {
            title.text = "IP地址";
            this.ipAddressField = inputNameField;
        }
        inputFieldView.addChild(inputNameField);
        return inputFieldView;
    }

    buttonView() {
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
        buttonView.height = 60;
        buttonView.layout = layoutA;

        let cancel = new Button();
        cancel.width = 200;
        cancel.height = 50;
        cancel.buttonColor = "#ffffff";
        cancel.styleType = Button.StyleType.None;
        cancel.text = "取消";
        cancel.color = "#000000aa";
        cancel.addGestureRecognizer(new TapRecognizer({
            event: "tap"
        }));

        cancel.on("tap", () => {
            this.close();
        });

        cancel.on("click", () => {
            this.close();
        });

        buttonView.addChild(cancel);

        let middleLine = new View();
        middleLine.height = 50;
        middleLine.width = 1;
        middleLine.opacity = 0.66;
        middleLine.background = "#efefef";
        buttonView.addChild(middleLine);

        let confirm = new Button();
        confirm.width = 200;
        confirm.height = 50;
        confirm.text = "确定";
        confirm.buttonColor = "#ffffff";
        confirm.styleType = Button.StyleType.None;
        confirm.color = "#02c57e";

        confirm.on("tap", () => {
            this.checkInput();
        });
        buttonView.addChild(confirm);
        return buttonView;
    }
    checkInput() {
        let userName = this.userNameField.text.trim();
        let ipAddress = this.ipAddressField.text.trim();
        if (userName === "" || ipAddress === "") {
            ViewUtil.showToast("输入不能为空");
        }
        if (!re.test(ipAddress)) {
            ViewUtil.showToast("IP地址格式不正确");
        } else {
            let userInfo = {
                user: this.userNameField.text,
                mcu: this.ipAddressField.text
            };
            StoreUtil.saveData("userInfo", JSON.stringify(userInfo), (err, key) => {
                log.I(TAG, "save user info error:", err, " key:", key);
                this.handler.emit(EVT_SETTINGS_CHANGED, this.userNameField.text, this.ipAddressField.text);
                this.close();
            });
        }
    }

    destroy() {
        // destroy your new created objects
        super.destroy();
    }
}

module.exports = SettingsDialog;
