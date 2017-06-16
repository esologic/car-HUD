var SerialPort = require('serialport');

var reportJSON = {}; // this is the JSON object that is to be sent back to the main process

reportJSON.sensorValues = [];
reportJSON.error = false;

var sensorNumber = 0; // the sensor number that this process is reading
var maxSensorNumber = 8; // the max sensor number, NOT zero indexed

var startMode = true;
var arduinoReady = true;
var numResponseErrors = 0;

// reset stuff
var numTransmissions = 0;

function calcCRC(preCRC) {
	var CRC = 0;
	for (var i in preCRC) {
		CRC = (CRC + preCRC[i]) % 255; 
	}
	return CRC;
}

function writeAndDrain (port, data, error) {
  port.write(data, function () {
    port.drain(error);
  });
}

// process a message from the parent process
function processMessage(m) {
	switch(m) {
		case "get":
			process.send(JSON.stringify(reportJSON));
			break;
		
		case "start":
			var port = new SerialPort("/dev/ttyS0", {
				  parser: SerialPort.parsers.byteLength(3)
			});
			
			function error(err) {
				if (err) {
					console.log('AR - Error on write: ' +  String(err.message));
				}
			}
			
			/* set up the handlers for port events */ 
			
			// open errors will be emitted as an error event 
			port.on('error', error);

			// fires whenever data arrives on the input buffer
			port.on('data', function (data) {
				console.log("AR - Got a message back from the Arduino: [" + String(data[0]) + "," + String(data[1]) + "," + String(data[2]) + "]");
				var d = data.readInt16LE(); // takes the first two bytes from the data buffer and turns them into a 16 bit int
				var incomingCRC = data[2]; // this is the CRC
				var calculatedCRC = calcCRC([data[0], data[1]]);
				var goodCRC = true;
				
				if (d == -256) {
					console.log("AR - Bad Master->Slave CRC, retrying sensor read");
					goodCRC = false;
				} else if (calculatedCRC != incomingCRC) {
					console.log("AR - Bad Slave->Master CRC, retrying sensor read");
					goodCRC = false;
				}
				
				if (startMode == true) {
					if (d == -512) {
						console.log("AR - Start mode ready received");
						goodCRC = false;
						startMode = false;
					} else {
						console.log("AR - Message received in startMode, but bad message");
						goodCRC = false;
					}
				}
				
				if (goodCRC) {
					console.log("AR - CRC passed, got regular message")
					reportJSON.sensorValues[sensorNumber] = d;	
					reportJSON.error = false;
					sensorNumber++;
					if (sensorNumber > maxSensorNumber) {
						sensorNumber = 0;
					}
				} else {
					port.flush();
				}
				arduinoReady = true;
			});
			
			// Start polling the arduino 
			setInterval(function() {
				if (arduinoReady) {
					var messageBytes;
					if (startMode) {
						console.log("AR - Sending start mode bytes")
						messageBytes = [2, 0];	
						port.drain();
					} else {
						messageBytes = [0, sensorNumber];	
					}
					
					messageBytes.push(calcCRC(messageBytes));
					writeAndDrain(port, messageBytes, error);
					console.log("AR - Sending message to arduino: [" + String(messageBytes) + "]");					
					numResponseErrors = 0;
					
					arduinoReady = false;
				} else {
					console.log("Arduino not ready, misses: " + String(numResponseErrors));
					numResponseErrors++;
					if (numResponseErrors > 100) {
						console.log("Big Problem");
						reportJSON.error = true;
						port.drain();
						arduinoReady = true;
						startMode = true;
					}
				}
			}, 10);
			break;
	}
}

process.on('message', (m) => {
	processMessage(m)
});