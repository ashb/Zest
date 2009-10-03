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

#include "mime_types.hpp"
#include <boost/algorithm/string/predicate.hpp>

using namespace boost::algorithm;

namespace zest {
namespace mime_types {

struct mapping
{
  const char* extension;
  const char* mime_type;
} mappings[] =
{
  { ".pdf", "application/pdf" },
  { ".sig", "application/pgp-signature" },
  { ".spl", "application/futuresplash" },
  { ".class", "application/octet-stream" },
  { ".ps", "application/postscript" },
  { ".torrent", "application/x-bittorrent" },
  { ".dvi", "application/x-dvi" },
  { ".gz", "application/x-gzip" },
  { ".pac", "application/x-ns-proxy-autoconfig" },
  { ".swf", "application/x-shockwave-flash" },
  { ".tar.gz", "application/x-tgz" },
  { ".tgz", "application/x-tgz" },
  { ".tar", "application/x-tar" },
  { ".zip", "application/zip" },
  { ".mp3", "audio/mpeg" },
  { ".m3u", "audio/x-mpegurl" },
  { ".wma", "audio/x-ms-wma" },
  { ".wax", "audio/x-ms-wax" },
  { ".ogg", "application/ogg" },
  { ".wav", "audio/x-wav" },
  { ".gif", "image/gif" },
  { ".jar", "application/x-java-archive" },
  { ".jpg", "image/jpeg" },
  { ".jpeg", "image/jpeg" },
  { ".png", "image/png" },
  { ".xbm", "image/x-xbitmap" },
  { ".xpm", "image/x-xpixmap" },
  { ".xwd", "image/x-xwindowdump" },
  { ".css", "text/css" },
  { ".html", "text/html" },
  { ".htm", "text/html" },
  { ".js", "text/javascript" },
  { ".asc", "text/plain" },
  { ".c", "text/plain" },
  { ".cpp", "text/plain" },
  { ".log", "text/plain" },
  { ".conf", "text/plain" },
  { ".text", "text/plain" },
  { ".txt", "text/plain" },
  { ".dtd", "text/xml" },
  { ".xml", "text/xml" },
  { ".mpeg", "video/mpeg" },
  { ".mpg", "video/mpeg" },
  { ".mov", "video/quicktime" },
  { ".qt", "video/quicktime" },
  { ".avi", "video/x-msvideo" },
  { ".asf", "video/x-ms-asf" },
  { ".asx", "video/x-ms-asf" },
  { ".wmv", "video/x-ms-wmv" },
  { ".bz2", "application/x-bzip" },
  { ".tbz", "application/x-bzip-compressed-tar" },
  { ".tar.bz2", "application/x-bzip-compressed-tar" },
  // default mime type
  { "", "application/octet-stream" },

  { 0, 0 } // Marks end of list.
};

std::string filename_to_type(const std::string& filename)
{
  for (mapping* m = mappings; m->extension; ++m)
  {
    if (iends_with(filename, m->extension))
    {
      return m->mime_type;
    }
  }

  return "application/octet-stream";
}

} // namespace mime_types
} // namespace zest
