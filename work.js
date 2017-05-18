var count = 0;

function work() {
	setInterval(c , 900);
}

function c()
{
	count++;
}

function tx()
{
	console.log('CHILD sending message:', String(count))
	process.send(String(count));
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
		console.log("Starting");
		tx();
		break;
  }
});

process.on('close', (code, signal) => {
	console.log("Process Closed")
	console.log(String(code));
	console.log(String(signal));
});