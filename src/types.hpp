/*
 * Copyright (c) 2009 Ash Berlin
 *
 * Distributed under the Boost Software License, Version 1.0. (See accompanying
 * file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */

#ifndef ZEST_TYPES_HPP
#define ZEST_TYPES_HPP

#include <boost/shared_ptr.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/system/error_code.hpp>
#include <flusspferd/object.hpp>

namespace zest {

typedef boost::shared_ptr<boost::asio::io_service> shared_io_service;

// Setup setTimeout et al., default asio instace and what have you
void setup_reactor(flusspferd::object exports, flusspferd::object require);

} // ns zest

#endif
