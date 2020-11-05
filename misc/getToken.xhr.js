function getToken(url, room_id) {
    if (url.indexOf("http://") < 0)
        url = "http://" + url + "/createToken";

    var xhr = new XMLHttpRequest();
    xhr.open('POST', url, false);
    var content = '{"role":"presenter","username":"user"';
    if (room_id)
        content += ',"room":"' + room_id + '"';
    content += '}';
    xhr.setRequestHeader("Content-Type", "application/json");
    xhr.setRequestHeader("Accept", "application/json");
    xhr.send(content);
    return xhr.responseText;
}
getToken("10.238.135.167:3001");
