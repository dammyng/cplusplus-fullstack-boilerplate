// path_cat.hpp
#ifndef PATH_CAT_HPP
#define PATH_CAT_HPP

#include <boost/beast/core.hpp>
#include <string>

std::string path_cat(boost::beast::string_view base, boost::beast::string_view path);

#endif 