var assert = require('test').asserts;
const z = require('zest');

exports.est_setTimeout = function() {
  z.asio.reset();

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
}

exports.test_MultipleTimers = function() {
  z.asio.reset();

  var fired1 = false, fired2 = false;
  z.asio.setTimeout(function(){ fired1 = true },10);
  z.asio.setTimeout(function(){ fired2 = true },5);

  assert.diag(z.asio.run_one());
  assert.same(false, fired1, "Timer 1 not yet fired");
  assert.same(true, fired2, "Timer 2 fired");
  assert.diag(z.asio.run_one());
  assert.same(true, fired1, "Timer 1 now fired");

  for (i in require.module_cache)
    delete require.module_cache[i];
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
