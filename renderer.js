// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

const {ipcRenderer} = require('electron');

var Chart = require('chart.js')

const config = {
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
		},
		
		tooltips: {
			enabled: false
		}
	},
	animation : false
}

var chart_element_names = ["channel_0", "channel_1", "channel_2", "channel_3", "channel_4", "channel_5", "channel_6", "channel_7"];
var charts = [];

for (var index = 0; index < chart_element_names.length; index++)
{
	var element = document.getElementById(chart_element_names[index]);
	element.height = 600;
	var chart = new Chart(element, config);
	charts[index] = chart;	
}

// Listen for async-reply message from main process
ipcRenderer.on('main-to-renderer', (event, arg) => {  
	
	var displayJSON = JSON.parse(String(arg));

	let el0 = document.getElementById("el0");
	let el1 = document.getElementById("el1");
	
	el0.innerHTML = String(displayJSON.zeroCount);
	el1.innerHTML = String(displayJSON.oneCount);
	
	charts[1].data.datasets[0].data[0] = displayJSON.oneCount;
	s = 'rgba(255,99,132,' + String(displayJSON.oneCount/1000) + ')';
	charts[1].data.datasets[0].backgroundColor[0] = s;
		
	for (var i in charts) {
		charts[i].update();
		var s = 'rgba(255,99,132,' + String(displayJSON.zeroCount/1000) + ')';
		charts[i].data.datasets[0].backgroundColor[0] = s;
	}
	
	// Reply on async message from renderer process
    event.sender.send('renderer-to-main', 1);
});

