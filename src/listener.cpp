#include "listener.hpp"

listener::listener(
    net::io_context& ioc,
    tcp::endpoint endpoint,
    std::shared_ptr<std::string const> const& doc_root,
    std::shared_ptr<IDatabase> db )
    : ioc_(ioc),  acceptor_(net::make_strand(ioc)), doc_root_(doc_root),db_(db)
{
    beast::error_code ec;

    acceptor_.open(endpoint.protocol(), ec);
    if (ec) {
        fail(ec, "open");
        return;
    }

    acceptor_.set_option(net::socket_base::reuse_address(true), ec);
    if (ec) {
        fail(ec, "set_option");
        return;
    }

    // Bind to the server address
    acceptor_.bind(endpoint, ec);
    if (ec) {
        fail(ec, "bind");
        return;
    }

    // Start listening for connections
    acceptor_.listen(net::socket_base::max_listen_connections, ec);
    if (ec) {
        fail(ec, "listen");
        return;
    }
}

void listener::run() {
    do_accept();
}

void listener::do_accept() {
    acceptor_.async_accept(
        net::make_strand(ioc_),
        beast::bind_front_handler(
            &listener::on_accept,
            shared_from_this()));
}

void listener::on_accept(beast::error_code ec, tcp::socket socket) {
    if (ec) {
        fail(ec, "accept");
    } else {
        // Create the session and run it
        //std::make_shared<session>(
        //    std::move(socket),
        //    doc_root_)->run();

        std::make_shared<session>(
            std::move(socket),
            doc_root_, 
            db_)->run();
    }
    do_accept();
}