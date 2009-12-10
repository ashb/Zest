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

#include "connection.hpp"
#include <vector>
#include <boost/bind.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include "connection_manager.hpp"
#include "request_handler.hpp"

#include <flusspferd.hpp>
#include <flusspferd/aliases.hpp>

using namespace flusspferd;
using namespace flusspferd::aliases;

namespace phoenix = boost::phoenix;
namespace args = phoenix::arg_names;

using namespace zest::http;

connection::connection(boost::asio::io_service& io_service,
    connection_manager& manager, request_handler& handler)
  : socket_(io_service),
    connection_manager_(manager),
    request_handler_(handler)
{
}

boost::asio::ip::tcp::socket& connection::socket()
{
  return socket_;
}

void connection::start()
{
  socket_.async_read_some(boost::asio::buffer(buffer_),
      boost::bind(&connection::handle_read, shared_from_this(),
        boost::asio::placeholders::error,
        boost::asio::placeholders::bytes_transferred));
}

void connection::stop()
{
  socket_.close();
}

void connection::handle_read(const boost::system::error_code& e,
    std::size_t bytes_transferred)
{
  if (!e)
  {
    boost::tribool result;
    buffer_iterator end_of_read = buffer_.begin() + bytes_transferred,
                    consumed;

    boost::tie(result, consumed) = request_parser_.parse(
        request_, buffer_.begin(), end_of_read);

    if (result)
    {
      // Request parsed!
      // Store extra data (message body) that we might have already read from
      // the socket
      data_buffer_.insert(data_buffer_.begin(), consumed, end_of_read);

      // Handle the request
      request_handler_.handle_request(request_, reply_, *this);

      // And queue the response to be written out
      boost::asio::async_write(socket_, reply_.to_buffers(),
          boost::bind(&connection::handle_write, shared_from_this(),
            boost::asio::placeholders::error));
    }
    else if (!result)
    {
      reply_ = reply::stock_reply(reply::bad_request);
      boost::asio::async_write(socket_, reply_.to_buffers(),
          boost::bind(&connection::handle_write, shared_from_this(),
            boost::asio::placeholders::error));
    }
    else
    {
      socket_.async_read_some(boost::asio::buffer(buffer_),
          boost::bind(&connection::handle_read, shared_from_this(),
            boost::asio::placeholders::error,
            boost::asio::placeholders::bytes_transferred));
    }
  }
  else if (e != boost::asio::error::operation_aborted)
  {
    connection_manager_.stop(shared_from_this());
  }
}

void connection::handle_write(const boost::system::error_code& e)
{
  if (!e)
  {
    // Initiate graceful connection closure.
    boost::system::error_code ignored_ec;
    socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both, ignored_ec);
  }

  if (e != boost::asio::error::operation_aborted)
  {
    connection_manager_.stop(shared_from_this());
  }
}

object connection::read_from_body(std::size_t len) {
  local_root_scope rooter;
  binary &blob = create<byte_string>( make_vector( (binary::element_type*)0, 0));

  binary::vector_type &blob_vec = blob.get_data();

  std::size_t buf_size = data_buffer_.size();

  // There is some data in the buffers already - copy what we need out.
  if (buf_size) {
    std::size_t n = std::min(buf_size,len);
    std::deque<char>::iterator end = data_buffer_.begin() + n;

    blob_vec.insert( blob_vec.end(), data_buffer_.begin(), end );
    data_buffer_.erase( data_buffer_.begin(), end);
    len -= n;
  }
  while (len > 0) {
    boost::system::error_code ec;
    std::size_t n;

    n = boost::asio::read(
      socket_,
      boost::asio::buffer(buffer_),
      boost::asio::transfer_all(), ec
    );

    if (!ec)
    {
      if (n > len) {
        // We read more bytes than asked for. Store the unasked for ones in
        // data_buffer_
        buffer_iterator it = buffer_.begin() + len;
        blob_vec.insert(blob_vec.end(), buffer_.begin(), it);
        data_buffer_.insert(data_buffer_.end(), it+1, buffer_.begin() + n);
        len = 0;
      }
      else {
        len -= n;
        blob_vec.insert(blob_vec.end(), buffer_.begin(), buffer_.begin() + n);
      }
    }
    else
      throw exception(ec.message());
  }

  return blob;
}
