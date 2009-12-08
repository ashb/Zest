/*
 * Copyright (c) 2009 Ash Berlin
 *
 * Distributed under the Boost Software License, Version 1.0. (See
 * http://www.boost.org/LICENSE_1_0.txt)
 *
 */

#include "connection.hpp"
#include "mime_types.hpp"
#include "request_handler.hpp"
#include "request.hpp"
#include "reply.hpp"
#include "zest.hpp"
#include <flusspferd/aliases.hpp>
#include <flusspferd/create_on.hpp>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/spirit/include/phoenix.hpp>
#include <string>
#include <fstream>
#include <iostream>

using namespace flusspferd;
using namespace flusspferd::aliases;
using namespace zest;

namespace phoenix = boost::phoenix;
namespace args = phoenix::arg_names;

request_handler::request_handler(zest_server &server)
  : _server(server)
{
}

object request_handler::build_jsgi_env(
  const request &req, connection &conn) {

  root_object env(create<object>());

  // env.jsgi --------------------------------
  object jsgi = create<object>(
    _name = "jsgi",
    _container = env, 
    _attributes = no_property_flag);

  create_on(jsgi)
    .create<array>("version", list_of(0)(3), _attributes = no_property_flag);

  jsgi.set_properties(
    "errors",
    global().call("require", "system")
            .to_object()
            .get_property("stderr"))
    ("multithread", false)
    ("multiprocess", false)
    ("runonce", false);

  // end env.jsgi ----------------------------

  env.set_properties("method", req.method)
    ("scriptName", "")
    ("pathInfo", req.uri)
    ("queryString", req.query_string)
    ("scheme", "http")
    ("remoteAddr", conn.socket().remote_endpoint().address().to_string())
    ("authType", object())
    ("version", create<array>(list_of(req.http_version_major)(req.http_version_minor)));


  object input = create<object>(_container = env, _name="input", _attributes = no_property_flag);

  // Create a callback closing over the socket.
  create<function>(
    _container = input,
    _name = "read",
    _function = boost::function<object (std::size_t)>(
      phoenix::bind(&connection::read_from_body, conn, args::arg1)
    ),
    _attributes = no_property_flag
  );


  // env.headers
  object headers = create<object>(
    _name = "header",
    _container = env,
    _attributes = no_property_flag
  );
  bool host_seen = false;

  BOOST_FOREACH(header h, req.headers) {
    string hdr_str;

    boost::to_lower(h.name);
    if (h.name == "host") {
      host_seen = true;
      std::size_t last_pos = h.value.find_last_of(":");
      if (last_pos != std::string::npos) {
        env.set_properties("host", h.value.substr(0, last_pos))
                          ("port", h.value.substr(last_pos+1));
      }
      else {
        env.set_properties("host", h.value)("port", "");
      }
    }

    if (headers.has_own_property(h.name)) {
      // already exists, combined as specified by RFC 2616
      // TODO: This should probably create/push to an array instead
      hdr_str = string::concat(
          string::concat( headers.get_property(h.name), "," ),
          h.value
      );
    }
    else {
      hdr_str = h.value;
    }
    headers.set_property(h.name, hdr_str);
  }

  if (!host_seen) {
    boost::asio::ip::tcp::endpoint local = conn.socket().local_endpoint();
    // TODO: This is very wrong
    env.set_properties
      ("host", local.address().to_string())
      ("port", boost::lexical_cast<std::string>(local.port()));
  }


  return env;
}

void request_handler::handle_request(
  const request &req, reply &rep,
  connection &conn)
{
  // Turn the req into the JSGI env.
  object env = build_jsgi_env(req, conn);

  // Root it! Make sure it doesn't get GC'd when we are sending, cos that would
  // be really bad
  try {
    root_value rv(_server._handler_cb.call(_server, env));

    if (rv.is_undefined_or_null() || !rv.is_object()) {
      std::cerr << "no res or res is not an object" << std::endl;
      rep = reply::stock_reply(reply::internal_server_error);
      return;
    }

    object jsgi_res = rv.to_object();

    value v = jsgi_res.get_property("status");
    int status;

    if (!v.is_int() || (status = v.get_int())  < 100) {
      std::cerr << "res.status is not a valid HTTP status code" << std::endl;
      rep = reply::stock_reply(reply::internal_server_error);
      return;
    }

    rep.status = reply::status_type(status);

    // Populate headers
    object hdrs = jsgi_res.get_property_object("headers");

    bool send_file = false;
    std::string x_sendfile;
    if (( send_file = hdrs.has_property("x-sendfile")) == true) {
      x_sendfile = hdrs.get_property("x-sendfile").to_std_string();
      hdrs.delete_property("x-sendfile");
    }

    bool content_type_set = false;
    for ( property_iterator iter = hdrs.begin(), end = hdrs.end(); iter != end; ++iter) {
      header h;
      h.name = iter->to_std_string();

      // If using X-SendFile, ignore any content length headers
      if (send_file && boost::to_lower_copy(h.name) == "content-length")
        continue;
      // If using X-SendFile, use Content-Type if provided
      if (send_file && boost::to_lower_copy(h.name) == "content-type")
        content_type_set = true;

      h.value = hdrs.get_property(*iter).to_std_string();
      rep.headers.push_back(h);
    }

    // If X-SendFile, ignore body.
    if (send_file) {
      serve_file(rep, x_sendfile, !content_type_set);
      return;
    }

    v = jsgi_res.get_property("body");
    if (v.is_undefined_or_null() || !v.is_object()) {
      std::cerr << "res.body is not an object" << std::endl;
      rep = reply::stock_reply(reply::internal_server_error);
      return;
    }

    object body = v.get_object();
    v = body.get_property("forEach");
    if (v.is_undefined_or_null() || !v.is_function()) {
      std::cerr << "res.body.forEach is not a function" << std::endl;
      rep = reply::stock_reply(reply::internal_server_error);
      return;
    }

    // Create a callback closure to pass to forEach
    root_function body_writer(create<function>(
      _name = "Zest.writeChunk",
      _function = boost::function<void (value)>(
        phoenix::bind(&reply::body_appender, rep, args::arg1)
      )
    ));

    body.call("forEach", body_writer);
    return;
  }
  catch (exception &e) {
    std::cerr << "JSGI Error happened" << std::endl;
    std::cerr << e.what() << std::endl;
  }
  catch (...) {
    std::cerr << "Unknown error" << std::endl;
  }
  rep = reply::stock_reply(reply::internal_server_error);
}


void request_handler::serve_file(reply &rep,
                  std::string const &fname, bool add_content_type) {

  std::ifstream is(fname.c_str(), std::ios::in | std::ios::binary);
  if (!is)
  {
    rep = reply::stock_reply(reply::not_found);
    return;
  }

  char buf[512];
  while (is.read(buf, sizeof(buf)).gcount() > 0)
    rep.content.append(buf, is.gcount());
  header h;
  h.name = "Content-Length";
  h.value = boost::lexical_cast<std::string>(rep.content.size());
  rep.headers.push_back(h);
  if (add_content_type) {
    h.name = "Content-Type";
    h.value = mime_types::filename_to_type(fname);
    rep.headers.push_back(h);
  }
}
