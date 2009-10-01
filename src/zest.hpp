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

namespace juice {

class jsgi_request_handler;

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
  friend class jsgi_request_handler;
  typedef boost::shared_ptr<http::server::server> server_ptr_t;
  typedef boost::shared_ptr<jsgi_request_handler> request_handler_t;

  server_ptr_t _server;
  request_handler_t _req_handler;
  flusspferd::function _handler_cb;
public:
  zest(flusspferd::object const &self, flusspferd::call_context &x);
  virtual ~zest();

  void start();
  void stop();

  void trace(flusspferd::tracer &trc);
};

class jsgi_request_handler : public http::server::request_handler {
protected:
  zest &_server;
public:
  explicit jsgi_request_handler(zest &server);

  void handle_request(const http::server::request &req,
                      http::server::reply & rep,
                      http::server::connection &conn);
};

}

#endif
