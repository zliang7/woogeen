"use strict";

const Woogeen = require("../utils/woogeen");
const Toast = require("yunos/ui/widget/Toast");

const TAG = "<ChatRoomClient>";

const EVT_USER_JOINED = "user-joined";
const EVT_USER_LEFT = "user-left";
const EVT_MSG_RECVED = "message-received";
const EVT_SERV_DISCONNECTED = "server-disconnected";

class ChatRoomClient {
    init(listener) {
        log.I(TAG, "init======");
        this.listener = listener;

        this.contacts = [];
        this.user = {};
        this.rooms = [];
        this.room = null;
        this.woogeenClient = null;
    }

    createClient(closure) {
        // Signaling server
        const iceServer = {
            urls: ["stun:" + global.config.stun],
            username: "",
            password: ""
        };
        // Media codecs
        const mediaCodec = {
            videoCodec: "H264",
            audioCodec: "OPUS"
        };
        // ConferenceClient configuration
        const clientConfiguration = {
            iceServers: [iceServer],
            mediaCodec: mediaCodec
        };

        this.woogeenClient = new Woogeen.ConferenceClient(clientConfiguration);

        log.I(TAG, "init woogeenClient:", this.woogeenClient);

        this.woogeenClient.addEventListener(EVT_SERV_DISCONNECTED, () => {
            log.I(TAG, "init event " + EVT_SERV_DISCONNECTED + " received");
            if (this.listener) {
                this.listener.emit(EVT_SERV_DISCONNECTED, this.user.name);
            }
        });

        if (closure instanceof Function) {
            closure();
        }
    }

    getWoogeenClient() {
        return this.woogeenClient;
    }

    getListener() {
        return this.listener;
    }

    getRoomCode(callback) {
        if (!this.room) {
            log.E(TAG, "getRoomCode not in chat room!");
            callback("Not in chat room!");
            return;
        }

        let roomCode = this.room._id.substr(-6);
        log.I(TAG, "getRoomCode room code:" + roomCode + ", id:" + this.room._id);

        callback(null, roomCode);
    }

    getRoomByCode(roomCode) {
        for (let index in this.rooms) {
            if (this.rooms[index]._id.substr(-6) === roomCode) {
                return this.rooms[index];
            }
        }

        return null;
    }

    listenMsgEvent() {
        log.I(TAG, "listenMsgEvent 0");
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
                this.contacts.push(user);
            }
            log.I(TAG, "init user joined, get " + this.contacts.length + " contacts:", this.contacts);
            if (this.listener) {
                this.listener.emit(EVT_USER_JOINED, user);
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
                this.listener.emit(EVT_USER_LEFT, user);
            }
        });

        this.woogeenClient.addEventListener(EVT_MSG_RECVED, (userId, msg) => {
            log.I(TAG, "listenMsgEvent event " + EVT_MSG_RECVED + " received: send user id=" + userId + ", message=" + msg);
            if (this.listener) {
                let sender = {};
                for (let index in this.contacts) {
                    if (this.contacts[index]) {
                        if (this.contacts[index].id === userId) {
                            sender.name = this.contacts[index].name;
                            sender.id = this.contacts[index].id;
                            break;
                        }
                    }
                }
                this.listener.emit(EVT_MSG_RECVED, sender, msg);
            }
        });
    }

    createRoom(roomName, callback) {
        // if (this.room) {
        //     if (this.room.name === roomName) {
        //         log.I(TAG, "createRoom room exist already, name:" + this.room.name);
        //         if (callback) {
        //             callback(null, this.room);
        //         }
        //     } else {
        //         log.E(TAG, "createRoom in another chat room already, name:" + this.room.name);
        //         if (callback) {
        //             callback("In another chat room already!");
        //         }
        //     }
        //     return;
        // }

        log.I(TAG, "createRoom room name:" + roomName);
        for (let index in this.rooms) {
            if (this.rooms[index].name === roomName) {
                // this.room = this.rooms[index];
                if (callback) {
                    callback(null, this.rooms[index]);
                }
                return;
            }
        }

        let addr = "";
        if (global.config.mcu.indexOf("http") < 0) {
            addr = "http://" + global.config.mcu + "/createRoom/";
        }

        var url = require("url").parse(addr);
        var body = '{"name":"' + roomName + '"}'; // jshint ignore:line

        log.I(TAG, "createRoom before name:" + roomName);
        this.roomRequest(url, body, "POST")
        .then((roomData) => {
            log.I(TAG, "createRoom data:" + roomData);
            // this.room = JSON.parse(roomData);
            let room = JSON.parse(roomData);
            if (callback) {
                callback(null, room);
            }
        })
        .catch((error) => {
            log.E(TAG, "createRoom error:" + error);
            if (callback) {
                callback(error);
            }
        });
        log.I(TAG, "createRoom after name:" + roomName);
    }

    deleteRoom(callback) {
        if (!this.room || !this.room.name.startsWith(this.user.name) ||
            this.user.role !== "presenter") {
            log.E(TAG, "deleteRoom no room can be delete!");
            if (callback) {
                callback("No room can be delete!");
            }
            return;
        }
        log.I(TAG, "deleteRoom room id:" + this.room._id);

        let addr = "";
        if (global.config.mcu.indexOf("http") < 0) {
            addr = "http://" + global.config.mcu + "/room/" + this.room._id;
        }

        var url = require("url").parse(addr);
        this.roomRequest(url, null, "DELETE")
        .then((result) => {
            log.I(TAG, "deleteRoom result:" + result);
            // this.room = JSON.parse(room);
            for (let index in this.rooms) {
                if (this.rooms[index]._id === this.room._id) {
                    this.rooms.splice(index, 1);
                    break;
                }
            }

            if (callback) {
                callback(null, result);
            }
        })
        .catch((error) => {
            log.E(TAG, "deleteRoom error:" + error);
            if (callback) {
                callback(error);
            }
        });
    }

    joinRoom(roomId, userName, closure, callback) {
        if (this.room) {
            if (this.room._id === roomId) {
                log.I(TAG, "joinRoom in chat room already, room name:" + this.room.name);
                if (callback) {
                    callback(null, this.user);
                }
            } else {
                log.E(TAG, "joinRoom in another chat room name:" + this.room.name);
                if (callback) {
                    callback("In another chat room already!");
                }
            }
            return;
        }
        log.I(TAG, "joinRoom room id:" + roomId + ", user:" + userName);

        Woogeen.getToken(global.config.mcu, userName, roomId)
        .then((token) => {
            log.I(TAG, "joinRoom token: " + token);
            this.createClient(closure);
            if (roomId) {
                this.listenMsgEvent();
            }
            return this.woogeenClient.join(token);
        })
        .catch((error) => {
            log.E(TAG, "joinRoom failed to get token from server: " + error);
            // warning toast
            let tokenWarning = new Toast();
            tokenWarning.text = "Could not connect to server " + global.config.mcu;
            tokenWarning.show();
            if (callback) {
                callback(error);
            }
        })
        .then((user) => {
            log.I(TAG, "joinRoom joined successfully, user: ", user);
            this.user = user;
            if (this.rooms.length === 0) {
                this.getRooms((error, rooms) => {
                    log.I(TAG, "getRooms " + rooms.length + " rooms:", rooms);

                    for (let index in this.rooms) {
                        if (this.rooms[index]._id === roomId || !roomId && this.rooms[index].name === "myRoom") {
                            log.I(TAG, "joinRoom room name:", this.rooms[index]);
                            this.room = this.rooms[index];
                            break;
                        }
                    }
                    if (callback) {
                        callback(this.room ? null : "Invalid room!", user);
                    }
                });
            } else {
                for (let index in this.rooms) {
                    if (this.rooms[index]._id === roomId || !roomId && this.rooms[index].name === "myRoom") {
                        log.I(TAG, "joinRoom room name:", this.rooms[index]);
                        this.room = this.rooms[index];
                        break;
                    }
                }
                if (callback) {
                    callback(this.room ? null : "Invalid room!", user);
                }
            }
        })
        .catch((error) => {
            log.E(TAG, "joinRoom failed to join: " + error);
            if (callback) {
                callback(error);
            }
        });
    }

    leaveRoom(callback) {
        if (!this.room) {
            log.E(TAG, "leaveRoom not in chat room now!");
            callback("Not in chat room now!");
            return;
        }

        this.woogeenClient.leave()
        .then(() => {
            log.I(TAG, "leaveRoom left successfully, user: " + global.config.user + ", room:", this.room);
            let roomName = this.room.name;
            this.room = null;
            this.woogeenClient = null;
            if (callback) {
                callback(null, roomName);
            }
        })
        .catch((error) => {
            log.E(TAG, "leaveRoom failed to leave: " + error);
            if (callback) {
                callback(error);
            }
        });
    }

    getRooms(callback) {
        log.I(TAG, "getRooms");

        let addr = "";
        if (global.config.mcu.indexOf("http") < 0) {
            addr = "http://" + global.config.mcu + "/getRooms/";
        }

        log.I(TAG, "getRooms addr:" + addr);

        var url = require("url").parse(addr);
        this.roomRequest(url)
        .then((rooms) => {
            log.I(TAG, "getRooms data:" + rooms);
            this.rooms = JSON.parse(rooms);
            if (callback) {
                callback(null, this.rooms);
            }
        })
        .catch((error) => {
            log.E(TAG, "getRooms error:" + error);
            if (callback) {
                callback(error);
            }
        });
    }

    getRoom(roomId, callback) {
        log.I(TAG, "getRoom");

        let addr = "";
        if (global.config.mcu.indexOf("http") < 0) {
            addr = "http://" + global.config.mcu + "/getRoom/" + roomId;
        }

        var url = require("url").parse(addr);
        this.roomRequest(url)
        .then((data) => {
            log.I(TAG, "getRoom data:" + data);
            if (callback) {
                callback(null, data);
            }
        })
        .catch((error) => {
            log.E(TAG, "getRoom error:" + error);
            if (callback) {
                callback(error);
            }
        });
    }

    getUsers(callback) {
        if (!this.room) {
            log.E(TAG, "getUsers not in chat room now!");
            callback("Not in chat room now!");
            return;
        }
        log.I(TAG, "getUsers room name:" + this.room.name);

        let addr = "";
        if (global.config.mcu.indexOf("http") < 0) {
            addr = "http://" + global.config.mcu + "/getUsers/" + this.room._id;
        }

        var url = require("url").parse(addr);
        this.roomRequest(url)
        .then((users) => {
            log.I(TAG, "getUsers data:" + users);
            this.contacts = JSON.parse(users);
            if (callback) {
                callback(null, this.contacts);
            }
        })
        .catch((error) => {
            log.E(TAG, "getUsers error:" + error);
            if (callback) {
                callback(error);
            }
        });
    }

    getUser(user, callback) {
        log.I(TAG, "getUser user id:" + user);
        if (!this.room) {
            log.E(TAG, "getUser not in chat room now!");
            callback("Not in chat room now!");
            return;
        }

        let addr = "";
        if (global.config.mcu.indexOf("http") < 0) {
            addr = "http://" + global.config.mcu + "/room/" + this.room._id + "/user/" + user;
        }

        var url = require("url").parse(addr);
        this.roomRequest(url)
        .then((data) => {
            log.I(TAG, "getUsers data:" + data);
            if (callback) {
                callback(null, data);
            }
        })
        .catch((error) => {
            log.E(TAG, "getUsers error:" + error);
            if (callback) {
                callback(error);
            }
        });
    }

    deleteUser(userId, callback) {
        log.I(TAG, "deleteUser user id:" + userId);
        let deleteUser = null;
        for (let index in this.contacts) {
            if (this.contacts[index].id === userId) {
                deleteUser = this.contacts[index];
                break;
            }
        }

        if (!this.room || this.user.role !== "presenter" || !deleteUser) {
            log.E(TAG, "deleteUser no user can be delete!");
            if (callback) {
                callback("No user can be delete!");
            }
            return;
        }

        let addr = "";
        if (global.config.mcu.indexOf("http") < 0) {
            addr = "http://" + global.config.mcu + "/room/" + this.room._id + "/user/" + userId;
        }

        var url = require("url").parse(addr);
        this.roomRequest(url, null, "DELETE")
        .then((result) => {
            log.I(TAG, "deleteUser result:" + result);
            for (let index in this.contacts) {
                if (this.contacts[index]._id === userId) {
                    this.contacts.splice(index, 1);
                    break;
                }
            }

            if (callback) {
                callback(null, result);
            }
        })
        .catch((error) => {
            log.E(TAG, "deleteUser error:" + error);
            if (callback) {
                callback(error);
            }
        });
    }

    sendMsg(msg, receiverId, callback) {
        log.I(TAG, "sendMsg msg:" + msg);
        if (!msg || typeof msg !== "string") {
            if (callback) {
                callback("Invalid message!");
                return;
            }
        }
        this.woogeenClient.send(msg, receiverId)
        .then(() => {
            log.I(TAG, "sendMsg successfully, user: " + receiverId + ", msg:" + msg);
            if (callback) {
                callback(null, msg);
            }
        })
        .catch((error) => {
            log.E(TAG, "sendMsg failed to send msg: " + error);
            if (callback) {
                callback(error);
            }
        });
    }

    roomRequest(url, body, method) {
        let options = {
            protocol: url.protocol,
            host: url.hostname,
            port: url.port,
            path: url.path,
            method: method ? method : "GET",
            headers: {
                Accept: "application/json",
                "Content-Type": "application/json",
                Host: url.host,
                "Content-Length": body ? body.length : 0
            },
            agent: false
        };

        return new Promise((resolve, reject) => {
            var data = "";
            var req = require("http").request(options, (rsp) => {
                rsp.setEncoding("utf8");
                rsp.on("data", (chunk) => {
                    data += chunk;
                    log.I(TAG, "roomRequest get data=====:" + data);
                });
                rsp.on("end", () => {
                    resolve(data);
                });
                rsp.on("error", (error) => {
                    reject(error);
                });
            });
            req.on("error", (error) => {
                reject(error);
            });
            if (body) {
                req.end(body);
            } else {
                req.end();
            }
        });
    }
}

module.exports = ChatRoomClient;
