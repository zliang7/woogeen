"use strict";

const BaseAdapter = require("yunos/ui/adapter/BaseAdapter");
const TextView = require("yunos/ui/view/TextView");
const CompositeView = require("yunos/ui/view/CompositeView");
const RelativeLayout = require("yunos/ui/layout/RelativeLayout");

const TAG = "<ChatListAdapter>: KONTAR";
class ChatListAdapter extends BaseAdapter {
    constructor() {
        super();
    }

    createItem(position, convertView) {
        let itemData = this.data[position];
        let viewType = this.getViewType(itemData);
        if (!convertView) {
            convertView = this.getConvertView(viewType);
        }
        if (convertView.content) {
            convertView.content.width = 300;
            convertView.name.text = itemData.name;
            convertView.content.text = itemData.message;
            convertView.content.width = convertView.content.contentWidth + 24;
            convertView.content.height = convertView.content.contentHeight + 24;
            convertView.height = convertView.content.height + 40;
        }
        return convertView;
    }

    getViewType(msg) {
        log.D(TAG, "getViewType", global.myInfo.id, msg.id);
        return global.myInfo.id === msg.id ? 0 : 1;
    }

    getConvertView(type) {
        let convertView = new CompositeView();
        convertView.width = 396;
        convertView.height = 100;
        convertView.background = "transparent";
        if (!this.mOtherLayout) {
            let layout = this.mOtherLayout = new RelativeLayout();
            layout.setLayoutParam(0, "align", {
                left: "parent",
                top: "parent"
            });
            layout.setLayoutParam(0, "margin", {
                left: 24
            });
            layout.setLayoutParam(1, "align", {
                left: "parent",
                top: {
                    target: 0,
                    side: "bottom"
                }
            });
            layout.setLayoutParam(1, "margin", {
                left: 24,
                top: 12
            });
        }
        if (!this.mMyLayout) {
            let layout = this.mMyLayout = new RelativeLayout();
            layout.setLayoutParam(0, "align", {
                right: "parent",
                top: "parent"
            });
            layout.setLayoutParam(0, "margin", {
                right: 24
            });
            layout.setLayoutParam(1, "align", {
                right: "parent",
                top: {
                    target: 0,
                    side: "bottom"
                }
            });
            layout.setLayoutParam(1, "margin", {
                right: 24,
                top: 12
            });
        }

        let name = new TextView();
        name.color = "white";
        name.text = "Tom";
        name.fontSize = 16;
        convertView.addChild(name);

        let content = new TextView();
        content.color = "white";
        content.width = 300;
        content.fontSize = 16;
        content.borderRadius = 6;
        content.multiLine = true;
        content.paddingLeft = 12;
        content.paddingRight = 12;
        content.paddingTop = 12;
        content.paddingBottom = 12;
        content.verticalAlign = TextView.VerticalAlign.Middle;
        convertView.addChild(content);
        if (type === 0) {
            convertView.layout = this.mMyLayout;
            content.background = "#0682C8";
        } else {
            convertView.layout = this.mOtherLayout;
            content.background = "#333333";
        }
        convertView.name = name;
        convertView.content = content;
        return convertView;
    }
}
module.exports = ChatListAdapter;
