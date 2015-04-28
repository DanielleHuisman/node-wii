var wii = require("../node-wii");

var wiimote = new wii.WiiMote();

wiimote.connect("00:00:00:00:00:00", function(err) {
	if(err) {
		console.error(err);
	}

	wiimote.rumble(true);
	setTimeout(function() {
		wiimote.rumble(false);
	}, 1000);

	wiimote.ext(true);
	wiimote.on("nunchuk", function(data) {
		console.log(data);
	});
});
