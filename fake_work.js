function work()
{
	setInterval(increment , 10);
}

function increment()
{
	if (target_value == current_value)
	{
		console.log("Target reached!");
		target_value = getRandomInt(0, 1000);
	}
	else if (target_value > current_value)
	{
		current_value = current_value + 1;
	}
	else if (target_value < current_value)
	{
		current_value = current_value - 1;
	}
	console.log("Current Value: " + String(current_value) + " Target: " + String(target_value));
}

function report()
{
	var reportJSON = {}; 
	
	reportJSON.sensorZero = current_value;
	reportJSON.sensorOne = target_value;
	
	process.send(JSON.stringify(reportJSON));
}

function getRandomInt(min, max) {
    return Math.floor(Math.random() * (max - min + 1)) + min;
}

process.on('message', (m) => {
  console.log('CHILD got message:', m);
  switch(m)
  {
	case "start":
		console.log("Starting");
		work();
		break;
	case "get":
		console.log("Parent has requested count");
		report();
		break;
  }
});