var wii = require("../../node-wii");
var express = require("express");
var app = express();
var http = require("http");
var server = http.createServer(app);
var io = require("socket.io").listen(server);

io.set("log level", 2);

server.listen(8888);

// Setup the Express framework
app.get("/", function(req, res) {
	res.sendfile(__dirname + "/client.html");
});

app.use(express.static(__dirname + "/static"));

var wiimote = new wii.WiiMote();

console.log("Put wiimote in discoverable mode...");
wiimote.connect("00:00:00:00:00:00", function(err) {
	if (err) {
		console.log("Could not establish connection");
		return;
	}
	console.log("wiimote connected");
	console.log("point your browser at http://localhost:8888");

	wiimote.on("button", function(data) {
		io.sockets.emit("button", data);
	});

	wiimote.on("ir", function(data) {
		//console.log("ir", data);

		function compareObjects(x, y) {
			if (typeof x != typeof y)
				return false;

			for (var propertyName in x) {
				if (x[propertyName] !== y[propertyName]) {
					return false;
				}
			}
			return true;
		}

		// Surpress the same object
		if (!compareObjects(data, this.lastIr)) {
			io.sockets.emit("ir", data);
			this.lastIr = data;
		}
	});

	wiimote.on("accelerometer", function(data) {
		io.sockets.emit("accelerometer", data);
	});

	wiimote.on("status", function(data) {
		io.sockets.emit("status", data);
	});

	wiimote.on("connect", function(data) {
		io.sockets.emit("connect", data);
	});

	wiimote.on("disconnect", function(data) {
		io.sockets.emit("disconnect", data);
	});

	// Turn on reporting for the following. Each one consumes more battery, so only use when needed
	wiimote.acc(false);
	wiimote.button(true);
	wiimote.ext(false);
	wiimote.ir(true);

	io.sockets.emit("wiimote_connected");
});

io.sockets.on("connection", function(socket) {
	console.log("client connect");
	/*
	  socket.on( "toggle", function( data ) {
	    var sensor = data["sensor"];
	    var value  = data["value"];

		  if (sensor == "rumble") {
			  // Special case. Disable this in 100ms
			  wiimote.rumble( true );

	          setTimeout(function() {
	            wiimote.rumble( false );
	          }, 100);

		  } else if (sensor == "ir") {
			  wiimote.ir( value );

		  } else if (sensor == "button") {
			  wiimote.button( value );

		  } else if (sensor == "acc") {
			  wiimote.acc( value );
		  }
		  
		  // Read the state and update all
		  var state = {
	        rumble: wiimote.rumble(),
	        ir:     wiimote.ir(),
	        button: wiimote.button(),
	        acc:    wiimote.acc(),
		  }
		  io.sockets.emit("toggle", state);
	  });
	*/
});