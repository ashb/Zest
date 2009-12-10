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

#ifndef ZEST_REQUEST_HPP
#define ZEST_REQUEST_HPP

#include <string>
#include <vector>
#include "header.hpp"

namespace zest {
namespace http {

/// A request received from a client.
struct request
{
  std::string method;
  std::string uri;
  std::string query_string;
  int http_version_major;
  int http_version_minor;
  std::vector<header> headers;
};

} // namespace http
} // namespace zest

#endif // ZEST_REQUEST_HPP
