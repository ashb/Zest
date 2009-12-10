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

#ifndef ZEST_REQUEST_HANDLER_HPP
#define ZEST_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>
#include <flusspferd/object.hpp>

namespace zest {

struct zest_server;

namespace http {

struct reply;
struct request;
struct connection;

/// The handler for all incoming requests.
class request_handler
  : private boost::noncopyable
{
protected:
  zest_server &_server;
  flusspferd::object build_jsgi_env(const request &req, connection &conn);

  void serve_file(reply &rep, std::string const &fname,
                  bool add_content_type);
public:
  explicit request_handler(zest_server &server);

  void handle_request(const request &req,
                      reply & rep,
                      connection &conn);
};


} // namespace http
} // namespace zest

#endif // ZEST_REQUEST_HANDLER_HPP
