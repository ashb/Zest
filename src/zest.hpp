/*
 * Copyright (c) 2009 Ash Berlin
 *
 * Based on work that is
 * Copyright (c) 2003-2008 Christopher M. Kohlhoff (chris at kohlhoff dot com)
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#ifndef zest_hhp
#define zest_hhp

#include <boost/shared_ptr.hpp>
#include <flusspferd.hpp>
#include "server.hpp"

namespace zest {

// Setup setTimeout et al.
void setup_evented();


class jsgi_request_handler;

FLUSSPFERD_CLASS_DESCRIPTION(
  zest_server,
    (full_name, "juice.Zest")
    (constructor_name, "Zest")
    (methods,
      ("start", bind, start)
      ("stop", bind, stop)
    )
    (properties,
      ("address", getter, address)
      ("port", getter, port)
    )
  )
{
private:
  friend class request_handler;
  typedef boost::shared_ptr<server> server_ptr_t;
  typedef boost::shared_ptr<request_handler> request_handler_t;

  server_ptr_t _server;
  request_handler_t _req_handler;
  flusspferd::function _handler_cb;

public:
  zest_server(flusspferd::object const &self, flusspferd::call_context &x);
  virtual ~zest_server();

  void start();
  void stop();

  void trace(flusspferd::tracer &trc);

  unsigned short port();
  std::string address();

  flusspferd::string to_string();
};

} // namespace zest

#endif
