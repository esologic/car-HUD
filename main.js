const electron = require('electron');
const fork = require('child_process').fork;

// Module to control application life.
const app = electron.app;
const {ipcMain} = require('electron');

// Module to create native browser window.
const BrowserWindow = electron.BrowserWindow;

const path = require('path');
const url = require('url');

var arduinoProcessReady = true;

const processConfig = {
	silent: false
}

var hardware_process = fork('./arduino_reader.js', options=processConfig);

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
let mainWindow

function createWindow () {
	// Create the browser window.
	mainWindow = new BrowserWindow()

	// and load the index.html of the app.
	mainWindow.loadURL(url.format({
		pathname: path.join(__dirname, 'index.html'),
		protocol: 'file:',
		slashes: true
	}))
	
	mainWindow.setFullScreen(true); // make the app full screen

	//Open the DevTools.
	//mainWindow.webContents.openDevTools()

	// Emitted when the window is closed.
	mainWindow.on('closed', function () {
		// Dereference the window object, usually you would store windows
		// in an array if your app supports multi windows, this is the time
		// when you should delete the corresponding element.
		mainWindow = null
	})
}

function startWorker() {
	hardware_process.send("start");
	
	setInterval(function() {
		if (arduinoProcessReady) { 
			// console.log("Main - Asking arduino_reader for data");
			hardware_process.send("get");
			arduinoProcessReady = false;
		}
	}, 100);
}

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)
app.on('ready', startWorker)

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
	
});

// Fires on new data from the arduino_reader process
hardware_process.on('message', (m) => {	

	// remap m from data from the arduino_reader to a display format
	
	var reportJSON = JSON.parse(m);
	
	var displayJSON = {
		chartValues: {
			chart_0: reportJSON.sensorValues[0], 
			chart_1: reportJSON.sensorValues[1],
			chart_2: reportJSON.sensorValues[2],
			chart_3: reportJSON.sensorValues[3],
			chart_4: reportJSON.sensorValues[4],
			chart_5: reportJSON.sensorValues[5]
		},
		
		switchValues: {
			sw1: reportJSON.sensorValues[6],
			sw2: reportJSON.sensorValues[7]
		},
		error: reportJSON.error,
		freq: reportJSON.sensorValues[8]
	};
	
	console.log(JSON.stringify(displayJSON))
	// send the remapped data to the renderer
	mainWindow.webContents.send('main-to-renderer', JSON.stringify(displayJSON));
	arduinoProcessReady = true;
});