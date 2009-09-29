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
  typedef boost::shared_ptr<http::server::server> server_ptr_t;
  typedef boost::shared_ptr<jsgi_request_handler> request_handler_t;

  server_ptr_t _server;
  request_handler_t _req_handler;
public:
  zest(flusspferd::object const &self, flusspferd::call_context &x);
  virtual ~zest();

  void start();
  void stop();
};

class jsgi_request_handler : public http::server::request_handler {
protected:
  flusspferd::function &cb;
public:
  friend class zest;
  explicit jsgi_request_handler(flusspferd::function &cb);

  void handle_request(const http::server::request &req,
                      http::server::reply & rep);
};

}

#endif
