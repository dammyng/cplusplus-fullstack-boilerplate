#include "path_cat.hpp"
#include <boost/config.hpp>

namespace beast = boost::beast;

std::string path_cat(beast::string_view base, beast::string_view path) {
    if (base.empty())
        return std::string(path);
    std::string result(base);

#ifdef BOOST_MSVC
    char constexpr path_separator = '\\';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
    for (auto& c : result)
        if (c == '/')
            c = path_separator;
#else
    char constexpr path_separator = '/';
    if (result.back() == path_separator)
        result.resize(result.size() - 1);
    result.append(path.data(), path.size());
#endif
    return result;
}