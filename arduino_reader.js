var SerialPort = require('serialport');

var reportJSON = {}; // this is the JSON object that is to be sent back to the main process

reportJSON.sensorValues = [];

var sensorNumber = 0; // the sensor number that this process is reading
var maxSensorNumber = 6; // the max sensor number

var arduinoReady = true;
var numResponseErrors = 0;

var numTransmissions = 0;

function calcCRC(preCRC) {
	var CRC = 0;
	for (var i in preCRC) {
		CRC = (CRC + preCRC[i]) % 255; 
	}
	return CRC;
}

// process a message from the parent process
function processMessage(m) {
	switch(m) {
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
				
				var d = data.readInt16LE(); // takes the first two bytes from the data buffer and turns them into a 16 bit int
				var incomingCRC = data[2]; // this is the CRC
				
				var calculatedCRC = calcCRC([data[0], data[1]]);
				
				var badMasterSlave = (d == -256);
				var badSlaveMaster = (calculatedCRC != incomingCRC);
				
				var goodCRC = true;
				
				if (badMasterSlave) {
					console.log("AR - Bad Master->Slave CRC, retrying sensor read");
					goodCRC = false;
				} if (badSlaveMaster) {
					console.log("AR - Bad Slave->Master CRC, retrying sensor read");
					goodCRC = false;
				} 
				
				if (goodCRC) {
					reportJSON.sensorValues[sensorNumber] = d;					
					sensorNumber++;
					if (sensorNumber > maxSensorNumber) {
						sensorNumber = 0;
					}
				}
				
				// console.log("AR - Got a message back from the Arduino: [" + String(data[0]) + "," + String(data[1]) + "," + String(data[2]) + "]");

				arduinoReady = true;
			});
			
			// Start polling the arduino 
			setInterval(function() {
				if (arduinoReady) {
					var messageBytes = [0, sensorNumber];
					var CRC = calcCRC(messageBytes);
										
					messageBytes.push(CRC);
					port.write(messageBytes, error);
					
					// console.log("AR - Sending message to arduino: [" + String(messageBytes) + "]");					
					
					numResponseErrors = 0;
					arduinoReady = false;
				} else {
					numResponseErrors++;
					if (numResponseErrors > 100){
						console.log("Can't write! haven't gotten response yet, this is a problem")
					}
				}
			}, 10);
			break;

		case "get":
			process.send(JSON.stringify(reportJSON));
			break;
	}
}

process.on('message', (m) => {
	processMessage(m)
});