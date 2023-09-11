//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HTTP_STREAM_DETAIL_STREAM_REF_HPP
#define BOOST_HTTP_STREAM_DETAIL_STREAM_REF_HPP

#include <boost/http-stream/detail/buffer_body.hpp>

#include <boost/asio/any_completion_handler.hpp>
#include <boost/asio/associated_immediate_executor.hpp>
#include <boost/asio/append.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/write.hpp>

#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>
#include <boost/beast/http/read.hpp>
#include <boost/beast/http/write.hpp>

namespace boost
{
namespace http_stream
{
namespace detail
{

struct stream_ref
{
  struct vtable
  {
    std::size_t (*read_some_request) (void * , void* , beast::http::request_parser <beast::http::buffer_body> &, system::error_code &);
    std::size_t (*read_some_response)(void * , void* , beast::http::response_parser<beast::http::buffer_body> &, system::error_code &);

    void (*async_read_some_request) (asio::any_completion_handler<void(system::error_code, std::size_t)>,
                                     void * , void *, beast::http::request_parser <beast::http::buffer_body> *);
    void (*async_read_some_response)(asio::any_completion_handler<void(system::error_code, std::size_t)>,
                                     void * , void *, beast::http::response_parser<beast::http::buffer_body> *);


    std::size_t (*write_some_request) (void * , beast::http::request_serializer <const_buffer_body> &, system::error_code &);
    std::size_t (*write_some_response)(void * , beast::http::response_serializer<const_buffer_body> &, system::error_code &);
    std::size_t (*write_request)      (void * , beast::http::request_serializer <const_buffer_body> &, system::error_code &);
    std::size_t (*write_response)     (void * , beast::http::response_serializer<const_buffer_body> &, system::error_code &);

    void (*async_write_some_request) (asio::any_completion_handler<void(system::error_code, std::size_t)>,
                                      void * , beast::http::request_serializer<const_buffer_body> *);
    void (*async_write_some_response)(asio::any_completion_handler<void(system::error_code, std::size_t)>,
                                      void * , beast::http::response_serializer<const_buffer_body> *);

    void (*async_write_request) (asio::any_completion_handler<void(system::error_code, std::size_t)>,
                                 void * , beast::http::request_serializer<const_buffer_body> *);
    void (*async_write_response)(asio::any_completion_handler<void(system::error_code, std::size_t)>,
                                 void * , beast::http::response_serializer<const_buffer_body> *);
  };

  template<typename Stream>
  stream_ref(Stream & str) : impl_(&str), vtable_(vtable_impl_<Stream>()) {}

  template<typename Stream, typename Buffer>
  stream_ref(Stream & str, Buffer & buffer)
    : impl_(&str), read_buffer_(&buffer), vtable_(vtable_impl_<Stream, Buffer>()) {}


  stream_ref(stream_ref & str) : impl_(str.impl_), vtable_(str.vtable_) {}


  std::size_t read_some(beast::http::request_parser <beast::http::buffer_body> & p, system::error_code & ec)
  {
    return vtable_.read_some_request(impl_, read_buffer_, p, ec);
  }

  std::size_t read_some(beast::http::response_parser <beast::http::buffer_body> & p, system::error_code & ec)
  {
    return vtable_.read_some_response(impl_, read_buffer_, p, ec);
  }

  template<typename ReadToken>
  void async_read_some(beast::http::request_parser <beast::http::buffer_body> & p,
                       ReadToken && read_token)
  {
    return asio::async_initiate<ReadToken, void(system::error_code, std::size_t)>(
        vtable_.async_read_some_request, impl_,  read_buffer_, &p);
  }

  template<typename ReadToken>
  void async_read_some(beast::http::response_parser <beast::http::buffer_body> & p,
                       ReadToken && read_token)
  {
    return asio::async_initiate<ReadToken, void(system::error_code, std::size_t)>(
        vtable_.async_read_some_request, impl_,  read_buffer_, &p);
  }

  std::size_t write_some(beast::http::request_serializer <const_buffer_body> & ser, system::error_code & ec)
  {
    return vtable_. write_some_request(impl_, ser, ec);
  }
  std::size_t write_some(beast::http::response_serializer<const_buffer_body> & ser, system::error_code & ec)
  {
    return vtable_. write_some_response(impl_, ser, ec);
  }

  std::size_t write(beast::http::request_serializer <const_buffer_body> & ser, system::error_code & ec)
  {
    return vtable_. write_request(impl_, ser, ec);
  }
  std::size_t write(beast::http::response_serializer<const_buffer_body> & ser, system::error_code & ec)
  {
    return vtable_. write_response(impl_, ser, ec);
  }

  template<typename WriteToken>
  void async_write_some(beast::http::request_serializer<const_buffer_body> & ser,
                        WriteToken && write_token)
  {
    return asio::async_initiate<WriteToken, void(system::error_code, std::size_t)>(
        vtable_.async_write_some_request, impl_,  &ser);
  }

  template<typename WriteToken>
  void async_write_some(beast::http::response_serializer<const_buffer_body> & ser,
                        WriteToken && write_token)
  {
    return asio::async_initiate<WriteToken, void(system::error_code, std::size_t)>(
        vtable_.async_write_some_response, impl_,  &ser);
  }


  template<typename WriteToken>
  void async_write(beast::http::request_serializer<const_buffer_body> & ser, WriteToken && write_token)
  {
    return asio::async_initiate<WriteToken, void(system::error_code, std::size_t)>(
        vtable_.async_write_request, impl_,  &ser);
  }

  template<typename WriteToken>
  void async_write(beast::http::response_serializer<const_buffer_body> & ser, WriteToken && write_token)
  {
    return asio::async_initiate<WriteToken, void(system::error_code, std::size_t)>(
        vtable_.async_write_response, impl_,  &ser);
  }

 private:
  template<typename Stream, typename Buffer = beast::flat_buffer>
  vtable & vtable_impl_()
  {
    static vtable vt{
      /*read_some_request*/[](void * this_, void * buffer,
                            beast::http::request_parser<beast::http::buffer_body> & p, system::error_code & ec)
       {
          return beast::http::read_some(*static_cast<Stream*>(this_), *static_cast<Buffer*>(buffer), p, ec);
       },
      /*read_some_response*/[](void * this_, void * buffer,
                             beast::http::response_parser<beast::http::buffer_body> & p,  system::error_code & ec)
      {
          return beast::http::read_some(*static_cast<Stream*>(this_), *static_cast<Buffer*>(buffer), p, ec);
      },
      /*async_read_some_request*/[](asio::any_completion_handler<void(system::error_code, std::size_t)> handler,
                                  void * this_ , void * buffer,
                                  beast::http::request_parser <beast::http::buffer_body> * p)
      {
          beast::http::async_read_some(*static_cast<Stream*>(this_), *static_cast<Buffer*>(buffer), *p, std::move(handler));
      },
      /*async_read_some_response*/[](asio::any_completion_handler<void(system::error_code, std::size_t)> handler,
                                  void * this_ , void * buffer,
                                  beast::http::response_parser <beast::http::buffer_body> * p)
      {
        beast::http::async_read_some(*static_cast<Stream*>(this_), *static_cast<Buffer*>(buffer), *p, std::move(handler));
      },
      /*write_some_request*/ [](void * this_, beast::http::request_serializer <const_buffer_body> & ser, system::error_code & ec) ->  std::size_t
      {
        return beast::http::write_some(*static_cast<Stream*>(this_), ser, ec);
      },
      /*write_some_response*/ [](void * this_, beast::http::response_serializer<const_buffer_body> & ser, system::error_code & ec) ->  std::size_t
      {
        return beast::http::write_some(*static_cast<Stream*>(this_), ser, ec);

      },
      /*write_request*/ [](void * this_, beast::http::request_serializer <const_buffer_body> & ser, system::error_code & ec) ->  std::size_t
      {
        return beast::http::write(*static_cast<Stream*>(this_), ser, ec);
      },
      /*write_response*/ [](void * this_, beast::http::response_serializer<const_buffer_body> & ser , system::error_code & ec) ->  std::size_t
      {
        return beast::http::write(*static_cast<Stream*>(this_), ser, ec);
      },
      /*async_write_some_request */ [] (asio::any_completion_handler<void(system::error_code, std::size_t)> handler, void * this_, beast::http::request_serializer<const_buffer_body> * ser)
      {
        beast::http::async_write_some(*static_cast<Stream*>(this_), *ser, std::move(handler));

      },
      /*async_write_some_response */ [](asio::any_completion_handler<void(system::error_code, std::size_t)> handler, void * this_, beast::http::response_serializer<const_buffer_body> * ser)
      {
        beast::http::async_write_some(*static_cast<Stream*>(this_), *ser, std::move(handler));
      },
      /*async_write_request */ [] (asio::any_completion_handler<void(system::error_code, std::size_t)> handler, void * this_, beast::http::request_serializer<const_buffer_body> * ser)
      {
        beast::http::async_write(*static_cast<Stream*>(this_), *ser, std::move(handler));
      },
      /*async_write_response */ [](asio::any_completion_handler<void(system::error_code, std::size_t)> handler, void * this_, beast::http::response_serializer<const_buffer_body> * ser)
      {
        beast::http::async_write(*static_cast<Stream*>(this_), *ser, std::move(handler));
      }
    };

    return vt;

  }


  void * impl_;
  void * read_buffer_;
  vtable & vtable_;
};

}
}
}

#endif //BOOST_HTTP_STREAM_DETAIL_STREAM_REF_HPP
