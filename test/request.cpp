//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include <boost/http-stream.hpp>
#include <boost/beast/http/buffer_body.hpp>
#include <boost/asio/local/stream_protocol.hpp>
#include <boost/asio/local/connect_pair.hpp>

#include <thread>

#include "server.hpp"
#include "doctest.h"

using namespace boost;

TEST_CASE("dummy")
{
  asio::io_context ctx, ctx2;
  asio::local::stream_protocol::socket s{ctx}, server{ctx2};
  asio::local::connect_pair(s, server);

  std::thread thr{[&]{run_server(server);}};
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  auto rq = boost::http_stream::request(
      s, http_stream::verb::get, "/dummy",  11,
      {{"Foo", "Bar"}, {beast::http::field::content_length, "6"}});

  CHECK(rq.write_some(boost::asio::buffer("xyz", 3)) == 3u);
  CHECK(!rq.is_done());
  CHECK(rq.write(boost::asio::buffer("zyx", 3)) == 3u);
  CHECK(rq.is_done());
  thr.join();
}