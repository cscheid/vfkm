var worker = new Worker("vfkm_load.js");

function send(o)
{
    worker.postMessage(JSON.stringify(o));
}
function recv(event) {
    var msg = JSON.parse(event.data);
    handle(msg);
}
worker.addEventListener("message", recv);

function handle(msg) {
    if (msg === "ready") {
        send("step");
    } else if (msg[0] === "stepresult") {
        if (msg[1].first > 0) {
            send("step");
        } else {
            send("get_vector_fields");
        }
    } else if (msg[0] === "vector_fields") {
        console.log(msg[1]);
    }
}
// worker.postMessage("");
