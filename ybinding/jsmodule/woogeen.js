"use strict";

var bindings = nativeLoad("woogeen_jsni.so");

var executor = function(method) {
    var args = Array.prototype.slice.call(arguments, 1);
    method.apply(this, args);
}

function getToken(addr, room_id) {
    if (addr.indexOf("http") < 0)
        addr = "http://" + addr + "/createToken";
    var url = require("url").parse(addr);

    var body = '{"role":"presenter","username":"user"';
    if (room_id)
        body += ',"room":"' + room_id + '"';
    body += '}';

    var options = {
      "protocol": url.protocol,
      "host": url.hostname,
      "port": url.port,
      "path": url.path,
      "method":"POST",
      "headers":{
        "Accept": "application/json",
        "Content-Type": "application/json",
        "Host": url.host,
        "Content-Length":body.length
      },
      "agent":false
    };

    return new Promise((resolve, reject) => {
        var token = '';
        var req = require('http').request(options, (rsp) => {
            rsp.setEncoding('utf8');
            rsp.on('data', (chunk) => { token += chunk;});
            rsp.on('end', () => { resolve(token); });
            rsp.on('error', (error) => { reject(error); });
        });
        req.on('error', (error) => { reject(error); });
        req.end(body);
    });
}

function ConferenceClient(configuration) {
    var client = new bindings.ConferenceClient(configuration);
    Object.defineProperty(this, '_client', {
        'configurable': false,
        'enumerable': false,
        'value': client,
        'writable': false
    });
}
ConferenceClient.prototype = {
    'constructor': ConferenceClient,

    'join': function(token) {
        var exec = executor.bind(this._client, this._client.join, token);
        return new Promise(exec);
    },

    'publish': function(stream) {
        var exec = executor.bind(this._client, this._client.publish, stream);
        return new Promise(exec);
    },

    'subscribe': function(stream, options) {
        var exec = executor.bind(this._client, this._client.subscribe, stream, options);
        return new Promise(exec);
    },

    'unpublish': function(stream) {
        var exec = executor.bind(this._client, this._client.unpublish, stream);
        return new Promise(exec);
    },

    'unsubscribe': function(stream) {
        var exec = executor.bind(this._client, this._client.unsubscribe, stream);
        return new Promise(exec);
    },

    'send': function(message, receiver) {
        var exec = executor.bind(this._client, this._client.send, message, receiver);
        return new Promise(exec);
    },

    'playAudio': function(stream) {
        var exec = executor.bind(this._client, this._client.playAudio, stream);
        return new Promise(exec);
    },

    'pauseAudio': function(stream) {
        var exec = executor.bind(this._client, this._client.pauseAudio, stream);
        return new Promise(exec);
    },

    'playVideo': function(stream) {
        var exec = executor.bind(this._client, this._client.playVideo, stream);
        return new Promise(exec);
    },

    'pauseVideo': function(stream) {
        var exec = executor.bind(this._client, this._client.pauseVideo, stream);
        return new Promise(exec);
    },

    'leave': function() {
        var exec = executor.bind(this._client, this._client.leave);
        return new Promise(exec);
    },

    'getRegion': function(stream) {
        var exec = executor.bind(this._client, this._client.getRegion, stream);
        return new Promise(exec);
    },

    'setRegion': function(stream, regionId) {
        var exec = executor.bind(this._client, this._client.getRegion, stream, regionId);
        return new Promise(exec);
    },

    'getConnectionStats': function(stream, regionId) {
        var exec = executor.bind(this._client, this._client.getConnectionStats, stream);
        return new Promise(exec);
    },

    // EventTarget implementation
    'addEventListener': function(type, listener) {
        this._client.addEventListener(type, listener);
    },
    'removeEventListener': function(type, listener) {
        this._client.removeEventListener(type, listener);
    },
    'dispatchEvent': function(type, event) {
        this._client.dispatchEvent(type, event);
    }
}

var woogeen = {
    ConferenceClient : ConferenceClient,
    LocalCameraStream : bindings.LocalCameraStream,
    getVideoCaptureDevices : bindings.getVideoCaptureDevices,
    getToken : getToken
};

module.exports = woogeen;
