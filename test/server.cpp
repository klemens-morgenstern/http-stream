//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "server.hpp"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <iostream>

using namespace boost;

namespace http = beast::http;

void run_server(boost::asio::local::stream_protocol::socket & s)
try
{
  while (s.is_open())
  {
    beast::flat_buffer buf;
    http::request<beast::http::string_body> req;

    auto n = http::read(s, buf, req);
    std::cout << "Request [" << n << "]: " << req << std::endl;

    http::response<http::string_body> res{http::status::ok, req.version()};

    http::response_serializer<http::string_body> ser{res};

    res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
    res.keep_alive(req.keep_alive());
    res.body() = req.body();
    if (req.target() == "/chunked")
      res.prepare_payload();
    else
      res.chunked(true);

    http::write(s, res);
  }
}
catch(std::exception &e)
{
  std::cout << "Ex: " << e.what() << std::endl;
}