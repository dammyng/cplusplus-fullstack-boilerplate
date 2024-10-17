#include "session.hpp"
/*
session::session(
    tcp::socket&& socket,
    std::shared_ptr<std::string const> const& doc_root)
    : stream_(std::move(socket)),
      doc_root_(doc_root),
      lambda_(*this) // Initialize lambda_ with *this
{
}
*/

session::session(
    tcp::socket&& socket,
    std::shared_ptr<std::string const> const& doc_root, 
    std::shared_ptr<IDatabase> db)
    : stream_(std::move(socket)),  doc_root_(doc_root),  db_(db),  lambda_(*this)
{
}


void session::run() {
    net::dispatch(
        stream_.get_executor(),
        beast::bind_front_handler(
            &session::do_read,
            shared_from_this()));
}

void session::do_read() {
    req_ = {};

    stream_.expires_after(std::chrono::seconds(30));

    http::async_read(
        stream_,
        buffer_,
        req_,
        beast::bind_front_handler(
            &session::on_read,
            shared_from_this()));
}

void session::on_read(
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec == http::error::end_of_stream)
        return do_close();

    if (ec)
        return fail(ec, "read");

    // Send the response
    handle_request(*doc_root_, std::move(req_), lambda_, db_);
    //handle_request(*doc_root_, std::move(req_), lambda_);
}

void session::on_write(
    bool close,
    beast::error_code ec,
    std::size_t bytes_transferred)
{
    boost::ignore_unused(bytes_transferred);

    if (ec)
        return fail(ec, "write");

    if (close) {
        return do_close();
    }

    res_ = nullptr;

    do_read();
}

void session::do_close() {
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);
}
