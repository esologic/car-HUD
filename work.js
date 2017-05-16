const crypto = require('crypto');

// this usually takes a few seconds
function work(limit = 100) {
  let start = Date.now();
  n = 0;
  while(n < limit) {
    crypto.randomBytes(1);
    n++;
  }
  return {
    timeElapsed: Date.now() - start,
  };
}

module.exports = work;