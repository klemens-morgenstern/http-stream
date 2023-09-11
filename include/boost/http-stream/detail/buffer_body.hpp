//
// Copyright (c) 2023 Klemens Morgenstern (klemens.morgenstern@gmx.net)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#ifndef BOOST_HTTP_STREAM_DETAIL_BUFFER_BODY_HPP
#define BOOST_HTTP_STREAM_DETAIL_BUFFER_BODY_HPP

#include <cstddef>
#include <boost/asio/buffer.hpp>
#include <boost/beast/http/message.hpp>

namespace boost
{
namespace http_stream
{
namespace detail
{

// non retarded version

struct const_buffer_body
{
    struct value_type
    {
        const void* data = nullptr;
        std::size_t size = 0;
        bool more = true;
    };

    class writer
    {
        value_type & body_;
    public:
        using const_buffers_type =
            boost::asio::const_buffer;

        template<bool isRequest, class Fields>
        explicit
        writer(boost::beast::http::header<isRequest, Fields> &, value_type & b)
            : body_(b)
        {
        }

        void
        init(system::error_code& ec)
        {
            ec = {};
        }

        boost::optional<
            std::pair<const_buffers_type, bool>>
        get(system::error_code& ec)
        {
            printf("HEyaaa %ld:%d - %s\n", body_.size, body_.more, body_.data);
            if (body_.size == 0 && !body_.more)
            {
                BOOST_BEAST_ASSIGN_EC(ec, beast::http::error::need_buffer);
                return boost::none;
            }

            const_buffers_type res{body_.data, body_.size};
            body_.data = nullptr;
            body_.size = 0u;
            ec.clear();
            return std::make_pair(res, body_.more);
        }
    };
};
}
}
}

#endif //BOOST_HTTP_STREAM_DETAIL_BUFFER_BODY_HPP
