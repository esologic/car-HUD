const {ipcRenderer} = require('electron');
 
function work() {
	var count = 0;
	setInterval(send, 1);
}

function send()
{
	ipcRenderer.send('worker-to-main', String(count++))
}

module.exports = work;