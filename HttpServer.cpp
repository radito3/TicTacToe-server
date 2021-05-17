#include "HttpServer.h"
#include <iostream>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <regex>
#include <ranges>
#include <sstream>
#include <utility>
#include <vector>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define check_err(func) if (errno != 0) \
                            perror(#func);

class socket_exception : public std::runtime_error {
public:
    explicit socket_exception(const std::string &arg) : runtime_error(arg) {}

    explicit socket_exception(const char *string) : runtime_error(string) {}
};

template<typename... Args>
void log(Args&&... args) {
    if (sizeof...(args) < 1) {
        return;
    }
    using std::chrono::system_clock;
    std::time_t tt = system_clock::to_time_t(system_clock::now());
    struct std::tm* ptm = std::localtime(&tt);

    std::clog << "[cpp-server] [" << std::put_time(ptm, "%a %b %d %T %Z %Y") << "] ";
    ((std::clog << args), ...);
    std::clog << std::endl;
}

HttpServer::HttpServer(HttpServer::Config server_config)
    : packet_size(server_config.packet_size),
      socket_connection_queue_size(server_config.socket_connection_queue_size),
      socket_read_timeout_millis(server_config.socket_read_timeout_millis),
      connection_pool(server_config.th_pool_config)
{
    create_tcp_socket();
    bind_socket();
}

HttpServer::~HttpServer() {
    log("Closing socket...");
    close(socket_fd);
    check_err(close);
    log("Socket closed");
}

void HttpServer::start() {
    if (listen(socket_fd, socket_connection_queue_size) != 0) {
        throw socket_exception("Error listening on socket: " + std::string(strerror(errno)));
    }
    log("Listening for connections...");

    while (true) {
        auto [connection_fd, connection_address] = accept_connection();
        if (connection_fd < 0) {
            continue;
        }

        //when persistent connection support is added,
        // change this to read and write in a loop on a separate thread
        // (which internally reads from and sends to socket in coroutines)
        std::stringstream packets;
        //TODO move this to async job so as to not block the main server thread
        bool timeout = read_data_from_socket(connection_fd, packets);
        if (timeout) {
            send_response_to_socket(connection_fd, HttpResponse::new_builder()
                                                    .status(408)
                                                    .build());
        }
        //TODO check for the length of the url
        // if it is too long (e.g. more than 8000 symbols (encoded)) return 414 URI Too Long
        HttpRequest request = parse_http_request(packets);

        auto handler = find_request_handler(request);
        if (handler == handlers.end()) {
            send_response_to_socket(connection_fd, HttpResponse::new_builder()
                                                    .status(404)
                                                    .build());
            continue;
        }

        connection_pool.submit_job(HandleConnectionJob(connection_fd, *handler, request));
    }
}

void HttpServer::create_tcp_socket() {
    log("Creating socket...");
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        throw socket_exception("Error creating socket: " + std::string(strerror(errno)));
    }
    log("Socket with fd ", socket_fd, " created");
}

void HttpServer::bind_socket() {
    struct sockaddr_in serveraddr;
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serveraddr.sin_port = htons(80);

    log("Binding socket to local address...");
    if (bind(socket_fd, (struct sockaddr*) &serveraddr, sizeof(serveraddr)) != 0) {
        throw socket_exception("Error binding socket: " + std::string(strerror(errno)));
    }
    log("Socket bound");
}

std::pair<int, struct sockaddr_in> HttpServer::accept_connection() {
    struct sockaddr_in connaddr;
    socklen_t connlen;
    int connfd = accept(socket_fd, (struct sockaddr*) &connaddr, &connlen);

    if (connfd >= 0) {
        log("Connection established");
    } else {
        switch (connfd) {
            case ENOBUFS:
            case ENOMEM:
            case EPERM:
                throw socket_exception("Error accepting connection: " + std::string(strerror(errno)));
            case EINTR:
            case ECONNABORTED:
            default:
                return {-1, connaddr};
        }
    }
    return {connfd, connaddr};
}

HttpRequest HttpServer::parse_http_request(std::istream &packets) {
    std::string http_method;
    packets >> http_method;
    HttpMethod method = parse_http_method(http_method);

    std::string url;
    packets >> url;

    std::string protocol;
    packets >> protocol;

    std::string line;
    std::multimap<std::string_view, std::string_view> headers;
    std::getline(packets, line);
    while (!line.empty()) {
        std::regex colon(R"(:\s+)");
        std::vector<std::string> header(std::sregex_token_iterator(line.begin(), line.end(), colon, -1),
                                        std::sregex_token_iterator());
        headers.insert({header[0], header[1]});
        std::getline(packets, line);
    }

    std::string body;
    while (packets) {
        char byte;
        packets.read(&byte, 1);
        if (byte == '\0') {
            break;
        }
        body += byte;
    }

    return HttpRequest(method, url, protocol, headers, body);
}

HttpMethod HttpServer::parse_http_method(const std::string_view& method) {
    if (method == "GET") {
        return HttpMethod::GET;
    }
    if (method == "POST") {
        return HttpMethod::POST;
    }
    if (method == "PUT") {
        return HttpMethod::PUT;
    }
    if (method == "OPTIONS") {
        return HttpMethod::OPTIONS;
    }
    if (method == "PATCH") {
        return HttpMethod::PATCH;
    }
    if (method == "DELETE") {
        return HttpMethod::DELETE;
    }
    if (method == "HEAD") {
        return HttpMethod::HEAD;
    }
    if (method == "CONNECT") {
        return HttpMethod::CONNECT;
    }
    if (method == "TRACE") {
        return HttpMethod::TRACE;
    }
    throw std::runtime_error("Invalid HTTP method");
}

auto HttpServer::find_request_handler(const HttpRequest &request) -> handlers_map::iterator {
    std::string path(request.url);
    if (path.find('?') != std::string::npos) {
        path.erase(path.find('?'));
    }
    if (path.find('#') != std::string::npos) {
        path.erase(path.find('#'));
    }

    for (auto& [matcher, handler] : handlers) {
        std::string matcher_path(matcher.path);
        if (matcher_path.find('{') != std::string::npos) {
            matcher_path = std::regex_replace(matcher_path, std::regex(R"({\w+})"), R"([-\w\d_]+)");
        }

        if (std::regex_match(path, std::regex(matcher_path))) {
            return handlers.find(matcher);
        }
    }
    return handlers.end();
}

bool HttpServer::check_for_client_timeout(int connection_fd) const {
    std::condition_variable timer;
    std::mutex t_mutex;
    unsigned packet_size_ = this->packet_size;

    std::thread([connection_fd, &timer, packet_size_]() {
        char packet[packet_size_];
        recv(connection_fd, (void *) packet, packet_size_, MSG_PEEK);
        timer.notify_one();
    }).detach();

    std::unique_lock<std::mutex> lock(t_mutex);
    if (timer.wait_for(lock, std::chrono::milliseconds(socket_read_timeout_millis)) == std::cv_status::timeout) {
        return true;
    }
    return false;
}

bool HttpServer::read_data_from_socket(int connection_fd, std::iostream &packets) {
    bool timeout = check_for_client_timeout(connection_fd);
    if (timeout) {
        return true;
    }

    ssize_t bytes_received = -1;
    char packet[packet_size];
    memset(packet, '\0', sizeof(packet));

    log("Receiving data from connection with fd ", connection_fd, "...");
    while ((bytes_received = recv(connection_fd, (void *) packet, packet_size, 0)) != -1) {
        if (bytes_received == 0) { //EOF
            break;
        }
        //we first need to examine the start of the request (start-line + headers),
        // determine what to do,
        // then decide whether we need to read more (i.e. rest of body)
        // if not, we need to free the memory that the body takes
        packets << packet;
        if (bytes_received < packet_size) {
            break;
        }
        memset(packet, '\0', sizeof(packet));
    }
    log("Data received");
    return false;
}

HttpResponse HttpServer::add_mandatory_headers_to_response(const HttpResponse &response) {
    auto result = HttpResponse::copy_from(response);
    using std::chrono::system_clock;
    std::time_t tt = system_clock::to_time_t(system_clock::now());
    struct std::tm* ptm = std::localtime(&tt);
    std::stringstream time;
    time << std::put_time(ptm, "%a %b %d %T %Z %Y");

    result.header("Date", time.str());
    result.header("Server", "cpp-server/0.1.0 (Alpine Linux)");
    //when switching to reading from and sending to a connection in a coroutine,
    // consider redesigning to allow for persistent connections
    // in which case, this header will be omitted
    result.header("Connection", "close");
    if (!result.contains_header("Content-Type") && result.contains_header("Content-Length")) {
        result.header("Content-Type", "text/plain");
    }
    return result.build();
}

void HttpServer::send_response_to_socket(int connection_fd, HttpResponse response) {
    auto res = add_mandatory_headers_to_response(response);
    char* response_str = res.to_c_str();
    size_t response_size = strlen(response_str);

    log("Sending data to connection with fd ", connection_fd, "...");
    ssize_t bytes_sent = -1;
    while ((bytes_sent = send(connection_fd, (void *) response_str, response_size, 0)) != -1) {
        if (bytes_sent >= response_size) {
            break;
        }
        response_str += bytes_sent;
        response_size -= bytes_sent;
    }
    log("Data sent");
    delete [] response_str;

    log("Closing connection...");
    close(connection_fd);
    log("Connection closed");
}

void remove_leading_trailing_slash(std::string& path) {
    if (path.find('/') == 0) {
        path.erase(path.begin());
    }
    if (path.rfind('/') == path.size() - 1) {
        path.erase(--path.end());
    }
}

std::string strip_leading_trailing_brace(const std::string& str) {
    return str.substr(1, str.size() - 1);
}

template <typename T>
std::string to_string(const T& view) {
    auto common = view | std::views::common;
    return std::string(common.begin(), common.end());
}

std::unordered_map<std::string_view, std::string_view> extract_path_params(const RequestMatcher& matcher,
                                                                           const HttpRequest& request) {
    if (matcher.path.find('{') == std::string_view::npos) {
        return {};
    }
    std::string matcher_path(matcher.path);
    std::string request_url(request.url);
    remove_leading_trailing_slash(matcher_path);
    remove_leading_trailing_slash(request_url);

    auto path_elems = matcher_path | std::views::split('/');
    auto url_elems = request_url | std::views::split('/');
    std::unordered_map<std::string_view, std::string_view> path_params;

    for (auto url_it = url_elems.begin(); const auto& path_view : path_elems) {
        auto path_elem = to_string(path_view);
        if (path_elem.find('{') != std::string::npos) {
            path_params.insert({ strip_leading_trailing_brace(path_elem), to_string(*url_it) });
        }
        ++url_it;
    }
    return path_params;
}

using r_iter = std::sregex_token_iterator;

std::multimap<std::string_view, std::string_view> extract_query_params(const HttpRequest& request) {
    if (request.url.find('?') == std::string_view::npos) {
        return {};
    }
    std::string query_url(request.url.substr(request.url.find('?') + 1));
    if (query_url.rfind('#') != std::string::npos) {
        query_url = query_url.substr(0, query_url.find('#'));
    }
    std::regex query_delims(R"([&;])");
    std::vector<std::string> query_elems(r_iter(query_url.begin(), query_url.end(), query_delims, -1), r_iter());
    std::multimap<std::string_view, std::string_view> query_params;
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

std::string_view extract_fragment(const HttpRequest& request) {
    if (request.url.rfind('#') == std::string_view::npos) {
        return {};
    }
    return request.url.substr(request.url.rfind('#') + 1);
}

HttpServer::HandleConnectionJob::HandleConnectionJob(int connection_fd, const handlers_map::value_type &pair,
                                                     const HttpRequest& request)
        : connection_fd(connection_fd), pair(pair),
        //TODO handle url encoding
        request_context(request.headers,
                        extract_path_params(pair.first, request),
                        extract_query_params(request),
                        extract_fragment(request),
                        request.body) {}

void HttpServer::HandleConnectionJob::operator()() {
    //these will be used for creating the gRPC stubs
    //TODO add them to query map
//    char *connection_address = inet_ntoa(connaddr.sin_addr);
//    unsigned short connection_port = connaddr.sin_port;

    HttpResponse response;
    try {
        response = pair.second(request_context);
    } catch (const std::runtime_error& e) {
        response = HttpResponse::new_builder()
                    .status(500)
                    .body(e.what())
                    .build();
    }
    send_response_to_socket(connection_fd, response);
}
