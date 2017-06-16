// This file is required by the index.html file and will
// be executed in the renderer process for that window.
// All of the Node.js APIs are available in this process.

const {ipcRenderer} = require('electron');

var Chart = require('chart.js')

var config = {
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

var chart_element_names = ["channel_0", "channel_1", "channel_2", "channel_3", "channel_4", "channel_5"];
var charts = [];

for (var i in chart_element_names) {
	var elementName = chart_element_names[i];
	console.log("Creating Chart for: " + String(elementName))
	var element = document.getElementById(elementName);
	element.height = 600;
	var myConfig = JSON.parse(JSON.stringify(config)); // you have to do this, or they will all reference the same DOM canvas. 
	var chart = new Chart(element, myConfig); 
	console.log(chart)
	charts[i] = chart;
}

// Listen for async-reply message from main process
ipcRenderer.on('main-to-renderer', (event, arg) => {  
	
	var displayJSON = JSON.parse(String(arg));

	var errorElement = document.getElementById("error");
	errorElement.innerHTML = String(displayJSON.error)
	
	if (displayJSON.error == false) {
	
		var valueNumber = 0;
		var values = [];
		
		for (var key in displayJSON.chartValues) {
			if (displayJSON.chartValues.hasOwnProperty(key)) {
				var val = displayJSON.chartValues[key];
				values[valueNumber] = val;
				valueNumber++;
			}	
		}
		
		var element = document.getElementById("el2");
		element.innerHTML = String(displayJSON.switchValues.sw1)
		
		for (var i in charts) {
			charts[i].data.datasets[0].data[0] = values[i];
			charts[i].update();
		}
		
		var element2 = document.getElementById("freq");
		element2.innerHTML = String(displayJSON.freq)
	}
	// Reply on async message from renderer process
    event.sender.send('renderer-to-main', 1);
});
