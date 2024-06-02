#pragma once

#include <boost/beast/http/string_body.hpp>
#include <boost/beast/http/message.hpp>
#include <boost/function.hpp>

namespace ch {
namespace manager {
namespace http {

using RequestType = boost::beast::http::request<boost::beast::http::string_body>;

using ResponseType = boost::beast::http::response<boost::beast::http::string_body>;

using CreateResponseFunctorType = boost::function<ResponseType(const RequestType &a)>;

} // namespace http
} // namespace manager
} // namespace ch