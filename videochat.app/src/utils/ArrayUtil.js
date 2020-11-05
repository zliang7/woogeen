"use strict";

class ArrayUtil {
    static deleteEle(array, time) {
        if (array.length > 0) {
            let index = array.indexOf(time);
            if (index > -1) {
                array.splice(index, 1);
            }
        }
        return array;
    }

    static isContainAll(targetArr, itemArr) {
        let isContainAll = true;
        if (targetArr.length > 0) {
            itemArr.forEach((item) => {
                if (targetArr.indexOf(item.time) === -1) {
                    isContainAll = false;
                    return;
                }
            });
        }
        else {
            isContainAll = false;
        }
        return isContainAll;
    }

    static filtrateArray(targetArray, filter) {
        let newArray = [];
        for (let i = 0, len = targetArray.length; i < len; i++) {
            let text = targetArray[i].text;
            if (text.indexOf(filter) !== -1) {
                newArray.push(targetArray[i]);
            }
        }
        return newArray;
    }
}
module.exports = ArrayUtil;
