"use strict";

class TimeUtil {
    static formatChattingTime(time) {
        let second = time % 60;
        let minute = Math.floor(time / 60 % 60);
        let hour = Math.floor(time / 3600);
        return this.formatTime(hour) + ":" + this.formatTime(minute) + ":" + this.formatTime(second);
    }

    static formatTime(time) {
        if (time >= 10) {
            return time.toString();
        } else {
            return "0" + time;
        }
    }
}
module.exports = TimeUtil;
