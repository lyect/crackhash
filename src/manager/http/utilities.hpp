#pragma once

#include "manager/http/types.hpp"

namespace ch {
namespace manager {
namespace http {

ResponseType makeEmptyResponse(
    const RequestType &request);

void makeMethodNotAllowedResponse(
    http::ResponseType &response,
    const std::string_view &method,
    const std::string_view &target);

void makeInvalidJsonResponse(
    ResponseType &response,
    const std::string_view &reason);

void makeInvalidParameters(
    ResponseType &response, 
    const std::string_view &reason);

void makeServerError(
    http::ResponseType &response,
    const std::string_view &error);

} // namespace http
} // namespace manager
} // namespace ch