// mime_types.hpp
#ifndef MIME_TYPES_HPP
#define MIME_TYPES_HPP

#include <boost/beast/core.hpp>

boost::beast::string_view mime_type(boost::beast::string_view path);

#endif