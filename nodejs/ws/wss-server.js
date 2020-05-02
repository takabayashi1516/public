const https = require("https");
const wssServer = require("ws").Server;
const fs = require("fs");

const argv = process.argv;

var options = {
	cert: fs.readFileSync(argv[2]),
	key: fs.readFileSync(argv[3]),
	ca: fs.readFileSync(argv[4])
};

var https_server = https.createServer(options);
var wss = new wssServer({ server: https_server });

https_server.listen(10010);
wss.on("connection", function (_ws, _rqst) {
	_ws.on("close", function() {
		console.log("ws close");
	});

	_ws.on("message", function (_data) {
		_ws.send(_data);
	});
});
