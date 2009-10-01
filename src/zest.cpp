/*
 * Copyright (c) 2009 Ash Berlin
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "zest.hpp"
#include <boost/foreach.hpp>
#include <boost/spirit/home/phoenix/core.hpp>
#include <boost/spirit/home/phoenix/bind.hpp>
#include <string>

using namespace flusspferd;
using namespace juice;
namespace httpd = http::server;
namespace phoenix = boost::phoenix;
namespace args = phoenix::arg_names;

FLUSSPFERD_LOADER_SIMPLE(exports) {
  load_class<zest>(exports);
}

zest::zest(object const &self, call_context &x)
  : base_type(self)
{
  std::string addr, port, docroot;

  object opts;
  if (x.arg.size() < 1 ||
      x.arg[0].is_undefined_or_null() ||
      !x.arg[0].is_object())
  {
    throw exception("juice.Zest requires an options dictionary as its first argument",
                    "TypeError");
  }

  opts = x.arg[0].get_object();

  value v;
  if (!(v = opts.get_property("handler")).is_function())
    throw exception("juice.Zest requires a function as the handler option", "TypeError");

  _handler_cb = v.get_object();

  if (opts.has_property("port"))
    port = opts.get_property("port").to_std_string();
  else
    port = "3000";

  if (opts.has_property("address"))
    addr = opts.get_property("address").to_std_string();
  else
    addr = "0.0.0.0";

  _req_handler.reset(new jsgi_request_handler(*this));
  _server.reset(new http::server::server(addr,port,*_req_handler));
}

zest::~zest() {
  if (_server.unique()) {
    _server->stop();
  }
}

void zest::start() {
  // Root the zest object to make sure it doesn't get GC'd
  root_object rooted_server(this->get_object());
  std::cerr << "Starting" << std::endl;
  _server->run();
}

void zest::stop() {
  _server->stop();
}

void zest::trace(tracer &trc) {
  trc("zest.handler callback", _handler_cb);
}

jsgi_request_handler::jsgi_request_handler(zest &server)
  : _server(server)
{
}

void jsgi_request_handler::handle_request(const httpd::request &req, httpd::reply &rep)
{
  // Turn the req into the JSGI env.
  object env;

  {
    local_root_scope scope;

    env = create_object();

    // env.jsgi
    object jsgi = create_object();
    env.set_property("jsgi", jsgi);

    array ver = create_array();
    ver.call("push", 0,3,0);
    jsgi.set_property("version", ver);

    jsgi.set_property(
      "errors",
      global().call("require", "system")
              .to_object()
              .get_property("stderr")
    );

    env.set_property("requestMethod", req.method);
    env.set_property("scriptName", "");
    env.set_property("pathInfo", req.uri);
    env.set_property("queryString", req.query_string);
    //env.set_property("protocol", req.

    // env.headers
    object headers = create_object();

    BOOST_FOREACH(http::server::header h, req.headers) {
      string hdr_str;
      if (headers.has_own_property(h.name)) {
        // already exists, combined as specified by RFC 2616
        hdr_str = string::concat(
            string::concat(
              headers.get_property(h.name),
              ","
            ),
            h.value
        );
      }
      else {
        hdr_str = h.value;
      }
      headers.set_property(h.name, hdr_str);
    }

    env.set_property("headers", headers);
  }

  // Root it! Make sure it doesn't get GC'd when we are sending, cos that would
  // be really bad
  root_value const &rv = _server._handler_cb.call(_server, env);

  if (rv.is_undefined_or_null() || !rv.is_object()) {
    std::cerr << "no res or res is not an object" << std::endl;
    rep = http::server::reply::stock_reply(http::server::reply::internal_server_error);
    return;
  }

  object jsgi_res = rv.to_object();
  object body = jsgi_res.get_property_object("body");

  value v = jsgi_res.get_property("status");
  int status;

  if (!v.is_int() || (status = v.get_int())  < 100) {
    std::cerr << "res.status is not a valid HTTP status code" << std::endl;
    rep = http::server::reply::stock_reply(http::server::reply::internal_server_error);
    return;
  }

  rep.status = http::server::reply::status_type(status);

  // Populate headers
  object const &hdrs = jsgi_res.get_property_object("headers");

  bool content_length_found = false;
  for ( property_iterator iter = hdrs.begin(), end = hdrs.end(); iter != end; ++iter) {
    http::server::header h;
    h.name = iter->to_std_string();
    h.value = hdrs.get_property(*iter).to_std_string();
    rep.headers.push_back(h);
  }

  // Create a callback closure to pass to forEach
  root_function body_writer(create_native_function(
    object(),
    "Zest.writeChunk",
    boost::function<void (value)>(
      phoenix::bind(&http::server::reply::body_appender, rep, args::arg1)
    )
  ));

  body.call("forEach", body_writer);
}
