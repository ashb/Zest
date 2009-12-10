var assert = require('test').asserts;
const z = require('zest');

exports.test_setTimeout = function() {

  var fired = false, arg, id;

  id = z.reactor.setTimeout(function(arg_in){
    fired = true;
    arg = arg_in
  }, 0, "arg in");

  assert.same(typeof id, "number", "setTimeout returns a number");

  assert.same(false, fired, "Timer not fired");
  assert.diag(z.reactor.run_one());

  assert.same(true, fired, "Timer fired");
  assert.same("arg in", arg, "Args passed through");

  z.reactor.reset();
}

exports.test_MultipleTimers = function() {
  var fired1 = false, fired2 = false;
  z.reactor.setTimeout(function(){ fired1 = true },10);
  z.reactor.setTimeout(function(){ fired2 = true },5);

  gc();
  assert.diag(z.reactor.run_one());
  assert.same(false, fired1, "Timer 1 not yet fired");
  assert.same(true, fired2, "Timer 2 fired");
  assert.diag(z.reactor.run_one());
  assert.same(true, fired1, "Timer 1 now fired");

  z.reactor.reset();
}

exports.test_clearTimeout = function() {

  var fired = false;
  var id = z.reactor.setTimeout(function(){ fired = true });
  z.reactor.clearTimeout(id);
  z.reactor.poll();
  assert.same(false, fired, "Timer cancelled before it fired");

  z.reactor.reset();
}

exports.test_setInterval = function() {

  var fired = 0, id;

  id = z.reactor.setInterval(function(){
    fired++;
  }, 1);

  assert.same(typeof id, "number", "setInterval returns a number");

  assert.same(0, fired, "Timer not fired");
  assert.diag(z.reactor.poll_one());

  assert.same(1, fired, "Timer fired once");
  assert.diag(z.reactor.poll_one());
  assert.diag(z.reactor.poll_one());
  assert.same(3, fired, "Timer fired thrice");

  z.reactor.clearInterval(id);
  assert.diag(z.reactor.poll_one());
  assert.same(3, fired, "Interval timer cancelled");

  z.reactor.reset();
}

if (require.main === module)
  require('test').runner(exports);
