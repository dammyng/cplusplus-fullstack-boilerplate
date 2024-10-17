#ifndef SESSION_HPP
#define SESSION_HPP

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/asio/strand.hpp>
#include <memory>
#include <string>
#include "handle_request.hpp"
#include "utility.hpp"
#include "IDatabase.hpp"


namespace beast = boost::beast;
namespace http = beast::http;
namespace net = boost::asio;

using tcp = net::ip::tcp;

// Handles an HTTP server connection
class session : public std::enable_shared_from_this<session> {
    beast::tcp_stream stream_;
    beast::flat_buffer buffer_;
    std::shared_ptr<std::string const> doc_root_;
    http::request<http::string_body> req_;
    std::shared_ptr<void> res_;
    std::shared_ptr<IDatabase> db_;

    // Define send_lambda inside session
    struct send_lambda {
        session& self_;

        explicit send_lambda(session& self) : self_(self) {}

        template <bool isRequest, class Body, class Fields>
        void operator()(http::message<isRequest, Body, Fields>&& msg) const {
            auto sp = std::make_shared<
                http::message<isRequest, Body, Fields>>(std::move(msg));

            self_.res_ = sp;

            // Write the response
            http::async_write(
                self_.stream_,
                *sp,
                beast::bind_front_handler(
                    &session::on_write,
                    self_.shared_from_this(),
                    sp->need_eof()));
        }
    };

    send_lambda lambda_;

public:
    // Constructor
    //session(tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root);
    session(tcp::socket&& socket, std::shared_ptr<std::string const> const& doc_root, std::shared_ptr<IDatabase> db);

    void run();

private:
    void do_read();
    void on_read(beast::error_code ec, std::size_t bytes_transferred);
    void on_write(bool close, beast::error_code ec, std::size_t bytes_transferred);
    void do_close();
};

#endif
