# wii

Wii provides asynchronous native bindings to the libcwiid C API.

## Contributors
* Created by [Tim Branyen](https://github.com/tbranyen)
* Modified by [Andrew Brampton](http://github.com/bramp)
* Modified by [Dan Yocom](https://github.com/danyocom)
* Modified by [Daniel Huisman](https://github.com/DanielHuisman)

## Building and installing

### Dependencies
To run wii you need Node.js (>=v0.10.0) or io.js (>=v1.0.0), bluez, and libcwiid installed. To run unit tests you will need to have git installed and accessible from your PATH to fetch any vendor/ addons.

It is sufficient enough to rely on the system package manager to install
libcwiid and bluez.  Refer to your distros repository search to find the
correct module names and installation procedure.

### Linux (Ubuntu/Debian)

Ensure you have the dependancies installed

``` bash
$ sudo apt-get install libbluetooth-dev libcwiid-dev
$ sudo npm install -g node-gyp
```

Install wii by from npm:
    
``` bash
$ npm install wii
```

A demo applicaton can be run like so:

``` bash
$ node example/simple/server.js
```

### Apple OS X/Windows via Cygwin
node-wii currently does not run on either Mac OS X or Windows machines.  This
is a problem with `libcwiid`.  A future plan is to fork `libcwiid` and write
support for at least Apple OS X.

## API Example Usage

### Connecting and enabling features

#### Raw API

``` javascript
var wii = require("wii");
var wiimote = new wii.WiiMote();

// You may specify a mac address to your wiimote, or use 00:00:00:00:00
wiimote.connect("00:00:00:00:00", function(err) {
  if( err ) {
    console.log( 'Could not establish connection' );
    return;
  }

  // Enable rumble
  wiimote.rumble(true);

  // Turn on led"s 1 and 3
  wiimote.led(1, true);
  wiimote.led(3, true);

  // Turn off led 3
  wiimote.led(3, false);

  // Get IR Data
  wiimote.ir(true);
  wiimote.on("ir", function(points) {
    for (var p in points)
      console.log("Point", p['x'], p['y'], p['size']);
  });
});
```

### Methods

* connect(address, callback)
* disconnect()
* rumble(flag)
* led(led, flag)
* ir(flag)
* ext(flag)
* acc(flag)

### Events

* connect
* disconnect
* accelerometer
* button
* ir
* status
* nunchuk

## Release information

### v0.1.0 (by DanielHuisman)
* Cleaned up the project
* Added nunchuk support

### v0.0.8 (by danyocom):
* Cleaned up the readme file

### v0.0.7 (by danyocom):
* Disabled console debug output

### v0.0.4 - v0.0.6 (by danyocom):
* Renamed project to node-wii for publishing on npmjs.org
* Fixed pacakge.json to point to the node-wii.js stub instead of erronusly trying to load the binary directly

### v0.0.3 (by bramp):
* Upgraded to support node 0.10.0
* Change build system to node-gyp
* Change event framework from EIO to UV

### v0.0.2 (by bramp):
* Added a new example using socket.io
* Changed the use of cwiid to be truely async

### v0.0.1:
* Some useful methods implemented
* Partial examples in example directory

## Getting involved
If you find this project of interest, please document all issues and fork if
you feel you can provide a patch.

