var assert = require('test').asserts;
const z = require('zest');

exports.test_setTimeout = function() {

  var fired = false, arg, id;

  id = z.asio.setTimeout(function(arg_in){
    fired = true;
    arg = arg_in
  }, 0, "arg in");

  assert.same(typeof id, "number", "setTimeout returns a number");

  assert.same(false, fired, "Timer not fired");
  assert.diag(z.asio.run_one());

  assert.same(true, fired, "Timer fired");
  assert.same("arg in", arg, "Args passed through");

  z.asio.reset();
}

exports.test_MultipleTimers = function() {
  var fired1 = false, fired2 = false;
  z.asio.setTimeout(function(){ fired1 = true },10);
  z.asio.setTimeout(function(){ fired2 = true },5);

  gc();
  assert.diag(z.asio.run_one());
  assert.same(false, fired1, "Timer 1 not yet fired");
  assert.same(true, fired2, "Timer 2 fired");
  assert.diag(z.asio.run_one());
  assert.same(true, fired1, "Timer 1 now fired");

  z.asio.reset();
}

exports.test_clearTimeout = function() {

  var fired = false;
  var id = z.asio.setTimeout(function(){ fired = true });
  z.asio.clearTimeout(id);
  z.asio.poll();
  assert.same(false, fired, "Timer cancelled before it fired");

  z.asio.reset();
}

exports.test_setInterval = function() {

  var fired = 0, id;

  id = z.asio.setInterval(function(){
    fired++;
  }, 1);

  assert.same(typeof id, "number", "setInterval returns a number");

  assert.same(0, fired, "Timer not fired");
  assert.diag(z.asio.poll_one());

  assert.same(1, fired, "Timer fired once");
  assert.diag(z.asio.poll_one());
  assert.diag(z.asio.poll_one());
  assert.same(3, fired, "Timer fired thrice");

  z.asio.clearInterval(id);
  assert.diag(z.asio.poll_one());
  assert.same(3, fired, "Interval timer cancelled");

  z.asio.reset();
}

if (require.main === module)
  require('test').runner(exports);
