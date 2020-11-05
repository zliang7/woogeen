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

const TAG = "<SettingsDialog>: KONTAR";

const DLG_W = 454;
const DLG_H = 181;

const IPADDRESS = "ipaddress";
const re = /^([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.([0-9]|[1-9]\d|1\d\d|2[0-4]\d|25[0-5]):\d{0,5}$/;

const EVT_SETTING_SERVER_CHANGED = "SettingServerChanged";

class SettingServerDialog extends Dialog {
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
        contentView.height = 121;
        let contentLayout = new RelativeLayout();
        contentLayout.setLayoutParam(0, "align", {
            middle: "parent",
            left: "parent"
        });
        contentLayout.setLayoutParam(0, "margin", {
            left: 40
        });
        contentLayout.setLayoutParam(1, "align", {
            middle: "parent",
            left: {
                target: 0,
                side: "right"
            }
        });

        contentView.layout = contentLayout;
        let title = new TextView();
        title.text = "服务器IP地址";
        title.color = "#333333";
        title.fontSize = 14;
        title.height = 50;
        // title.opacity = 0.56;
        contentView.addChild(title);
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
        inputFieldView.width = 300;
        inputFieldView.height = 60;

        let inputField = new TextField();
        inputField.left = 10;
        inputField.top = 5;
        inputField.width = 280;
        inputField.height = 50;
        inputField.clearable = false;
        inputField.maxLength = 50;

        inputField.color = "#333333";
        inputField.underLine = false;
        inputField.on("textchange", () => {
            log.I("textchanged", inputField.text);
            this[textId] = inputField.text;
        });
        inputField.text = global.config.mcu;
        this.ipAddressField = inputField;

        inputFieldView.addChild(inputField);

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
        buttonView.width = DLG_W;
        buttonView.height = 60;
        buttonView.layout = layoutA;

        let cancel = new Button();
        cancel.width = 227;
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
        confirm.width = 227;
        confirm.height = 50;
        confirm.text = "确定";
        confirm.buttonColor = "#ffffff";
        confirm.styleType = Button.StyleType.None;
        confirm.color = "#02c57e";

        confirm.on("tap", () => {
            let ipAddress = this.ipAddressField.text;
            this.checkIpAddress(ipAddress);
        });

        buttonView.addChild(confirm);
        return buttonView;
    }

    checkIpAddress(ipAddress) {
        let newIpAddress = ipAddress.trim();
        if (newIpAddress === "") {
            ViewUtil.showToast("服务器IP地址不能为空");
            return;
        }
        if (!re.test(newIpAddress)) {
            ViewUtil.showToast("服务器IP地址格式不正确");
        }
        if (newIpAddress === global.config.mcu) {
            this.close();
            return;
        }
        let userInfo = {
            user: global.config.user,
            mcu: this.ipAddressField.text
        };
        StoreUtil.saveData("userInfo", JSON.stringify(userInfo), (err, key) => {
            log.I(TAG, "save user info error:", err, " key:", key);
            global.homeController.emit(EVT_SETTING_SERVER_CHANGED, userInfo.user, this.ipAddressField.text);
            this.close();
        });
    }

    destroy() {
        // destroy your new created objects
        super.destroy();
    }
}

module.exports = SettingServerDialog;
