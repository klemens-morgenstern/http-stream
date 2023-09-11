//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HTTP_STREAM_RESPONSE_HPP
#define BOOST_HTTP_STREAM_RESPONSE_HPP

#include <boost/http-stream/stream.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/read.hpp>

namespace boost
{
namespace http_stream
{

template<typename Stream>
auto respond(Stream & str,
             status status_,
             unsigned int version,
             fields fld) -> basic_response_stream<typename Stream::executor_type>
{
  auto msg = std::make_unique<beast::http::response<detail::const_buffer_body>>(
      static_cast<status>(status_), version,
      detail::const_buffer_body::value_type{}, std::move(fld));

  beast::http::response_serializer<detail::const_buffer_body> res{*msg};

  beast::http::write_header(str, res);
  return basic_stream<false, typename Stream::executor_type>{std::move(res), str, str.get_executor()};
}


template<typename Stream>
auto respond(Stream & str,
             unsigned int status_,
             unsigned int version,
             fields fld) -> basic_response_stream<typename Stream::executor_type>
{
  auto msg = std::make_unique<beast::http::response<detail::const_buffer_body>>(
      static_cast<status>(status_), version,
      detail::const_buffer_body::value_type{}, std::move(fld));

  beast::http::response_serializer<detail::const_buffer_body> res{*msg};

  beast::http::write_header(str, res);
  return basic_stream<false, typename Stream::executor_type>{
    std::move(res), std::move(msg), detail::stream_ref{str}, str.get_executor()};
}


template<typename Stream, typename Buffer>
auto receive_response(Stream & str, Buffer & buffer) -> basic_response_stream<typename Stream::executor_type>
{
  beast::http::response_parser<beast::http::empty_body> p;
  beast::http::read_header(str, buffer, p);
  return basic_stream<false, typename Stream::executor_type>{
    std::move(p), buffer, detail::stream_ref{str, buffer}, str.get_executor()};
}

}
}

#endif //BOOST_HTTP_STREAM_RESPONSE_HPP
