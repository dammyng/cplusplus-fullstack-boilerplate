// listener.hpp
#ifndef LISTENER_HPP
#define LISTENER_HPP

#include <boost/asio.hpp>
#include <memory>
#include "session.hpp"
#include "utility.hpp"
#include "IDatabase.hpp"

namespace net = boost::asio;
using tcp = net::ip::tcp;

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener> {
    net::io_context& ioc_;
    tcp::acceptor acceptor_;
    std::shared_ptr<std::string const> doc_root_;
    std::shared_ptr<IDatabase> db_;
public:
    //listener(
    //    net::io_context& ioc,
    //    tcp::endpoint endpoint,
    //    std::shared_ptr<std::string const> const& doc_root);

    listener(
        net::io_context& ioc,
        tcp::endpoint endpoint,
        std::shared_ptr<std::string const> const& doc_root,
        std::shared_ptr<IDatabase> db);

    void run();

private:
    void do_accept();
    void on_accept(beast::error_code ec, tcp::socket socket);
};

#endif