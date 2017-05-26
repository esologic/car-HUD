var SerialPort = require('serialport');

var reportJSON = {}; // this is the JSON object that is to be sent back to the main process

reportJSON.sensorZero = 0; 	// init the sensor to zero
reportJSON.sensorOne = 0;	// init the sensor to zero
reportJSON.sensorTwo = 0;	// init the sensor to zero
reportJSON.sensorThree = 0;	// init the sensor to zero
reportJSON.sensorFour = 0;	// init the sensor to zero
reportJSON.sensorFive = 0;	// init the sensor to zero
reportJSON.sensorSix = 0;	// init the sensor to zero

var sensorNumber = 0; // the sensor number that this process is reading
var maxSensorNumber = 6; // the max sensor number

var readyToGet = true;

function getRandomInt(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

process.on('message', (m) => {
  switch(m)
  {
	case "start":
		console.log("Starting");
		
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
				case 2:
					reportJSON.sensorTwo = d;
					break;
				case 3:
					reportJSON.sensorThree = d;
					break;
				case 4:
					reportJSON.sensorFour = d;
					break;
				case 5:
					reportJSON.sensorFive = d;
					break;
				case 6:
					reportJSON.sensorSix = d;
					break;
			}
			
			sensorNumber++;
			if (sensorNumber > maxSensorNumber)
			{
				sensorNumber = 0;
			}
			
			readyToGet = true;
		});
		
		setInterval(function(){
			
			if (readyToGet) {

				var CRC = 0;
				var preCRC = [0, sensorNumber];
				
				for (var i in preCRC) {
					CRC = (CRC + preCRC[i]) % 255; 
				}
				
				preCRC.push(CRC)
				
				port.write(preCRC, error);
								
				readyToGet = false; 
			}
			
		}, 10);
		break;

	case "get":
		process.send(JSON.stringify(reportJSON));
		break;
	  }
});