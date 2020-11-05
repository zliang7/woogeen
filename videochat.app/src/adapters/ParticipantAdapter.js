"use strict";

const BaseAdapter = require("yunos/ui/adapter/BaseAdapter");
const CompositeView = require("yunos/ui/view/CompositeView");
const TextView = require("yunos/ui/view/TextView");

class ParticipantAdapter extends BaseAdapter {
    createItem(position, convertView) {
        var itemData = this.data[position];
        if (!convertView) {
            convertView = new CompositeView();
            convertView.width = 396;
            convertView.height = 50;
            convertView.background = "transparent";
            var name = new TextView();
            name.left = 24;
            name.height = 50;
            name.verticalAlign = TextView.VerticalAlign.Middle;
            name.color = "white";
            convertView.addChild(name);
            convertView.name = name;
        }
        if (convertView.name) {
            convertView.name.text = itemData.name;
        }
        return convertView;
    }
};
module.exports = ParticipantAdapter;
