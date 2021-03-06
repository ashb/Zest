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

#include "server.hpp"
#include <boost/bind.hpp>

namespace asio = boost::asio;

using namespace zest::http;

server::server(const std::string& address, const std::string& port,
    request_handler& handler, boost::shared_ptr<boost::asio::io_service> io_service)
  : request_handler_(handler),
    io_service_(io_service),
    acceptor_(*io_service_),
    connection_manager_(),
    new_connection_(new connection(*io_service_,
          connection_manager_, request_handler_))
{
  // Open the acceptor with the option to reuse the address (i.e. SO_REUSEADDR).
  asio::ip::tcp::resolver resolver(*io_service_);
  asio::ip::tcp::resolver::query query(address, port);
  asio::ip::tcp::endpoint endpoint = *resolver.resolve(query);

  port_ = endpoint.port();

  if (endpoint.address() == asio::ip::address_v4::any()) {
    address_ = asio::ip::host_name();
  }
  else {
    address_ = endpoint.address().to_string();
  }

  acceptor_.open(endpoint.protocol());
  acceptor_.set_option(asio::ip::tcp::acceptor::reuse_address(true));
  acceptor_.bind(endpoint);
  acceptor_.listen();
  acceptor_.async_accept(new_connection_->socket(),
      boost::bind(&server::handle_accept, this,
        boost::asio::placeholders::error));
}

void server::run()
{
  // The io_service::run() call will block until all asynchronous operations
  // have finished. While the server is running, there is always at least one
  // asynchronous operation outstanding: the asynchronous accept call waiting
  // for new incoming connections.
  io_service_->run();
}

void server::stop()
{
  // Post a call to the stop function so that server::stop() is safe to call
  // from any thread.
  io_service_->post(boost::bind(&server::handle_stop, this));
}

void server::handle_accept(const boost::system::error_code& e)
{
  if (!e)
  {
    connection_manager_.start(new_connection_);
    new_connection_.reset(new connection(*io_service_,
          connection_manager_, request_handler_));
    acceptor_.async_accept(new_connection_->socket(),
        boost::bind(&server::handle_accept, this,
          asio::placeholders::error));
  }
}

void server::handle_stop()
{
  // The server is stopped by cancelling all outstanding asynchronous
  // operations. Once all operations have finished the io_service::run() call
  // will exit.
  acceptor_.close();
  connection_manager_.stop_all();
}
