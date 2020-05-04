const net = require("net");

var host = "192.168.1.1";
var port = 10010;

var client = new net.Socket();
var no = 0;

client.connect(port, host, function () {
  console.log("connect: " + host + ":" + port);
  client.write("hello tcp " + no.toString() + "\r\n");
});

client.on("data", function (data) {
  console.log(data);
  setTimeout(function () {
    no++;
    client.write("hello tcp " + no.toString() + "\r\n");
  }, 500);
});

client.on("close", function () {
  console.log("disconect");
});
