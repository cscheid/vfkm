importScripts("vfkm.js");

var vfkm;
function send(o)
{
    postMessage(JSON.stringify(o));
}

onmessage = function(oEvent) {
    var msg = oEvent.data;
    handle(JSON.parse(msg));
};

function init(url, res, k, lambda) {
    var myRequest = new XMLHttpRequest();
    myRequest.onload = function() {
        vfkm = Module.init(this.responseText, res, k, lambda);
        send("ready");
    };
    myRequest.open("get", url, true);
    myRequest.send();
}

function handle(msg) {
    if (msg === "step") {
        if (vfkm === void 0) {
            send("not ready");
            return;
        }
        send(["stepresult", vfkm.step()]);
    } else if (msg === "get_vector_fields") {
        var v0 = vfkm.get(0);
        var v1 = vfkm.get(1);
        var result = [[[], []], 
                      [[], []]];
        for (var i=0; i<9; ++i) {
            result[0][0][i] = v0.first.getValue(i);
            result[0][1][i] = v0.second.getValue(i);
            result[1][0][i] = v1.first.getValue(i);
            result[1][1][i] = v1.second.getValue(i);
        }
        send(["vector_fields", result]);
    }
};

init("synthetic.txt", 3, 2, 0.05);
