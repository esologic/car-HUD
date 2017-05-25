var SerialPort = require('serialport');

var reportJSON = {}; 

reportJSON.sensorZero = 0;
reportJSON.sensorOne = 0;

var sensorNumber = 0;
var maxSensorNumber = 1; 

function start()
{	
	var port = new SerialPort("/dev/ttyS0", {
	  parser: SerialPort.parsers.byteLength(3)
	});
	
	function error(err) {
		if (err) {
			return console.log('Error on write: ', err.message);
		}
	}
	
	// open errors will be emitted as an error event 
	port.on('error', error);

	port.on('data', function (data) {
		
		var d = data.readInt16LE(); // takes the first two bytes from the data buffer and turns them into a 16 bit int
		var c = data[2]; // this is the CRC
		
		switch (sensorNumber)
		{
			case 0:
				reportJSON.sensorZero = d;
				break;
			case 1:
				reportJSON.sensorOne = d;
				break;
		}
		
		sensorNumber++;
		if (sensorNumber > maxSensorNumber)
		{
			sensorNumber = 0;
		}
		
	});
	
	setInterval(function(){

		// From an array
		var arr = new Uint8Array([0, sensorNumber, 0]);
		
		port.write(arr, error);
		
	}, 100);
}

function report()
{	
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