
var EventEmitter = require("events").EventEmitter;
var WiiMote = require("./build/Release/wii.node").WiiMote;

for(var key in EventEmitter.prototype) {
	WiiMote.prototype[key] = EventEmitter.prototype[key];
}

module.exports = WiiMote;
