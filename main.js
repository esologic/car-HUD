const electron = require('electron')

// Module to control application life.
const app = electron.app;
const {ipcMain} = require('electron')

// Module to create native browser window.
const BrowserWindow = electron.BrowserWindow

const path = require('path')
const url = require('url')

var request_count = 0; 
const maxValue = 1000;
var zeroCount = 0;
var oneCount = maxValue;

const fork = require('child_process').fork;

const os = {
	silent: false
}

const child = fork('./work.js', options=os);

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let mainWindow

function createWindow () {
	// Create the browser window.
	mainWindow = new BrowserWindow({width: 800, height: 600})

	// and load the index.html of the app.
	mainWindow.loadURL(url.format({
		pathname: path.join(__dirname, 'index.html'),
		protocol: 'file:',
		slashes: true
	}))

	//Open the DevTools.
	mainWindow.webContents.openDevTools()

	// Emitted when the window is closed.
	mainWindow.on('closed', function () {
		// Dereference the window object, usually you would store windows
		// in an array if your app supports multi windows, this is the time
		// when you should delete the corresponding element.
		mainWindow = null
	})
}

function startWorker()
{
	child.send("start");
	setTimeout(function(){child.send("get")}, 5000 );
	setTimeout(function(){child.send("get")}, 2000 );
	setTimeout(function(){child.send("get")}, 3000 );
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)
app.on('ready', startWorker)

child.on('message', (m) => {
  console.log('PARENT got message:', m);
});

ipcMain.on('worker-to-main', (event, arg) => {  
	console.log("Got something from worker: " + String(arg));
	event.sender.send('main-to-worker', 1);
})

// Quit when all windows are closed.
app.on('window-all-closed', function () {
	// On OS X it is common for applications and their menu bar
	// to stay active until the user quits explicitly with Cmd + Q
	if (process.platform !== 'darwin') {
		app.quit()
	}
})

app.on('activate', function () {
	// On OS X it's common to re-create a window in the app when the
	// dock icon is clicked and there are no other windows open.
	if (mainWindow === null) {
		createWindow()
	}
})

// Listen for async message from renderer process
ipcMain.on('renderer-to-main', (event, arg) => {  
	
	var displayJSON = {}; 
	
	zeroCount = zeroCount + 1;
	if (zeroCount > maxValue)
	{
		zeroCount = 0;
	}
	
	oneCount = oneCount - 1;
	if (oneCount < 0)
	{
		oneCount = maxValue;
	}
	
	displayJSON.zeroCount = zeroCount;
	displayJSON.oneCount = oneCount;
	
	// console.log("Sending request: " + String(JSON.stringify(displayJSON)) + " to DOM");
	request_count++;
	
	// Reply on async message from renderer process
	event.sender.send('main-to-renderer', JSON.stringify(displayJSON));
});