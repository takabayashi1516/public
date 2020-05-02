
const WsClient = require("websocket").client;

process.env.NODE_EXTRA_CA_CERTS = process.argv[2];
process.env.NODE_TLS_REJECT_UNAUTHORIZED = "0";

var _wss = new WsClient();
_wss.on("connectFailed", function (_err) {
	console.log(_err);
});
_wss.on("connect", function (_conection) {
	console.log("connect");
	_conection.on("close", function () {
		console.log("wss close");
	});
	_conection.on("message", function (_data) {
		console.log(_data);
	});
    _conection.send("hello wss");
});

_wss.connect("wss://192.168.8.10:10010/", "");
