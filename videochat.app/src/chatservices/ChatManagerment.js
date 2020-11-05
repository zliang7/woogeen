"use strict";

const WebrtcConnection = require("./WebrtcConnection");
const ChatRoomClient = require("./ChatRoomClient");
const MgrRoomClient = require("./MgrRoomClient");

const TAG = "<ChatManagerment>";

const MGRUSR_PREFIX = "_####_";

class ChatManagerment {
    init(listener, callback) {
        log.I(TAG, "init");
        this.mgrClient = new MgrRoomClient();
        this.mgrClient.init(listener, callback);

        this.chatClient = new ChatRoomClient();
        this.chatClient.init(listener);
    }

    initWebrtc(preview, rmtSurface) {
        this.webrtcConn = new WebrtcConnection();
        this.webrtcConn.init(this.chatClient, preview, rmtSurface);
    }

    getInviteCode(callback) {
        log.I(TAG, "getInviteCode");
        this.mgrClient.createRoom(global.config.user + "_room", (error, room) => {
            if (!error) {
                log.I(TAG, "getInviteCode join room name:", room);
                this.chatClient.joinRoom(room._id, global.config.user, this.webrtcConn.listenStreamEvents.bind(this.webrtcConn), (error, user) => {
                    log.I(TAG, "getInviteCode join room error:" + error + ", user:" + user.name);
                    if (!error) {
                        this.webrtcConn.connect(/*callback*/);
                        this.chatClient.getRoomCode(callback);
                    } else {
                        if (callback) {
                            callback(error);
                        }
                    }
                });
            } else {
                if (callback) {
                    callback(error);
                }
            }
        });
    }

    inviteContact(contact, callback) {
        if (contact.name === global.config.user) {
            log.E(TAG, "inviteContact can't invite self!");
            if (callback) {
                callback("Can't invite self!");
            }
            return;
        }

        log.I(TAG, "inviteContact contact:" + contact.name);
        this.mgrClient.createRoom(global.config.user + "_room", (error, room) => {
            if (!error) {
                log.I(TAG, "inviteContact join room name:", room);
                this.chatClient.joinRoom(room._id, global.config.user, this.webrtcConn.listenStreamEvents.bind(this.webrtcConn), (error, user) => {
                    log.I(TAG, "inviteContact error:" + error + ", user:" + user.name);
                    if (!error) {
                        this.webrtcConn.connect(/*callback*/);
                        this.mgrClient.inviteContact(contact, room._id, callback);
                    } else {
                        if (callback) {
                            callback(error);
                        }
                    }
                });
            } else {
                if (callback) {
                    callback(error);
                }
            }
        });
    }

    acceptInvitation(callback) {
        log.I(TAG, "acceptInvitation");
        this.mgrClient.acceptInvitation((error, roomId) => {
            if (!error) {
                this.joinChatRoom(roomId, callback);
            } else {
                if (callback) {
                    callback(error);
                }
            }
        });
    }

    refuseInvitation(callback) {
        this.mgrClient.refuseInvitation(callback);
    }

    joinChatRoom(roomId, callback) {
        log.I(TAG, "joinChatRoom room id:" + roomId);
        this.chatClient.joinRoom(roomId, global.config.user, this.webrtcConn.listenStreamEvents.bind(this.webrtcConn), (error, user) => {
            log.I(TAG, "joinChatRoom user:" + user.name);
            if (!error) {
                this.webrtcConn.connect(callback);
            } else {
                if (callback) {
                    callback(error);
                }
            }
        });
    }

    joinChatByInviteCode(inviteCode, callback) {
        let room = this.mgrClient.getRoomByCode(inviteCode);
        if (room) {
            this.joinChatRoom(room._id, callback);
        } else {
            if (callback) {
                callback("Invalid room!");
            }
        }
    }

    leaveChatRoom(callback) {
        log.I(TAG, "leaveChatRoom");
        // Zhenyu
        if (!this.webrtcConn.connected) {
            if (callback) {
                callback(null);
            }
            return;
        }
        this.webrtcConn.disconnect(err => {
            if (err && callback) {
                callback(err);
            }
            if (!err) {
                this.chatClient.leaveRoom((error, roomName) => {
                    if (!error) {
                        log.I(TAG, "leaveChatRoom current room:" + roomName);
                        if (roomName.startsWith(this.chatClient.user.name)) {
                            this.mgrClient.finishChat(callback);
                        } else if (callback) {
                            callback(null);
                        }
                        // this.chatClient.deleteRoom(callback);
                    } else if (callback) {
                        callback(null);
                    }
                });
            }
        });
    }

    deinit() {
        log.I(TAG, "deinit");
        this.leaveChatRoom();
        this.mgrClient.leaveRoom();

        this.webrtcConn.deinit();
        this.webrtcConn = null;
    }

    getContacts(callback) {
        this.mgrClient.getUsers((error, users) => {
            if (!error) {
                let contacts = [];
                for (let index in users) {
                    if (users[index]) {
                        if (users[index].name.startsWith(MGRUSR_PREFIX)) {
                            users[index].name = users[index].name.substr(6, users[index].name.length - 12);
                        }
                        if (users[index].id !== this.mgrClient.user.id) {
                            contacts.push(users[index]);
                        }
                    }
                }
                if (callback) {
                    callback(null, contacts);
                }
            } else {
                if (callback) {
                    callback(error);
                }
            }
        });
    }

    getParticipants(callback) {
        this.chatClient.getUsers(callback);
    }

    getMyself() {
        return this.chatClient.user;
    }

    sendMsg(msg, userId, callback) {
        this.chatClient.sendMsg(msg, userId, callback);
    }

    mute(callback) {
        this.webrtcConn.mute(callback);
    }

    umute(callback) {
        this.webrtcConn.umute(callback);
    }

    playAudio(callback) {
        this.webrtcConn.playAudio(callback);
    }

    pauseAudio(callback) {
        this.webrtcConn.pauseAudio(callback);
    }

    receiveStream(rmtSurface, stream, callback) {
        this.webrtcConn.subscribeStream(rmtSurface, stream, callback);
    }

    removeAllRooms(callback) {
        this.mgrClient.removeAllRooms(callback);
    }
}

module.exports = ChatManagerment;
