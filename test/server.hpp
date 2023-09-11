//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HTTP_STREAM_SERVER_HPP
#define BOOST_HTTP_STREAM_SERVER_HPP

#include <boost/asio/local/stream_protocol.hpp>

void run_server(boost::asio::local::stream_protocol::socket & sock);

#endif //BOOST_HTTP_STREAM_SERVER_HPP
