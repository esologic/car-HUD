const electron = require('electron')

// Module to control application life.
const app = electron.app;
const {ipcMain} = require('electron')

// Module to create native browser window.
const BrowserWindow = electron.BrowserWindow

const path = require('path')
const url = require('url')

var request_count = 0; 

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

// This method will be called when Electron has finished
// initialization and is ready to create browser windows.
// Some APIs can only be used after this event occurs.
app.on('ready', createWindow)

// app.on('ready', thing)

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
	console.log("Sending request: " + String(request_count) + " to DOM");
	request_count++;
	// Reply on async message from renderer process
	event.sender.send('main-to-renderer-0', "done");
	event.sender.send('main-to-renderer-1', "done");
});