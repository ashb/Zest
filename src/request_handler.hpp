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

#ifndef HTTP_REQUEST_HANDLER_HPP
#define HTTP_REQUEST_HANDLER_HPP

#include <string>
#include <boost/noncopyable.hpp>

namespace http {
namespace server {

struct reply;
struct request;
struct connection;

/// The common handler for all incoming requests.
class request_handler
  : private boost::noncopyable
{
public:
  virtual ~request_handler() {}

  /// Handle a request and produce a reply.
  virtual void handle_request(const request& req, reply& rep, connection &conn) = 0;
};

} // namespace server
} // namespace http

#endif // HTTP_REQUEST_HANDLER_HPP
