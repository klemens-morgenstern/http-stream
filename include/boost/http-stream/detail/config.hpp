//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HTTP_STREAM_DETAIL_CONFIG_HPP
#define BOOST_HTTP_STREAM_DETAIL_CONFIG_HPP

#define BOOST_HTTP_STREAM_ASSIGN_EC(ec, error)                                       \
    do {                                                                             \
      static constexpr auto BOOST_PP_CAT(loc_, __LINE__) ((BOOST_CURRENT_LOCATION)); \
      ec.assign(error, & BOOST_PP_CAT(loc_, __LINE__) );                             \
    }                                                                                \
    while (false)

#endif //BOOST_HTTP_STREAM_DETAIL_CONFIG_HPP
