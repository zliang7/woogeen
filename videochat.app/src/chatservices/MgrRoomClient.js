"use strict";

const ChatRoomClient = require("./ChatRoomClient");
const util = require("util");

const TAG = "<MgrRoomClient>";

const ACTION_TYPE_INVITE = "INV";
const ACTION_TYPE_REFUSE = "REF";
const ACTION_TYPE_ACCEPT = "ACP";
const ACTION_TYPE_FINISH = "FIN";
const ACTION_TYPE_ERROR = "ERR";

const EVT_USER_JOINED = "user-joined";
const EVT_USER_LEFT = "user-left";
const EVT_CONTACT_JOINED = "contact-joined";
const EVT_CONTACT_LEFT = "contact-left";
const EVT_MSG_RECVED = "message-received";
const EVT_CHAT_INVITE = "chat-invite";
const EVT_CHAT_ACCEPTED = "chat-accepted";
const EVT_CHAT_REFUSED = "chat-refused";
const EVT_CHAT_ENDED = "chat-ended";
const EVT_CHAT_ERROR = "chat-error";

// "<<ACTION::id::type::from::to::data>>"
const ACTION_FORMAT = "<<ACTION::%d::%s::%s::%s::%s>>";

const MGRUSR_PREFIX = "_####_";
const MGRUSR_SUFFIX = "_@@@@_";

class MgrRoomClient extends ChatRoomClient {
    init(listener, callback) {
        log.I(TAG, "init======");
        super.init(listener);

        this.sndReqId = 0;
        this.sndRspId = 0;
        this.actionSent = {};

        this.rcvReqId = 0;
        this.rcvRspId = 0;
        this.actionRecv = {};

        // setTimeout(() => {
        this.getRooms((error, rooms) => {
            log.I(TAG, "getRooms rooms:", rooms);
            if (!error || !rooms || rooms.length === 0) {
                this.joinRoom(null, MGRUSR_PREFIX + global.config.user + MGRUSR_SUFFIX, this.listenMsgEvent.bind(this), (error, user) => {
                    log.I(TAG, "joinRoom user:", user);
                    if (callback) {
                        callback(error);
                    }
                });
            } else {
                log.E(TAG, "getRooms failed, can't join mgr room!");
                if (callback) {
                    callback(error);
                }
            }
        });
        // }, 3000);
    }

    listenMsgEvent() {
        log.I(TAG, "set listenMsgEvent");
        this.woogeenClient.addEventListener(EVT_USER_JOINED, (user) => {
            log.I(TAG, "init event " + EVT_USER_JOINED + " received: id=" + user.id + ", name=" + user.name + ", role=" + user.role);
            let i = 0;
            for (let index in this.contacts) {
                if (this.contacts[index]) {
                    if (this.contacts[index].id === user.id) {
                        break;
                    }
                    i++;
                }
            }
            if (i === this.contacts.length) {
                if (user.name.startsWith(MGRUSR_PREFIX)) {
                    user.name = user.name.substr(6, user.name.length - 12);
                }
                this.contacts.push(user);
            }
            log.I(TAG, "init user joined, get " + this.contacts.length + " contacts:", this.contacts);
            if (this.listener) {
                this.listener.emit(EVT_CONTACT_JOINED, user);
            }
        });
        this.woogeenClient.addEventListener(EVT_USER_LEFT, (user) => {
            log.I(TAG, "init event " + EVT_USER_LEFT + " received: id=" + user.id + ", name=" + user.name + ", role=" + user.role);
            for (let index in this.contacts) {
                if (this.contacts[index].id === user.id) {
                    this.contacts.splice(index, 1);
                    break;
                }
            }
            log.I(TAG, "init user left, get " + this.contacts.length + " contacts:", this.contacts);
            if (this.listener) {
                this.listener.emit(EVT_CONTACT_LEFT, user);
            }
        });

        this.woogeenClient.addEventListener(EVT_MSG_RECVED, (userId, msg) => {
            log.I(TAG, "listenMsgEvent event " + EVT_MSG_RECVED + " received: send user id=" + userId + ", message=" + msg);
            if (msg.startsWith("<<ACTION::") && msg.endsWith(">>")) {
                this.onNewAction(userId, msg);
            } else if (this.listener) {
                this.listener.emit(EVT_MSG_RECVED, userId, msg);
            }
        });
    }

    makeAction(id, actionType, to, data) {
        let sender = {};
        let username = this.user.name;
        if (this.user.name.startsWith(MGRUSR_PREFIX)) {
            username = this.user.name.substr(6, this.user.name.length - 12);
        }
        sender.name = username;
        sender.id = this.user.id;
        log.I(TAG, "makeAction myself:", this.user);
        let receiver = {};
        if (to) {
            receiver.name = to.name;
            receiver.id = to.id;
        }

        let action = util.format(ACTION_FORMAT, id, actionType, JSON.stringify(sender), JSON.stringify(receiver), data);
        log.I(TAG, "makeAction action:" + action);

        return action;
    }

    sendAction(action, receiverId, callback) {
        this.sendMsg(action, receiverId, callback);
    }

    errorAction(errorMsg) {
        let action = this.makeAction(this.actionRecv.id, ACTION_TYPE_ERROR, this.actionRecv.sender, errorMsg);
        log.I(TAG, "errorAction action:" + action);
        this.sendAction(action, this.actionRecv.sender, (error) => {
            log.E(TAG, "errorAction error:" + error);
        });
    }

    inviteContact(user, roomId, callback) {
        if (user.id === this.user.id) {
            log.E(TAG, "inviteContact can't invite self!");
            callback("Can't invite self!");
            return;
        }

        let action = this.makeAction(this.sndReqId, ACTION_TYPE_INVITE, user, roomId);
        this.sendAction(action, user.id, (error) => {
            if (!error) {
                this.actionSent.id = this.sndReqId;
                this.actionSent.type = ACTION_TYPE_INVITE;
                this.actionSent.receiver = user;
                this.actionSent.data = roomId;

                this.sndReqId++;
            }
            if (callback) {
                callback(error);
            }
        });
    }

    acceptInvitation(callback) {
        log.I(TAG, "acceptInvitation rcvReqId:" + this.rcvReqId + ", rcvRspId:" + this.rcvRspId);
        if (this.rcvReqId - this.rcvRspId !== 1) {
            callback("Invalid ACCEPT action!");
            return;
        }

        let action = this.makeAction(this.actionRecv.id, ACTION_TYPE_ACCEPT, this.actionRecv.sender, this.actionRecv.data);
        this.sendAction(action, this.actionRecv.sender.id, (error) => {
            if (!error) {
                this.rcvRspId++;
            }
            if (callback) {
                callback(error, this.actionRecv.data);
            }
        });
    }

    refuseInvitation(callback) {
        log.I(TAG, "refuseInvitation rcvReqId:" + this.rcvReqId + ", rcvRspId:" + this.rcvRspId);
        if (this.rcvReqId - this.rcvRspId !== 1) {
            callback("Invalid REFUSE action!");
            return;
        }

        let action = this.makeAction(this.actionRecv.id, ACTION_TYPE_REFUSE, this.actionRecv.sender, this.actionRecv.data);
        this.sendAction(action, this.actionRecv.sender.id, (error) => {
            if (!error) {
                this.rcvRspId++;
            }
            if (callback) {
                callback(error);
            }
        });
    }

    finishChat(callback) {
        log.I(TAG, "finishChat");
        let action = this.makeAction(this.sndReqId, ACTION_TYPE_FINISH, null, "FINISHED");
        this.sendAction(action, null, (error) => {
            if (!error) {
                this.actionSent.id = this.sndReqId;
                this.actionSent.type = ACTION_TYPE_FINISH;
                this.actionSent.receiver = null;
                this.actionSent.data = "FINISHED";

                this.sndReqId++;
            }
            if (callback) {
                callback(error);
            }
        });
    }

    onNewAction(senderId, action) {
        log.I(TAG, "onNewAction action:" + action);
        let aParams = action.split("::");
        let id = aParams[1];
        let type = aParams[2];
        let sender = JSON.parse(aParams[3]);
        // let receiver = aParams[4];
        let data = aParams[5].split(">>")[0];

        if (type === ACTION_TYPE_INVITE) {
            if (this.rcvReqId !== this.rcvRspId) {
                this.errorAction("Peer is busying!");
            } else {
                this.actionRecv.id = id;
                this.actionRecv.type = type;
                this.actionRecv.sender = sender;
                this.actionRecv.data = data;

                if (this.listener) {
                    this.listener.emit(EVT_CHAT_INVITE, this.actionRecv.sender, this.actionRecv.data);
                }
                this.rcvReqId++;
            }
        } else if (type === ACTION_TYPE_FINISH) {
            this.actionRecv.id = id;
            this.actionRecv.type = type;
            this.actionRecv.sender = sender;
            this.actionRecv.data = data;

            if (this.listener) {
                this.listener.emit(EVT_CHAT_ENDED, this.actionRecv.sender, this.actionRecv.data);
            }
            this.rcvReqId++;
            this.rcvRspId = this.rcvReqId;
        } else {
            if (this.rcvReqId === this.rcvRspId || this.actionSent.receiver.id !== sender.id) {
                // dismatch error, do nothing
                log.I(TAG, "onNewAction dismatch, do nothing!");
            } else {
                if (this.listener) {
                    if (type === ACTION_TYPE_ACCEPT) {
                        this.listener.emit(EVT_CHAT_ACCEPTED, sender);
                    } else if (type === ACTION_TYPE_ERROR) {
                        this.listener.emit(EVT_CHAT_ERROR, data);
                    } else {
                        this.listener.emit(EVT_CHAT_REFUSED, sender);
                    }
                }
                this.sndRspId++;
            }
        }
    }

    removeAllRooms(callback) {
        log.I(TAG, "removeAllRooms rooms count:" + (this.rooms.length - 1));
        let i = 0;
        let handleResult = (result) => {
            log.I(TAG, "removeAllRooms room" + i + " result:" + result);
            this.rooms.splice(i, 1);

            if (callback) {
                callback(null, result);
            }
        };
        let handleError = (error) => {
            log.E(TAG, "removeAllRooms error:" + error);
            if (callback) {
                callback(error);
            }
        };
        for (let index = this.rooms.length - 1; index >= 0; index--) {
            if (this.rooms[index].name !== "myRoom") {
                let addr = "";
                if (global.config.mcu.indexOf("http") < 0) {
                    addr = "http://" + global.config.mcu + "/room/" + this.rooms[index]._id;
                }

                i = index;
                var url = require("url").parse(addr);
                this.roomRequest(url, null, "DELETE")
                .then(handleResult)
                .catch(handleError);
            }
        }
    }
}

module.exports = MgrRoomClient;
