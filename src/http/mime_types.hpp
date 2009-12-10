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

#ifndef ZEST_MIME_TYPES_HPP
#define ZEST_MIME_TYPES_HPP

#include <string>

namespace zest {
namespace http {
namespace mime_types {

/// Convert a filename into a MIME type.
std::string filename_to_type(const std::string& extension);

} // namespace mime_types
} // namespace http
} // namespace zest

#endif // ZEST_MIME_TYPES_HPP
