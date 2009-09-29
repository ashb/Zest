/*
 * Copyright (c) 2009 Ash Berlin
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "zest.hpp"
#include <string>

using namespace flusspferd;
using namespace juice;
namespace httpd = http::server;

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
  function cb = v.get_object();

  if (opts.has_property("port"))
    port = opts.get_property("port").to_std_string();
  else
    port = "3000";

  if (opts.has_property("address"))
    addr = opts.get_property("address").to_std_string();
  else
    addr = "0.0.0.0";

  _req_handler.reset(new jsgi_request_handler(cb));
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
  _server->run();
}

void zest::stop() {
  _server->stop();
}

jsgi_request_handler::jsgi_request_handler(function &cb_)
  : cb(cb_)
{ }

void jsgi_request_handler::handle_request(const httpd::request &req, httpd::reply &rep)
{
  cb.call();
}
