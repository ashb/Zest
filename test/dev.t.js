
require.paths.unshift('build');
var z = require('zest');

var server = z.Zest({
  handler: function(req) {
    print(uneval(req));
    return {
      status: 200,
      body: [1]
    }
  }
});

server.start()
