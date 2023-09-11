//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HTTP_STREAM_REQUEST_HPP
#define BOOST_HTTP_STREAM_REQUEST_HPP

#include <boost/http-stream/stream.hpp>
#include <boost/beast/http/empty_body.hpp>
#include <boost/beast/http/write.hpp>
#include <boost/beast/http/read.hpp>

namespace boost
{
namespace http_stream
{

template<typename Stream>
auto request(Stream & str,
             verb method,
             beast::string_view target,
             unsigned int version,
             fields fld) -> basic_request_stream<typename Stream::executor_type>
{
  auto msg = std::make_unique<beast::http::request<detail::const_buffer_body>>(
      method, target, version,
      detail::const_buffer_body::value_type{},  std::move(fld));

  auto ser = std::make_unique<beast::http::request_serializer<detail::const_buffer_body>>(*msg);

  auto & res = *ser;

  beast::http::write_header(str, res);
  return basic_stream<true, typename Stream::executor_type>{
    std::move(ser), std::move(msg), detail::stream_ref{str}, str.get_executor()};
}

template<typename Stream, typename Buffer>
auto receive_request(Stream & str, Buffer & buffer) -> basic_response_stream<typename Stream::executor_type>
{
  beast::http::response_parser<beast::http::empty_body> p;
  beast::http::read_header(str, buffer, p);
  return basic_stream<true, typename Stream::executor_type>{
    std::move(p), buffer, detail::stream_ref{str, buffer}, str.get_executor()};
}

}
}

#endif //BOOST_HTTP_STREAM_REQUEST_HPP
