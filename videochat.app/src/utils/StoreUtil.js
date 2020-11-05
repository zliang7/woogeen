"use strict";

const KVStore = require("yunos/storage/KVStore");

const TAG = "StoreUtil: KONTAR";
class StoreUtil {

    static getData(key, callback) {
        log.D(TAG, "getData", key);
        KVStore.getInstance((error, instance) => {
            if (error === null) {
                instance.get(key, (err, value) => {
                    log.D(TAG, "get err:", err, " value:", value);
                    callback(err, value);
                });
            } else {
                callback(error);
            }
        });
    }

    static saveData(key, value, callback) {
        log.D(TAG, "saveData", key, value);
        KVStore.getInstance((error, instance) => {
            if (!error) {
                instance.set(key, value, (err, setKey) => {
                    log.D(TAG, "set err:", err, " key:", setKey);
                    callback(err, setKey);
                });
            } else {
                callback(err);
            }
        });
    }
}

module.exports = StoreUtil;
