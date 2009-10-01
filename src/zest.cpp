/*
 * Copyright (c) 2009 Ash Berlin
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "zest.hpp"
#include "mime_types.hpp"
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/home/phoenix/core.hpp>
#include <boost/spirit/home/phoenix/bind.hpp>
#include <string>
#include <fstream>

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

object jsgi_request_handler::build_jsgi_env(
  const httpd::request &req, httpd::connection &conn) {
  local_root_scope scope;

  object env = create_object();

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

  object body = create_object();
  env.set_property("body", body);

  // Create a callback closing over the socket.
  root_function body_writer(create_native_function(
    body,
    "read",
    boost::function<object (std::size_t)>(
      phoenix::bind(&http::server::connection::read_from_body, conn, args::arg1)
    )
  ));


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

  return env;
}

void jsgi_request_handler::handle_request(
  const httpd::request &req, httpd::reply &rep,
  httpd::connection &conn)
{
  // Turn the req into the JSGI env.
  object env = build_jsgi_env(req, conn);

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
  object hdrs = jsgi_res.get_property_object("headers");

  bool send_file = false;
  std::string x_sendfile;
  if (( send_file = hdrs.has_property("x-sendfile")) == true) {
    x_sendfile = hdrs.get_property("x-sendfile").to_std_string();
    hdrs.delete_property("x-sendfile");
  }

  bool content_type_set = false;
  for ( property_iterator iter = hdrs.begin(), end = hdrs.end(); iter != end; ++iter) {
    http::server::header h;
    h.name = iter->to_std_string();

    // If using X-SendFile, ignore any content length headers
    if (send_file && boost::to_lower_copy(h.name) == "content-length")
      continue;
    // If using X-SendFile, use Content-Type if provided
    if (send_file && boost::to_lower_copy(h.name) == "content-type")
      content_type_set = true;

    h.value = hdrs.get_property(*iter).to_std_string();
    rep.headers.push_back(h);
  }

  // If X-SendFile, ignore body.
  if (send_file) {
    serve_file(rep, x_sendfile, !content_type_set);
    return;
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


void jsgi_request_handler::serve_file(http::server::reply &rep,
                  std::string const &fname, bool add_content_type) {

  std::ifstream is(fname.c_str(), std::ios::in | std::ios::binary);
  if (!is)
  {
    rep = httpd::reply::stock_reply(httpd::reply::not_found);
    return;
  }

  char buf[512];
  while (is.read(buf, sizeof(buf)).gcount() > 0)
    rep.content.append(buf, is.gcount());
  httpd::header h;
  h.name = "Content-Length";
  h.value = boost::lexical_cast<std::string>(rep.content.size());
  rep.headers.push_back(h);
  if (add_content_type) {

    std::string extension;
    // Determine the file extension.
    std::size_t last_slash_pos = fname.find_last_of("/");
    std::size_t last_dot_pos = fname.find_last_of(".");
    if (last_dot_pos != std::string::npos && last_dot_pos > last_slash_pos)
    {
      extension = fname.substr(last_dot_pos + 1);
    }

    h.name = "Content-Type";
    h.value = httpd::mime_types::extension_to_type(extension);
    rep.headers.push_back(h);
  }
}
