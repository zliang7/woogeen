"use strict";

const CompositeView = require("yunos/ui/view/CompositeView");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");
const Button = require("yunos/ui/widget/Button");
const View = require("yunos/ui/view/View");
const PopupMenu = require("yunos/ui/widget/PopupMenu");
const PopupMenuItem = PopupMenu.PopupMenuItem;
const AlertDialog = require("yunos/ui/widget/AlertDialog");
const ClipboardManager = require("yunos/content/ClipboardManager");
const clipboardManager = ClipboardManager.getInstance();
const ClipContent = require("yunos/content/ClipContent");
const INCodeVerifyDialog = require("./INCodeVerifyDialog");
const ViewUtil = require("../utils/ViewUtil");

const EVT_JOIN_CHAT = "join_chat";
const BUTTON_WIDTH = 200;
const BUTTON_HEIGHT = 48;

const TAG = "CallActionsView: KONTAR";
class CallActionsView extends CompositeView {
    constructor(pView) {
        super();
        this.width = pView.width;
        this.height = 60;
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
        let centerLine = new View();
        centerLine.width = 1;
        centerLine.height = 1;
        centerLine.background = "transparent";
        this.addChild(centerLine);

        let connect = new Button();
        connect.width = BUTTON_WIDTH;
        connect.height = BUTTON_HEIGHT;
        connect.text = global.getString("connect_video_chat");
        this.addChild(connect);
        connect.on("tap", () => {
            log.D("KONTAR connect button taped");
            this.showConnectOptions(connect);
        });

        let join = new Button();
        join.width = BUTTON_WIDTH;
        join.height = BUTTON_HEIGHT;
        join.text = global.getString("join_video_chat");
        this.addChild(join);
        join.on("tap", () => {
            log.D("KONTAR join button taped");
            let icvDlg = new INCodeVerifyDialog();
            icvDlg.buttons = [global.getString("button_cancel"), global.getString("button_join")];
            icvDlg.show();
            this.visibility = View.Visibility.None;
            icvDlg.on("result", (index, code) => {
                log.D(TAG, "result", index, code);
                if (index === 0) {
                    this.visibility = View.Visibility.Visible;
                } else if (index === 1) {
                    global.homeController.emit("join_chat", code);
                }
            });
        });
    }

    showConnectOptions(targetView) {
        if (!this.mConnectOptions) {
            let connectOptions = this.mConnectOptions = new PopupMenu();
            connectOptions.width = 220;
            // let item0 = new PopupMenuItem(global.getString("copy_metting_address"));
            // connectOptions.addChild(item0);
            // let item1 = new PopupMenuItem(global.getString("generate_qr_code"));
            // connectOptions.addChild(item1);
            let item2 = new PopupMenuItem(global.getString("generate_invatation_code"));
            connectOptions.addChild(item2);
            connectOptions.on("result", (index) => {
                switch (index) {
                    case 0:
                        this.showInvitationCode();
                        break;
                        // case 1:
                        //     ViewUtil.showToast(global.getString("function_not_open"));
                        //     break;
                        // case 2:
                        //     ViewUtil.showToast(global.getString("function_not_open"));
                        //     break;
                    default:
                        break;
                }
            });
        }
        this.mConnectOptions.show(targetView.left - 25, this.top - 80);
    }

    showInvitationCode() {
        log.I(TAG, "showInvitationCode");
        global.homeController.mChatMgr.getInviteCode((err, code) => {
            log.I(TAG, "getInviteCode error:", err, "code:", code);
            if (err) {
                ViewUtil.showToast(global.getString("get_invite_code_failed"));
                return;
            }
            global.homeController.emit(EVT_JOIN_CHAT);
            this.visibility = View.Visibility.None;
            let dialog = new AlertDialog();
            dialog.message = code;
            dialog.buttons = [global.getString("button_cancel"), global.getString("copy_invatation_code")];
            dialog.show();
            dialog.on("result", (index) => {
                if (index === 1) {
                    let content = ClipContent.newText(code);
                    clipboardManager.setContent(content, function (r, err) {
                        log.D(TAG, "setContent", r, err);
                        ViewUtil.showToast(global.getString("copy_success"));
                    });
                }
            });
        });
    }

}
module.exports = CallActionsView;
