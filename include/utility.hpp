#ifndef UTILITY_HPP
#define UTILITY_HPP

#include <boost/beast/core.hpp>
#include <iostream>

void fail(boost::beast::error_code ec, char const* what);

#endif // UTILITY_HPP