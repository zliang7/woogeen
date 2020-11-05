"use strict";

const TapRecognizer = require("yunos/ui/gesture/TapRecognizer");
const LongPressRecognizer = require("yunos/ui/gesture/LongPressRecognizer");
const Toast = require("yunos/ui/widget/Toast");
const View = require("yunos/ui/view/View");
const willShowLog = false;
const TAG = "ViewUtil: KONTAR ";
class ViewUtil {
    static addTapListener(view, callback) {
        view.addGestureRecognizer(new TapRecognizer({
            event: "tap"
        }));
        view.on("tap", (e) => {
            callback(e);
        });
        view.on("click", (e) => {
            callback(e);
        });
    }

    static addLongPressListener(view, callback) {
        view.addGestureRecognizer(new LongPressRecognizer());
        view.on("longpress", (e) => {
            callback(e);
        });
    }

    static getSplitLine(config) {
        let line = new View();
        line.width = config.width;
        line.height = 1;
        line.background = "#FFFFFF";
        line.opacity = 0.2;
        return line;
    }

    static showToast(str) {
        if (!this.toast) {
            this.isShowToast = true;
            this.toast = new Toast();
            this.toast.text = str;
            this.toast.show();
            setTimeout(() => {
                this.isShowToast = false;
            }, 3000);
        }
        else {
            if (!this.isShowToast) {
                this.isShowToast = true;
                this.toast.text = str;
                this.toast.show();
                setTimeout(() => {
                    this.isShowToast = false;
                }, 3000);
            }
        }
    }

    static showLog(willShow, str) {
        if (willShow) {
            console.log(str);
        }
    }
}
module.exports = ViewUtil;
