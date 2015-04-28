var wii = require("../../wii");
var app = require("express").createServer();
var io = require("socket.io");
var wiimote = new wii.WiiMote();

var clients = {
	lookup: function(key) {
		var key;
		Object.keys(clients).forEach(function(e) {
			if (clients[e] === key) {
				key = e;
				return false;
			}
		});
		return key;
	},
	all: function() {
		var _ = [];
		Object.keys(clients).forEach(function(e) {
			if (typeof clients[e] !== "function") {
				_.push(e);
			}
		});
		return _;
	}
};

function sendAll(name, args) {
	args.method = name;

	clients.all().forEach(function(e) {
		clients[e].socket.send(args);
	});
}

var socket = io.listen(app);


console.log("Put wiimote in discoverable mode...");
wiimote.connect("00:00:00:00:00:00", function(err) {
	if (err) {
		console.log("Could not establish connection");
		return;
	}
	console.log("connected");

	//wiimote.rumble( true );

	//setTimeout(function() {
	//  wiimote.rumble( false );

	//  wiimote.led(1, true);
	//  wiimote.led(3, true);

	//  setTimeout(function() {
	//    wiimote.led(1, false);
	//    wiimote.led(3, false);
	//    
	//    wiimote.led(2, true);
	//    wiimote.led(4, true);
	//  }, 50);

	//}, 50);

	wiimote.button(true);
	wiimote.on("button", function(err, data) {
		if (err) {
			console.log("Button error");
			return;
		}

		console.log(data);
		sendAll("button", {
			data: data
		});
	});

	// Wait for infrared data
	var bit = 0,
		prev = [];

	wiimote.ir(true);
	wiimote.on("ir", function(err, data) {
		if (err) {
			console.log("IR error");
			return;
		}

		console.log(data);

		if (data["x"] !== 0 && data["y"] !== 0 && bit < 1) {
			bit += 1;
			prev = data;
			//data["x"] = ( data["x"] * -1 ) + 500;
		} else if (data["x"] !== 0 && data["y"] !== 0 && bit === 1) {
			//data["x"] = (data["x"]+prev["y"]) / 2;
			//data["x"] = ( data["x"] * -1 ) + 500;

			//data[1] = (data[1]+prev[1]) / 2;
			prev["x"] = prev["x"] * -1;
			prev["y"] = prev["y"];

			sendAll("ir", {
				data: prev
			});
			bit += 1;
		} else {
			bit = 0;
		}
	});

	//wiimote.ext( true );
	//wiimote.on( "nunchuk", function( err, data ) {
	//  if( err ) { return; }

	//  sendAll( "nunchuk", { data: data } );
	//});

	wiimote.on("accelerometer", function(err, data) {
		if (err) {
			console.log("Accelerometer error");
			return;
		}
		console.log(data);
	});

	socket.on("connection", function(socket) {
		wiimote.rumble(true);
		wiimote.led(1, true);

		setTimeout(function() {
			wiimote.rumble(false);
		}, 500);

		var uuid = socket.sessionId;

		var client = clients[uuid] = {
			socket: socket
		};

		function send(name, args) {
			args.method = name;
			socket.send(args);
		}

		send("uuid", {
			uuid: uuid
		});

		socket.on("message", function(e) {
			var actions = {
				// Some action
			};

			return function(e) {
				e.method && e.method in actions && actions[e.method](e);
			};
		}());

		socket.on("disconnect", function(socket) {
			delete clients[clients.lookup(socket)];
		});
	});

	app.listen(8888);

});