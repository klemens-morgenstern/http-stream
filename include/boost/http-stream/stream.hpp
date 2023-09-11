//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HTTP_STREAM_STREAM_HPP
#define BOOST_HTTP_STREAM_STREAM_HPP

#include <boost/http-stream/fields.hpp>
#include <boost/http-stream/detail/config.hpp>
#include <boost/http-stream/detail/buffer_body.hpp>
#include <boost/http-stream/detail/stream_ref.hpp>

#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/deferred.hpp>
#include <boost/asio/write.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/beast/http/parser.hpp>
#include <boost/beast/http/serializer.hpp>

#include <boost/variant2/variant.hpp>

namespace boost
{
namespace http_stream
{

template<bool IsRequest, typename Executor = asio::any_io_executor>
struct basic_stream
{
  const request_header & header() const &
  {

    switch (impl_.index())
    {
      case 0: return request_header{};
      case 1: return variant2::get<1u>(impl_).first.get();
      case 2: return variant2::get<2u>(impl_).first.get();
    }
  }

  request_header header() &&
  {
    {
      switch (impl_.index())
      {
        case 0: return request_header{};
        case 1: return std::move(variant2::get<1u>(impl_).get());
        case 2: return std::move(variant2::get<2u>(impl_).first->get());
      }
    }
  }
  bool is_done() const
  {
    switch (impl_.index())
    {
      default: return true;
      case 1: return variant2::get<1u>(impl_).is_done();
      case 2: return variant2::get<2u>(impl_).first->is_done();
    }
  }
  bool is_open() const
  {
    switch (impl_.index())
    {
      default: return false;
      case 1: return !variant2::get<1u>(impl_).is_done();
      case 2: return !variant2::get<2u>(impl_).first->is_done();
    }
  }

  using executor_type = Executor;
  executor_type get_executor() {return executor_;}

  template<typename MutableBufferSequence>
  std::size_t read_some(const MutableBufferSequence & buffers)
  {
    system::error_code ec;
    std::size_t s = read_some(buffers, ec);
    asio::detail::throw_error(ec, "read_some");
    return s;
  }

  template<typename MutableBufferSequence>
  std::size_t read_some(const MutableBufferSequence & buffers, system::error_code & ec)
  {
    auto itr = std::find_if(
        boost::asio::buffer_sequence_begin(buffers),
        boost::asio::buffer_sequence_end(buffers),
        [](asio::mutable_buffer cb) { return cb.size() != 0u;});

    if (is_done())
      BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::eof);
    else if (itr != boost::asio::buffer_sequence_end(buffers))
      BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::no_buffer_space);
    else if (impl_.index() != 1u)
      BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::operation_not_supported);

    if (ec)
      return 0u;

    auto & parser = variant2::get<1u>(impl_);
    parser.get().body().data = itr->data();
    parser.get().body().size = itr->size();
    return ref_.read_some(parser, ec);
  }

  template<
      typename MutableBufferSequence,
      BOOST_ASIO_COMPLETION_TOKEN_FOR(void (system::error_code, std::size_t)) ReadToken
      BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type)>
  auto async_read_some(const MutableBufferSequence & buffers,
                       ReadToken && token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(executor_type))
  {
    return asio::async_initiate<ReadToken, void(system::error_code, std::size_t)>(
        [](auto handler, basic_stream * this_, MutableBufferSequence buffers)
        {
          auto itr = std::find_if(
              boost::asio::buffer_sequence_begin(buffers),
              boost::asio::buffer_sequence_end(buffers),
              [](asio::mutable_buffer cb) { return cb.size() != 0u;});

          system::error_code ec;
          if (this_->is_done())
            BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::eof);
          else if (itr == boost::asio::buffer_sequence_end(buffers))
            BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::no_buffer_space);
          else if (this_->impl_.index() != 1u)
            BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::operation_not_supported);

          if (ec)
            return asio::dispatch(
                asio::get_associated_immediate_executor(handler, this_->get_executor()),
                asio::append(std::move(handler), ec, 0u));

          auto & pp = variant2::get<1u>(this_->impl_);
          auto & parser = pp.first;
          parser.get().body().data = itr->data();
          parser.get().body().size = itr->size();
          this_->ref_.async_read_some(pp.second, std::move(handler));
        });
  }

  template<typename ConstBufferSequence>
  std::size_t write_some(const ConstBufferSequence & buffers)
  {
    system::error_code ec;
    std::size_t s = write_some(buffers, ec);
    asio::detail::throw_error(ec, "write_some");
    return s;
  }

  template<typename ConstBufferSequence>
  std::size_t write_some(const ConstBufferSequence & buffers, system::error_code & ec)
  {
    auto itr = std::find_if(
        boost::asio::buffer_sequence_begin(buffers),
        boost::asio::buffer_sequence_end(buffers),
        [](asio::const_buffer cb) { return cb.size() != 0u;});

    if (is_done())
      BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::eof);
    else if (itr == boost::asio::buffer_sequence_end(buffers))
      BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::no_data);
    else if (impl_.index() != 2u)
      BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::operation_not_supported);

    if (ec)
      return 0u;

    auto & s = *variant2::get<2u>(impl_).first;
    s.get().body().more = true;
    s.get().body().data = itr->data();
    s.get().body().size = itr->size();
    return ref_.write_some(s, ec);
  }

  template<
      typename ConstBufferSequence,
      BOOST_ASIO_COMPLETION_TOKEN_FOR(void (system::error_code, std::size_t)) WriteToken
      BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type)>
  auto async_write_some(const ConstBufferSequence & buffers,
                       WriteToken && token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(executor_type))
  {
    return asio::async_initiate<WriteToken, void(system::error_code, std::size_t)>(
        [](auto handler, basic_stream * this_, ConstBufferSequence buffers)
        {
          auto itr = std::find_if(
              boost::asio::buffer_sequence_begin(buffers),
              boost::asio::buffer_sequence_end(buffers),
              [](asio::mutable_buffer cb) { return cb.size() != 0u;});

          system::error_code ec;

          if (this_->is_done())
            BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::eof);
          else if (itr == boost::asio::buffer_sequence_end(buffers))
            BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::no_buffer_space);
          else if (this_->impl_.index() != 2u)
            BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::operation_not_supported);

          if (ec)
              return asio::dispatch(
                asio::get_associated_immediate_executor(handler, this_->get_executor()),
                asio::append(std::move(handler), ec, 0u));

          auto & s = variant2::get<2u>(this_->impl_).first;
          s.get().body().more = true;
          s.get().body().data = itr->data();
          s.get().body().size = itr->size();
          return this_->ref_.async_write_some(s, std::move(handler));
        });
  }

  template<typename ConstBufferSequence>
  std::size_t write(const ConstBufferSequence & buffers)
  {
    system::error_code ec;
    std::size_t s = write(buffers, ec);
    asio::detail::throw_error(ec, "write");
    return s;
  }

  template<typename ConstBufferSequence>
  std::size_t write(const ConstBufferSequence & buffers, system::error_code & ec)
  {
    auto itr = std::find_if(
        boost::asio::buffer_sequence_begin(buffers),
        boost::asio::buffer_sequence_end(buffers),
        [](asio::const_buffer cb) { return cb.size() != 0u;});


    if (is_done())
      BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::eof);
    else if (itr == boost::asio::buffer_sequence_end(buffers))
      BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::no_data);
    else if (impl_.index() != 2u)
      BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::operation_not_supported);

    if (ec)
      return 0u;

    auto & s = *variant2::get<2u>(impl_).first;
    s.get().body().more = false;
    s.get().body().data = itr->data();
    s.get().body().size = itr->size();
    printf("Sz: %ld\n", itr->size());
    return ref_.write(s, ec);
  }

  template<
      typename ConstBufferSequence,
      BOOST_ASIO_COMPLETION_TOKEN_FOR(void (system::error_code, std::size_t)) WriteToken
      BOOST_ASIO_DEFAULT_COMPLETION_TOKEN_TYPE(executor_type)>
  auto async_write(const ConstBufferSequence & buffers,
                   WriteToken && token BOOST_ASIO_DEFAULT_COMPLETION_TOKEN(executor_type))
  {
    return asio::async_initiate<WriteToken, void(system::error_code, std::size_t)>(
        [](auto handler, basic_stream * this_, ConstBufferSequence buffers)
        {
          auto itr = std::find_if(
              boost::asio::buffer_sequence_begin(buffers),
              boost::asio::buffer_sequence_end(buffers),
              [](asio::mutable_buffer cb) { return cb.size() != 0u;});

          system::error_code ec;
          if (this_->is_done())
            BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::eof);
          else if (itr != boost::asio::buffer_sequence_end(buffers))
            BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::no_buffer_space);
          else if (this_->impl_.index() != 2u)
            BOOST_HTTP_STREAM_ASSIGN_EC(ec, asio::error::operation_not_supported);

          if (ec)
              return asio::dispatch(
                asio::get_associated_immediate_executor(handler, this_->get_executor()),
                asio::append(std::move(handler), ec, 0u));

          auto & s = *variant2::get<2u>(this_->impl_).first;
          s.get().body().more = false;
          s.get().body().data = itr->data();
          s.get().body().size = itr->size();
          return this_->ref_.async_write_some(s, std::move(handler));
        });
  }


  template<typename Executor1>
  struct rebind_executor
  {
    using other = basic_stream<IsRequest, Executor1>;
  };

  basic_stream(basic_stream && ) noexcept = default;

  template<typename Body>
  basic_stream(beast::http::parser<IsRequest, Body> && parser,
               detail::stream_ref ref,
               Executor exec)
        : impl_(variant2::in_place_index<1u>, std::move(parser)), ref_(ref), executor_(std::move(exec))
  {
  }

  basic_stream(std::unique_ptr<beast::http::serializer<IsRequest, detail::const_buffer_body>> && ser,
               std::unique_ptr<beast::http::message<IsRequest, detail::const_buffer_body>> msg,
               detail::stream_ref ref,
               Executor exec)
      : impl_(variant2::in_place_index<2u>, std::move(ser), std::move(msg)), ref_(ref), executor_(std::move(exec))
  {
  }

 private:



  variant2::variant<
    variant2::monostate,
    beast::http::parser<IsRequest, beast::http::buffer_body>,
    std::pair<
        std::unique_ptr<beast::http::serializer<IsRequest, detail::const_buffer_body>>,
        std::unique_ptr<beast::http::message<IsRequest, detail::const_buffer_body>>>> impl_;

  detail::stream_ref ref_;
  executor_type executor_;

};

template<typename Executor = asio::any_io_executor>
using basic_request_stream = basic_stream<true, Executor>;

template<typename Executor = asio::any_io_executor>
using basic_response_stream = basic_stream<false, Executor>;

using request_stream  = basic_request_stream<>;
using response_stream = basic_response_stream<>;

}
}

#endif //BOOST_HTTP_STREAM_STREAM_HPP
