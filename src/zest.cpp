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

  if (!opts.has_property("docRoot"))
    throw exception("juice.Zest require a docRoot option", "TypeError");
  docroot = opts.get_property("docRoot").to_std_string();

  if (opts.has_property("port"))
    port = opts.get_property("port").to_std_string();
  else
    port = "3000";

  if (opts.has_property("address"))
    addr = opts.get_property("address").to_std_string();
  else
    addr = "0.0.0.0";

  _server.reset(new http::server::server(addr,port,docroot));
}

zest::~zest() {
  if (_server.unique()) {
    _server->stop();
  }
}

void zest::start() {
  root_object rooted_server(this->get_object());
  _server->run();
}

void zest::stop() {
  _server->stop();
}
