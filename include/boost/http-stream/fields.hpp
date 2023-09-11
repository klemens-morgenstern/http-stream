//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HTTP_STREAM_FIELDS_HPP
#define BOOST_HTTP_STREAM_FIELDS_HPP

#include <boost/beast/http/field.hpp>
#include <boost/core/detail/string_view.hpp>

#include <boost/beast/http/fields.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/beast/http/status.hpp>
#include <boost/beast/http/verb.hpp>

#include <initializer_list>

namespace boost
{
namespace http_stream
{


using boost::beast::http::field;
using boost::beast::http::status;
using boost::beast::http::status_class;
using boost::beast::http::to_status_class;
using boost::beast::http::to_string;
using boost::beast::http::verb;

template<typename Allocator = std::allocator<char>>
struct basic_fields : boost::beast::http::basic_fields<Allocator>
{
  struct entry
  {
    http_stream::field field = http_stream::field::unknown;
    core::string_view key;
    core::string_view value;

    entry() = default;
    entry(http_stream::field field, core::string_view value) : field(field), value(value) {}
    entry(core::string_view key, core::string_view value) : key(key), value(value) {}
  };

  using boost::beast::http::basic_fields<Allocator>::operator=;
  using boost::beast::http::basic_fields<Allocator>::basic_fields;

  basic_fields(std::initializer_list<entry> inits,
               Allocator allocator = {}) : boost::beast::http::basic_fields<Allocator>(std::move(allocator))
  {
    for (const auto & init : inits)
      if (init.field != field::unknown)
        this->set(init.field, init.value);
      else
        this->set(init.key, init.value);
  }
};

using fields = basic_fields<>;


using request_header  = beast::http::request_header <>;
using response_header = beast::http::response_header<>;

}
}

#endif //BOOST_HTTP_STREAM_FIELDS_HPP
