// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

const {ipcRenderer} = require('electron');

const max_value = 1000;

var zero_count = 0;
var one_count = max_value;

ipcRenderer.send('renderer-to-main', 1);

// Listen for async-reply message from main process
ipcRenderer.on('main-to-renderer', (event, arg) => {  

	zero_count = zero_count + 1;
	
	if (zero_count > max_value)
	{
		zero_count = 0;
	}
	
	one_count = one_count - 1;

	if (one_count < 0)
	{
		one_count = max_value;
	}
	
	let el0 = document.getElementById("el0");
	el0.innerHTML = String(zero_count);
	
	let el1 = document.getElementById("el1");
	el1.innerHTML = String(one_count);
	
	// Reply on async message from renderer process
    event.sender.send('renderer-to-main', 1);
});