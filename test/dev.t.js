
require.paths.unshift('build');
var z = require('zest');

var server = z.Zest({
  handler: function(req) {

    var blob = req.body.read(4);
    req.jsgi.errors.print(blob.decodeToString());

    req.jsgi.errors.print("*hello");
    return {
      status: 200,
      body: ["I'm content from " + module.uri],
      headers: {
        'content-type': 'text/plain'
      }
    }
  }
});

server.start()
