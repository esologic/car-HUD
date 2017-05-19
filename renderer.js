// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

const {ipcRenderer} = require('electron');

var Chart = require('chart.js')

var ctx = document.getElementById("myChart");
ctx.height = 100;

var myChart = new Chart(ctx, {
	type: 'bar',
	data: {
		labels: ["Red"],
		datasets: [{
			//label: '# of Votes',
			data: [0],
			backgroundColor: [
				'rgba(255, 99, 132, 0.2)'
			],
			borderColor: [
				'rgba(255,99,132,1)',
			],
			borderWidth: 1
		}]
	},
	options: {
		scales: {
			yAxes: [{
				ticks: {
					beginAtZero:true,
					max: 1000
				}
				
			}]
		},
		legend: {
			display: false
		}
	}
});

// Listen for async-reply message from main process
ipcRenderer.on('main-to-renderer', (event, arg) => {  
	
	var displayJSON = JSON.parse(String(arg));
	
	let el0 = document.getElementById("el0");
	let el1 = document.getElementById("el1");
	
	el0.innerHTML = String(displayJSON.zeroCount);
	el1.innerHTML = String(displayJSON.oneCount);
	
	myChart.data.datasets[0].data[0] = displayJSON.zeroCount;
	myChart.update();

	var s = 'rgba(255,99,132,' + String(displayJSON.zeroCount/1000) + ')';
	console.log(s);
	myChart.data.datasets[0].backgroundColor[0] = s;
	myChart.render();
	
	// Reply on async message from renderer process
    event.sender.send('renderer-to-main', 1);
});

