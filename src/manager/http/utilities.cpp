#include "utilities.hpp"

#include "tp/nlohmann/json.hpp"

namespace ch {
namespace manager {
namespace http {

ResponseType makeEmptyResponse(const RequestType &request) {
    ResponseType response;
    response.version(request.version());
    response.set(boost::beast::http::field::content_type, "application/json");
    return response;
}

void makeMethodNotAllowedResponse(
    http::ResponseType &response,
    const std::string_view &method,
    const std::string_view &target
) {
    response.keep_alive(false);
    response.result(boost::beast::http::status::method_not_allowed);

    std::stringstream ss;
    ss << "'" << method << "' is not allowed on " << target;

    nlohmann::json jsonResponseBody = {
        {"response", ss.str()}
    };

    response.body() = jsonResponseBody.dump();
}

void makeInvalidJsonResponse(
    ResponseType &response,
    const std::string_view &reason
) {
    response.keep_alive(false);
    response.result(boost::beast::http::status::unprocessable_entity);

    std::stringstream ss;
    ss << "Invalid JSON in body: " << reason;

    nlohmann::json jsonResponseBody = {
        {"response", ss.str()}
    };

    response.body() = jsonResponseBody.dump();
}

void makeInvalidParameters(
    ResponseType &response, 
    const std::string_view &reason
) {
    response.keep_alive(false);
    response.result(boost::beast::http::status::bad_request);

    std::stringstream ss;
    ss << "Invalid parameters in URL: " << reason;

    nlohmann::json jsonResponseBody = {
        {"response", ss.str()}
    };

    response.body() = jsonResponseBody.dump();
}

void makeServerError(
    http::ResponseType &response,
    const std::string_view &error
) {
    response.keep_alive(false);
    response.result(boost::beast::http::status::internal_server_error);

    nlohmann::json jsonResponseBody = {
        {"response", error}
    };

    response.body() = jsonResponseBody.dump();
}

} // namespace http
} // namespace manager
} // namespace ch