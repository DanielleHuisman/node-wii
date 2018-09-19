var EventEmitter = require("events").EventEmitter;
var wii = require("./build/Release/wii.node");

for(var key in EventEmitter.prototype) {
	wii.WiiMote.prototype[key] = EventEmitter.prototype[key];
}

module.exports = wii;
