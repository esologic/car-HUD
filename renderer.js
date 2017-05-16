// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

const {ipcRenderer} = require('electron');
var main = require.resolve('./main')

// Send async message to main process
ipcRenderer.send('renderer-to-main', 1);

// Listen for async-reply message from main process
ipcRenderer.on('main-to-renderer', (event, arg) => {  
    console.log("In Renderer: " + String(arg))
    let value = parseInt(arg);
	value = value + 1;
	// Reply on async message from renderer process
    event.sender.send('renderer-to-main', String(value));
});

// Listen for main message
ipcRenderer.on('ping', (event, arg) => {  
    // Print 5
    console.log(arg);
    // Invoke method directly on main process
    main.pong(6);
});