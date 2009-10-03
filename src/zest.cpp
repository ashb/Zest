/*
 * Copyright (c) 2009 Ash Berlin
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */
#include "zest.hpp"
#include <boost/lexical_cast.hpp>

using namespace flusspferd;
using namespace zest;

FLUSSPFERD_LOADER_SIMPLE(exports) {
  load_class<zest_server>(exports);
}

zest_server::zest_server(object const &self, call_context &x)
  : base_type(self)
{
  std::string addr, port;

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

  _req_handler.reset(new request_handler(*this));
  _server.reset(new server(addr,port,*_req_handler));
}

zest_server::~zest_server() {
  if (_server.unique()) {
    _server->stop();
  }
}

void zest_server::start() {
  // Root the zest object to make sure it doesn't get GC'd
  root_object rooted_server(this->get_object());
  std::cerr << "Listening on http://" << address() << ":" << port() << std::endl;
  _server->run();
}

void zest_server::stop() {
  _server->stop();
}

void zest_server::trace(tracer &trc) {
  trc("zest.handler callback", _handler_cb);
}

unsigned short zest_server::port() {
  return _server->port();
}

std::string zest_server::address() {
  return _server->address();
}

string zest_server::to_string() {
  std::string ret = "[object Zest http://";
  ret += address();
  ret += ":";
  ret += boost::lexical_cast<std::string>(port());
  ret += " ]";
  return ret;
}
