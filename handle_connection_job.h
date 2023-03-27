#ifndef TICTACTOE_SERVER_HANDLE_CONNECTION_JOB_H
#define TICTACTOE_SERVER_HANDLE_CONNECTION_JOB_H

#include <utility>
#include <ranges>
#include <unordered_map>
#include <map>
#include <compare>
#include <memory>
#include "connection.h"

namespace detail {
    class HandleConnectionJob {
        std::shared_ptr<Connection> connection;
        using handlers_map = std::unordered_map<RequestMatcher, std::function<HttpResponse(const HttpRequestContext &)>>;
        const handlers_map* handlers;

    public:
        HandleConnectionJob(std::shared_ptr<Connection> conn, const handlers_map* handlers)
                : connection(std::move(conn)), handlers(handlers) {}

        void operator()() {
            //wrap this in a try-catch block
            auto [method, path] = connection->read_method_and_path();

            auto [match_err, handler] = find_request_handler(method, path);
            if (handler == nullptr) {
                if (match_err.path_not_found) {
                    connection->send_response(HttpResponse::new_builder()
                                                      .status(404) //Not Found
                                                      .build());
                    return;
                }
                connection->send_response(HttpResponse::new_builder()
                                                  .status(405) //Method Not Allowed
                                                  .build());
                return;
            }

            //when persistent connection support is added,
            // change this to read and write in a loop on a separate thread
            // (which internally reads from and sends to socket in coroutines)
            HttpRequest request;
            try {
                request = connection->read_request();
            } catch (const std::runtime_error& e) {
                connection->send_response(HttpResponse::new_builder()
                                                  .status(408)
                                                  .build());
                return;
            }

            //these will be used for creating the gRPC stubs
            char *conn_address = inet_ntoa(connection->get_connection_address().sin_addr);
            unsigned short connection_port = connection->get_connection_address().sin_port;

            auto context = HttpRequestContext::new_builder()
                    .client_address(std::string(conn_address) + ":" + std::to_string(connection_port))
                    .headers(request.headers)
                    .path_params(extract_path_params(request))
                    .query_params(extract_query_params(request))
                    .fragment(extract_fragment(request))
                    .body(request.body) //change to body reader
                    .build();

            HttpResponse response;
            try {
                response = handler(context);
            } catch (const std::runtime_error& e) {
                response = HttpResponse::new_builder()
                        .status(500)
                        .body(e.what())
                        .build();
            }
            connection->send_response(response);
        }

        void on_rejected() {
            connection->send_response(HttpResponse::new_builder()
                                              .status(503)
                                              .build());
        }

    private:
        template <typename T>
        static std::string to_string(const T& view) {
            auto common = view | std::views::common;
            return std::string(common.begin(), common.end());
        }

        static void remove_leading_trailing_slash(std::string& path) {
            if (path.front() == '/') {
                path.erase(path.begin());
            }
            if (path.back() == '/') {
                path.pop_back();
            }
        }

        static std::string strip_leading_trailing_brace(const std::string& str) {
            return str.substr(1, str.size() - 2);
        }

        static std::unordered_map<std::string, std::string> extract_path_params(const HttpRequest& request) {
            if (request.url.find('{') == std::string::npos) {
                return {};
            }
            std::string matcher_path(request.url);
            std::string request_url(request.raw_url);
            remove_leading_trailing_slash(matcher_path);
            remove_leading_trailing_slash(request_url);

            auto path_elems = matcher_path | std::views::split('/');
            auto url_elems = request_url | std::views::split('/');
            std::unordered_map<std::string, std::string> path_params;

            for (auto url_it = url_elems.begin(); const auto& path_view : path_elems) {
                auto path_elem = to_string(path_view);
                if (path_elem.find('{') != std::string::npos) {
                    path_params.emplace(strip_leading_trailing_brace(path_elem), to_string(*url_it));
                }
                ++url_it;
            }
            return path_params;
        }

        static std::multimap<std::string, std::string> extract_query_params(const HttpRequest& request) {
            using r_iter = std::sregex_token_iterator;

            if (request.url.find('?') == std::string::npos) {
                return {};
            }
            std::string query_url(request.url.substr(request.url.find('?') + 1));
            if (query_url.rfind('#') != std::string::npos) {
                query_url = query_url.substr(0, query_url.rfind('#'));
            }
            std::regex query_delims("[&;]");
            std::vector<std::string> query_elems(r_iter(query_url.begin(), query_url.end(), query_delims, -1), r_iter());
            std::multimap<std::string, std::string> query_params;

            for (auto& query_elem : query_elems) {
                if (query_elem.find('=') == std::string::npos) {
                    query_params.insert({query_elem, query_elem});
                    continue;
                }
                auto query_elem_parts = query_elem | std::views::split('=');
                auto it = query_elem_parts.begin();
                auto key = to_string(*it);
                auto val = to_string(*++it);
                query_params.insert({key, val});
            }
            return query_params;
        }

        static std::string extract_fragment(const HttpRequest& request) {
            if (request.url.rfind('#') == std::string::npos) {
                return "";
            }
            return request.url.substr(request.url.rfind('#') + 1);
        }

        struct match_error {
            bool path_not_found;
            bool method_not_allowed;
        };

        //try to simplify this?
        //rename this
        static match_error* path_matches(const RequestMatcher& matcher, HttpMethod method, const std::string& path) {
            auto strip_trailing_fw_slash = [] (const std::string& str) -> std::string {
                if (str.size() > 1 && str.back() == '/') {
                    return str.substr(0, str.size() - 1);
                }
                return str;
            };

            if (std::ranges::count(matcher.path, '/') != std::ranges::count(strip_trailing_fw_slash(path), '/')) {
                return new match_error{true, false};
            }
            auto matcher_parts = matcher.path | std::views::split('/');
            auto path_parts = path | std::views::split('/');

            for (auto path_it = std::ranges::begin(path_parts); const auto& matcher_part_view : matcher_parts) {
                std::string matcher_part = to_string(matcher_part_view);
                if (matcher_part.find('{') == std::string::npos && (matcher_part <=> to_string(*path_it)) != 0) {
                    return new match_error{true, false};
                }
                ++path_it;
            }
            if (matcher.method != method) {
                return new match_error{false, true};
            }
            return nullptr;
        }

        auto find_request_handler(HttpMethod method, std::string path) -> std::tuple<match_error, std::function<HttpResponse(const HttpRequestContext &)>> {
            if (path.find('?') != std::string::npos) {
                path.erase(path.find('?'));
            }
            if (path.find('#') != std::string::npos) {
                path.erase(path.find('#'));
            }

            for (const auto& [matcher, handler] : *handlers) {
                auto* match_err = path_matches(matcher, method, path);
                //rework this
                // if there is a matched path but disallowed method, return, rather than continuing the loop
                if (!match_err) {
                    return {{}, handler};
                }
                delete match_err;
            }
            return {{true, true}, nullptr};
        }
    };
}

#endif //TICTACTOE_SERVER_HANDLE_CONNECTION_JOB_H
