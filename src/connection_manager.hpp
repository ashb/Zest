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

#ifndef ZEST_CONNECTION_MANAGER_HPP
#define ZEST_CONNECTION_MANAGER_HPP

#include <set>
#include <boost/noncopyable.hpp>
#include "connection.hpp"

namespace zest {

/// Manages open connections so that they may be cleanly stopped when the server
/// needs to shut down.
class connection_manager
  : private boost::noncopyable
{
public:
  /// Add the specified connection to the manager and start it.
  void start(connection_ptr c);

  /// Stop the specified connection.
  void stop(connection_ptr c);

  /// Stop all connections.
  void stop_all();

private:
  /// The managed connections.
  std::set<connection_ptr> connections_;
};

} // namespace zest

#endif // ZEST_CONNECTION_MANAGER_HPP
