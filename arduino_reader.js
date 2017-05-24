var SerialPort = require('serialport');

var d = 0;
var c = 0;

function start()
{	
	var port = new SerialPort("/dev/ttyS0", {
	  parser: SerialPort.parsers.byteLength(3)
	});
	
	// From an array
	var arr = new Uint8Array([0, 0, 0]);

	function error(err) {
		if (err) {
			return console.log('Error on write: ', err.message);
		}
	}
	
	// open errors will be emitted as an error event 
	port.on('error', error);

	port.on('data', function (data) {
		
		d = data.readInt16LE(); // takes the first two bytes from the data buffer and turns them into a 16 bit int
		c = data[2]; // this is the CRC
		
		console.log("Data: " + String(d)); 
		console.log("CRC: " + String(c)); 
	});

	port.on('open', function() {
		console.log("Port Opening")
		port.write(arr, error);
	});
	
	setInterval(function(){ 
		port.write(arr, error);
	}, 50);
}

function report()
{
	var reportJSON = {}; 
	
	reportJSON.sensorZero = d;
	reportJSON.sensorOne = c;
	
	process.send(JSON.stringify(reportJSON));
}

function getRandomInt(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

process.on('message', (m) => {
  // console.log('CHILD got message:', m);
  switch(m)
  {
	case "start":
		console.log("Starting");
		start(); 
		break;
	case "get":
		// console.log("Parent has requested count");
		report();
		break;
  }
});