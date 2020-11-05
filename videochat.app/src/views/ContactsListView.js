"use strict";

var CompositeView = require("yunos/ui/view/CompositeView");
var TextView = require("yunos/ui/view/TextView");
var View = require("yunos/ui/view/View");
var Button = require("yunos/ui/widget/Button");
var CheckBox = require("yunos/ui/widget/CheckBox");
var TapRecognizer = require("yunos/ui/gesture/TapRecognizer");
var BaseAdapter = require("yunos/ui/adapter/BaseAdapter");
var ListView = require("yunos/ui/view/ListView");
var RelativeLayout = require("yunos/ui/layout/RelativeLayout");

const TAG = "ContactsListView: KONTAR";
class ContactsListView extends CompositeView {
    constructor(pView) {
        super();
        this.mPView = pView;
        this.width = global.homeController.width;
        this.height = global.homeController.height;
        this.background = "transparent";
        this.createView();
        this.createEvents();
        this.isShowing = false;
        this.id = "ContactsListView";
        this.visibility = View.Visibility.None;
        this.on("touchstart", () => {
            this.hide();
        });
    }

    show() {
        if (!this.isShowing) {
            this.visibility = View.Visibility.Visible;
            this.isShowing = true;
            global.homeController.showOutsideView(true, this);
        }
    }

    hide() {
        if (this.isShowing) {
            this.clearAll();
            this.visibility = View.Visibility.None;
            this.isShowing = false;
            global.homeController.showOutsideView(false, this);
        }
    }

    hangup() {
        this.hide();
    }

    createView() {
        let container = this.mListContainer = new CompositeView();
        container.width = 280;
        container.height = 367;
        container.top = 60;
        container.left = this.width - 300;
        container.borderRadius = 4;
        container.background = "#ffffff";
        this.addChild(container);
        container.on("touchstart", (e) => {
            e.stopPropagation();
        });

        let mainLayout = new RelativeLayout();
        mainLayout.setLayoutParam(0, "align", {
            top: "parent",
            left: "parent",
            right: "parent"
        });
        mainLayout.setLayoutParam(1, "align", {
            bottom: "parent",
            left: "parent",
            right: "parent"
        });
        mainLayout.setLayoutParam(1, "margin", {
            bottom: 16,
            left: 24,
            right: 24
        });
        mainLayout.setLayoutParam(2, "align", {
            top: {
                target: 0,
                side: "bottom"
            },
            bottom: {
                target: 1,
                side: "top"
            },
            eft: "parent",
            right: "parent"
        });
        mainLayout.setLayoutParam(2, "margin", {
            bottom: 16,
            left: 24,
            right: 24
        });
        // add titleView
        this.mListContainer.layout = mainLayout;
        let titleView = this.createTitleView();
        this.mListContainer.addChild(titleView);
        let footView = this.createFootView();
        this.mListContainer.addChild(footView);
        let constactsList = this.creatListView();
        this.mListContainer.addChild(constactsList);
    }

    createTitleView() {
        let titleView = new CompositeView();
        titleView.height = 50;

        let titleLayout = new RelativeLayout();
        titleLayout.setLayoutParam(0, "align", {
            middle: "parent",
            left: "parent"
        });
        titleLayout.setLayoutParam(0, "margin", {
            left: 24
        });
        titleLayout.setLayoutParam(1, "align", {
            middle: "parent",
            right: "parent"
        });
        titleLayout.setLayoutParam(1, "margin", {
            right: 24
        });
        titleLayout.setLayoutParam(2, "align", {
            middle: "parent",
            right: {
                target: 1,
                side: "left"
            }
        });
        titleLayout.setLayoutParam(2, "margin", {
            right: 12
        });
        titleView.layout = titleLayout;

        let titleName = new TextView();
        titleName.text = "联系人";
        titleName.color = "#999999";
        titleName.fontSize = 14;
        titleView.addChild(titleName);

        let checkAll = new CheckBox();
        checkAll.isLightMode = true;
        checkAll.opacity = 1;
        this._checkAll = checkAll;
        titleView.addChild(checkAll);

        let checkTv = new TextView();
        checkTv.text = "全选";
        checkTv.color = "#999999";
        checkTv.fontSize = 14;
        titleView.addChild(checkTv);
        return titleView;

    }

    creatListView() {
        let listView = new ListView();
        listView.background = "#ffffff";
        listView.height = 245;
        listView.width = 232;

        let adapter = new MyAdapter();
        listView.adapter = adapter;
        this._adapter = adapter;
        this._listView = listView;
        // this.updateList();
        return listView;
    }

    createFootView() {
        let btnFoot = new Button();
        btnFoot.height = 40;
        btnFoot.text = "邀请联系人";
        btnFoot.textColor = "#FFFFFF";
        btnFoot.fontSize = 16;
        this._btnFoot = btnFoot;
        return btnFoot;
    }
    createEvents() {
        let self = this;
        if (this._checkAll) {
            this._checkAll.addGestureRecognizer(new TapRecognizer({
                event: "tap"
            }));
            this._checkAll.on("tap", function () {
                self.checkAllFun();
            });
        }

        if (this._listView) {
            this._listView.on("itemselect", function (itemView, position) {
                self._adapter.data[position].selected = !self._adapter.data[position].selected;
                self._adapter.onDataChange();
                self.updataTitleCheckAll();
            });
        }

        this._btnFoot.on("tap", function () {
            self.invite();
            self.hide();
        });
    }

    updataTitleCheckAll() {
        let count = 0;
        if (this._adapter) {
            for (let i = 0; i < this._adapter.data.length; i++) {
                if (this._adapter.data[i].selected) {
                    count++;
                }
            }
        }
        if (count === this._adapter.data.length) {
            this._checkAll.checked = true;
        } else {
            this._checkAll.checked = false;
        }
    }

    clearAll() {
        this._checkAll.checked = false;
        for (let i = 0; i < this._adapter.data.length; i++) {
            this._adapter.data[i].selected = false;
        }
        this._adapter.onDataChange();
    }

    checkAllFun() {
        let isSelected = this._checkAll.checked;
        for (let i = 0; i < this._adapter.data.length; i++) {
            this._adapter.data[i].selected = isSelected;
        }
        this._adapter.onDataChange();
    }

    invite() {
        let inviteList = [];
        for (let i = 0; i < this._adapter.data.length; i++) {
            if (this._adapter.data[i].selected) {
                inviteList.push(this._adapter.data[i]);
            }
        }
        if (inviteList.length > 0) {
            // this.showToast("邀请的联系人有：" + inviteList.length + ":" + inviteList);
            global.homeController.emit("start_invite", inviteList);
        }
    }

    showToast(msg) {
        let Toast = require("yunos/ui/widget/Toast");
        let toast = new Toast();
        toast.text = msg;
        toast.show();
    }

    updateList() {
        if (global.homeController.mChatMgr) {
            global.homeController.mChatMgr.getContacts((error, contacts) => {
                if (!error) {
                    log.I(TAG, "getContacts contacts:" + contacts);
                    for (let i = 0, len = contacts.length; i < len; i++) {
                        if (contacts[i].id === global.myInfo.id) {
                            contacts.splice(i, 1);
                            break;
                        }
                    }
                    this.contacts = global.contactList = contacts;
                    this._adapter.data = contacts;
                } else {
                    log.E(TAG, "getContacts error:" + error);
                    this._adapter.data = [];
                }
            });
        }
    }

}

class MyAdapter extends BaseAdapter {
    createItem(position, convertView) {
        var itemData = this.data[position];
        if (!convertView) {
            convertView = new CompositeView();
            convertView.width = 232;
            convertView.height = 60;

            let itemLayout = new RelativeLayout();
            itemLayout.setLayoutParam(0, "align", {
                middle: "parent",
                left: "parent"
            });
            itemLayout.setLayoutParam(1, "align", {
                middle: "parent",
                right: "parent"
            });
            convertView.layout = itemLayout;

            let tv = new TextView();
            tv.color = "#333333";
            tv.fontSize = 16;
            convertView.addChild(tv);

            let checkItem = new CheckBox();
            checkItem.isLightMode = true;
            checkItem.enabled = false;
            checkItem.opacity = 1;
            convertView.addChild(checkItem);
            convertView.textView = tv;
            convertView.checkItem = checkItem;
        }
        if (convertView.textView) {
            convertView.textView.text = itemData.name;
        }
        if (convertView.checkItem) {
            convertView.checkItem.checked = Boolean(itemData.selected);
        }
        return convertView;
    }
}
module.exports = ContactsListView;
