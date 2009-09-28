#ifndef zest_hhp
/*
 * Copyright (c) 2009 Ash Berlin
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#define zest_hhp

#include <boost/shared_ptr.hpp>
#include <flusspferd.hpp>
#include "server.hpp"

namespace juice {

FLUSSPFERD_CLASS_DESCRIPTION(
  zest,
    (full_name, "juice.Zest")
    (constructor_name, "Zest")
    (methods,
      ("start", bind, start)
      ("stop", bind, stop)
    )
  )
{
private:
  typedef boost::shared_ptr<http::server::server> server_ptr_t;
  server_ptr_t _server;
public:
  zest(flusspferd::object const &self, flusspferd::call_context &x);
  virtual ~zest();

  void start();
  void stop();
};

}

#endif
