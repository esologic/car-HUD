function work() {
  count = 0
  let val = getRandomArbitrary(0, 10000);
  setTimeout(function(){console.log("done")}, val);	// done should get shot into the void
  return val;
}

function getRandomArbitrary(min, max) {
  return Math.floor(Math.random() * (max - min)) + min;
}

module.exports = work;