// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

const {ipcRenderer} = require('electron');

ipcRenderer.send('renderer-to-main', 1);

// Listen for async-reply message from main process
ipcRenderer.on('main-to-renderer', (event, arg) => {  
	
	var displayJSON = JSON.parse(String(arg));
	
	let el0 = document.getElementById("el0");
	let el1 = document.getElementById("el1");
	
	el0.innerHTML = String(displayJSON.zeroCount);
	el1.innerHTML = String(displayJSON.oneCount);
	
	// Reply on async message from renderer process
    event.sender.send('renderer-to-main', 1);
});