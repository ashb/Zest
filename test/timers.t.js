var assert = require('test').asserts;
const z = require('zest');

exports.test_setTimeout = function() {
  var fired = false;
  var id = z.asio.setTimeout(function(){ fired = true }, 1000);

  assert.same(typeof id, "number", "setTimeout returns a number");

  print(z.asio.run_one());
  assert.same(true, fired, "Timer fired");
}

exports.est_MultipleTimers = function() {
  var fired1 = false, fired2 = false;
  z.asio.setTimeout(function(){ fired1 = true },10);
  z.asio.setTimeout(function(){ fired2 = true },10);

  z.asio.run();
  assert.same(true, fired1, "Timer 1 fired");
  assert.same(true, fired2, "Timer 2 fired");
}

exports.est_setTimeout = function() {
  var fired = false;
  var id = z.asio.setTimeout(function(){ fired = true });
  z.asio.clearTimeout(id);
  z.asio.poll();
  assert.same(false, fired, "Timer cancelled before it fired");
}

if (require.main === module)
  require('test').runner(exports);
