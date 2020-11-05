"use strict";

const CompositeView = require("yunos/ui/view/CompositeView");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");
const ImageView = require("yunos/ui/view/ImageView");
const PopupMenu = require("yunos/ui/widget/PopupMenu");
const PopupMenuItem = PopupMenu.PopupMenuItem;
const ContactsListView = require("../views/ContactsListView");
const SettingServerDialog = require("../views/SettingServerDialog");
const ViewUtil = require("../utils/ViewUtil");

const EVT_CONTACT_JOINED = "contact-joined";
const EVT_CONTACT_LEFT = "contact-left";

const TAG = "ContactController: KONTAR ";
class ContactController extends CompositeView {
    constructor(pView) {
        super();
        this.width = pView.width;
        this.height = 48;
        this.clipBound = false;
        this.createLayout();
        this.initView();
        this.registerListener();
    }

    createLayout() {
        this.mRootLayout = new RelativeLayout();
        this.layout = this.mRootLayout;
        this.mRootLayout.setLayoutParam(0, "align", {
            middle: "parent",
            right: "parent"
        });
        this.mRootLayout.setLayoutParam(0, "margin", {
            right: 24
        });

        this.mRootLayout.setLayoutParam(1, "align", {
            middle: "parent",
            right: {
                target: 0,
                side: "left"
            }
        });
        this.mRootLayout.setLayoutParam(1, "margin", {
            right: 24
        });
    }

    initView() {
        let moreSettingIcon = new ImageView();
        moreSettingIcon.width = 30;
        moreSettingIcon.height = 30;
        moreSettingIcon.scaleType = ImageView.ScaleType.Centercrop;
        moreSettingIcon.src = global.getImageSrc("icon_more.png");
        this.addChild(moreSettingIcon);
        ViewUtil.addTapListener(moreSettingIcon, () => {
            log.D(TAG, "moreSettingIcon.show");
            this.showSettingPopupMenu();
        });

        let contactsIcon = new ImageView();
        contactsIcon.width = 30;
        contactsIcon.height = 30;
        contactsIcon.src = global.getImageSrc("icon_videochat_add.png");
        this.addChild(contactsIcon);
        ViewUtil.addTapListener(contactsIcon, () => {
            log.D(TAG, "mContactList.show");
            this.mContactList.show();
        });

        let contactList = this.mContactList = new ContactsListView(this);
        this.addChild(contactList);
    }

    registerListener() {
        global.homeController.on(EVT_CONTACT_JOINED, (user) => {
            log.I(TAG, EVT_CONTACT_JOINED, user);
            this.updateContactList();
        });
        global.homeController.on(EVT_CONTACT_LEFT, (user) => {
            log.I(TAG, EVT_CONTACT_LEFT, user);
            this.updateContactList();
        });
    }

    onUpdate() {
        this.mContactList.updateList();
    }

    hangup() {
        this.mContactList.hangup();
    }

    switchToChatting(status) {
        if (status) {
            this.mRootLayout.setLayoutParam(0, "margin", {
                right: -40
            });
        } else {
            this.mRootLayout.setLayoutParam(0, "margin", {
                right: 24
            });
        }
    }

    updateContactList() {
        this.mContactList.updateList();
    }

    showSettingPopupMenu() {
        if (!this.mMoreOptions) {
            let mMoreOptions = this.mMoreOptions = new PopupMenu();
            mMoreOptions.width = 220;
            let item0 = new PopupMenuItem("配置服务器");
            mMoreOptions.addChild(item0);
            mMoreOptions.on("result", (index) => {
                switch (index) {
                    case 0:
                        this.showSettingServerDialog();
                        break;
                    default:
                        break;
                }
            });
        }
        this.mMoreOptions.show(this.width - 25, 65);
    }

    showSettingServerDialog() {
        log.I(TAG, "showSettings");
        let settingServerDialog = new SettingServerDialog(this);
        settingServerDialog.show();
    }
}
module.exports = ContactController;
