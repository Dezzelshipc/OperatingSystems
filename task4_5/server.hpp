#pragma once

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <fstream>
#include <unordered_map>

#include "utility.hpp"

#ifdef WIN32

#include <winsock2.h> /* socket */
#include <ws2tcpip.h> /* getaddrinfo */

#else

#include <sys/socket.h> /* socket */
#include <sys/types.h>
#include <netinet/in.h> /* socket */
#include <arpa/inet.h>  /* socket */
#include <unistd.h>
#include <netdb.h> /* getaddrinfo */
#include <poll.h>  /* poll */
#include <signal.h>
#include <string.h> /* memset */
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#endif

namespace srvlib
{
#define READ_WAIT_MS 50
#define DEFAULT_HTTP_VERSION "HTTP/1.1"
    namespace fs = std::filesystem;

    // Находит тип MIME для правильной отправки контента
    std::string MimeTypeFromString(const std::string &str)
    {
        std::string szResult = "application/unknown";
#ifdef WIN32
        std::string szExtension = str.substr(str.find_last_of('.'), std::string::npos);

        HKEY hKey = NULL;

        // open registry key
        if (RegOpenKeyEx(HKEY_CLASSES_ROOT, szExtension.c_str(),
                         0, KEY_READ, &hKey) == ERROR_SUCCESS)
        {
            // define buffer
            char szBuffer[256] = {0};
            DWORD dwBuffSize = sizeof(szBuffer);

            // get content type
            if (RegQueryValueEx(hKey, "Content Type", NULL, NULL,
                                (LPBYTE)szBuffer, &dwBuffSize) == ERROR_SUCCESS)
            {
                // success
                szResult = szBuffer;
            }

            // close key
            RegCloseKey(hKey);
        }
#else
        auto resp = utillib::Exec(("file --mime-type -b " + str).c_str());
        // "cannot open" если файл не существует, "ERROR", если cmd не открылась
        if (resp.find("cannot open") == std::string::npos && resp.find("ERROR") == std::string::npos)
        {
            szResult = resp;
        }
#endif
        szResult = utillib::Trim(szResult);
        std::cout << szResult << std::endl;
        return szResult;
    }

    // Ищет файл в папках рядом с .exe и выводит его путь, если он нашёлся, либо "", если нет
    std::string FindAssets(const std::string file_path)
    {
        std::string path = file_path;

        if (path.find(".") == std::string::npos)
        {
            if (!path.ends_with("/"))
            {
                path += "/";
            }
            path += "index.html";
        }

        std::vector<std::string> try_paths;
        if (path.ends_with(".html"))
        {
            try_paths.emplace_back("../html");
            try_paths.emplace_back("./html");
        }
        else if (path.ends_with(".css"))
        {
            try_paths.emplace_back("../css");
            try_paths.emplace_back("./css");
        }
        else if (path.ends_with(".js"))
        {
            try_paths.emplace_back("../js");
            try_paths.emplace_back("./js");
        }

        try_paths.emplace_back("../assets");
        try_paths.emplace_back("./assets");
        try_paths.emplace_back("..");
        try_paths.emplace_back(".");

        for (auto &p : try_paths)
        {
            auto pp = p + path;
            if (fs::exists(pp))
            {
                return pp;
            }
        }
        return "";
    }

    class SpecialResponse;

    // Распарсенный запрос клиента
    class Request
    {
        std::string m_method;
        std::string m_url;
        std::string m_file_of_url;

        std::string m_version;
        std::unordered_map<std::string, std::string> m_headers;
        std::unordered_map<std::string, std::string> m_url_args;

    public:
        Request(const std::string &recieved_data)
        {
            std::istringstream recv(recieved_data);
            std::string line;
            std::getline(recv, line);

            auto split = utillib::Split(utillib::Trim(line), " ");
            auto url_split = utillib::Split(split[1], "?");

            m_method = split[0];
            m_url = url_split[0];
            if (url_split.size() > 1)
            {
                for (auto args : utillib::Split(url_split[1], "&"))
                {
                    auto sp = utillib::Split(args, "=");
                    m_url_args[sp[0]] = sp[1];
                }
            }
            m_version = split[2];

            m_file_of_url = FindAssets(m_url);

            while (true)
            {
                if (!std::getline(recv, line) || line == "\r" || line == "\r\n")
                {
                    break;
                }
                split = utillib::Split(utillib::Trim(line), ": ");
                m_headers[split[0]] = split[1];
            }
        }

        // Проверяет, что метод и URL запроса совпадают для специального ответа
        bool CheckResponse(SpecialResponse response) const;

        std::string GetMethod() const
        {
            return m_method;
        }

        std::string GetURL() const
        {
            return m_url;
        }

        std::string GetFileURL() const
        {
            return m_file_of_url;
        }

        std::string GetVersion() const
        {
            return m_version;
        }

        std::string GetHeader(const std::string &key) const
        {
            return m_headers.at(key);
        }
    };

    // Ответ сервера, готовый для отправки
    class Response
    {
    protected:
        std::string m_response_type;
        std::string m_content_type;
        std::string m_version;

    public:
        Response(const std::string &response_type, const std::string &content_type, const std::string &version)
            : m_response_type(response_type), m_content_type(content_type), m_version(version) {}

        Response(const std::string &response_type, const std::string &content_type)
            : Response(response_type, content_type, DEFAULT_HTTP_VERSION) {}

        Response(const std::string &response_type) : Response(response_type, "text/plain") {}

        Response() : Response("200 OK") {}

        Response(const Request &request) : Response()
        {
            m_version = request.GetVersion();
            SetContentType(MimeTypeFromString(request.GetFileURL()));
        }

        Response SetVersion(const std::string &version)
        {
            m_version = version;
            return *this;
        }

        Response SetResponseType(const std::string &response_type)
        {
            m_response_type = response_type;
            return *this;
        }

        Response SetContentType(const std::string &content_type)
        {
            m_content_type = content_type;
            return *this;
        }

        // Строка с минимальным хедером и телом ответа
        std::string GetAnswer(const std::string &body) const
        {
            std::stringstream ans;
            ans << m_version << " " << m_response_type << "\r\n"
                << "Content-Type: " << m_content_type << "\r\n" //; charset=utf-8
                << "Content-Length: " << body.length()
                << "\r\n\r\n"
                << body;
            return ans.str();
        }
    };

    // Спациальный ответ, тело которого определяется функцией void -> string [string f(void)]
    class SpecialResponse : public Response
    {
    protected:
        std::string m_method;
        std::string m_url;
        std::string (*m_body_func)(void);
        bool m_is_raw;

    public:
        SpecialResponse(const std::string &method, const std::string &url, std::string (*body_func)(void), bool is_raw = false)
            : Response(), m_method(method), m_url(url), m_body_func(body_func), m_is_raw(is_raw) {}

        std::string GetBody()
        {
            if (m_body_func == NULL)
            {
                return "";
            }
            return m_body_func();
        }

        std::string GetAnswer()
        {
            return Response::GetAnswer(GetBody());
        }

        std::string GetMethod()
        {
            return m_method;
        }

        std::string GetURL()
        {
            return m_url;
        }

        // true, если ответ требуется без хедера (сырые данные)
        bool IsRaw()
        {
            return m_is_raw;
        }
    };

    bool Request::CheckResponse(SpecialResponse response) const
    {
        return response.GetMethod() == GetMethod() && response.GetURL() == GetURL();
    }

    // Ответ 404
    class ErrorResponse : public Response
    {

    public:
        ErrorResponse() : Response("404 Not Found") {}

        std::string GetAnswer() const
        {
            auto error_page_path = FindAssets("/404.html");
            if (error_page_path != "")
            {
                return Response::GetAnswer(utillib::ReadFile(error_page_path));
            }
            return "";
        }
    };

    class SocketBase
    {
    public:
        SocketBase() : m_socket(INVALID_SOCKET)
        {
#ifdef WIN32
            WSADATA wsaData;
            WSAStartup(MAKEWORD(2, 2), &wsaData);
#else
            // Игнорируем SIGPIPE сигнал
            // чтобы программа не терминировалась при попытке записи в закрытый сокет
            signal(SIGPIPE, SIG_IGN);
#endif
        }
        ~SocketBase()
        {
            CloseSocket();
#if defined(WIN32)
            WSACleanup();
#endif
        }

        static int ErrorCode()
        {
#ifdef WIN32
            return WSAGetLastError();
#else
            return errno;
#endif
        }

        bool IsValid()
        {
            return m_socket != INVALID_SOCKET;
        }

    protected:
        void CloseSocket()
        {
            CloseSocket(m_socket);
            m_socket = INVALID_SOCKET;
        }

        static void CloseSocket(SOCKET sock)
        {
            if (sock == INVALID_SOCKET)
                return;
#ifdef WIN32
            shutdown(sock, SD_SEND);
            closesocket(sock);
#else
            shutdown(sock, SHUT_WR);
            close(sock);
#endif
        }

        // Если < 0, то сокет не готов к общению
        static int Poll(const SOCKET &socket, int timeout_ms = READ_WAIT_MS)
        {
            struct pollfd polstr;
            memset(&polstr, 0, sizeof(polstr));
            polstr.fd = socket;
            polstr.events |= POLLIN;
#ifdef WIN32
            return WSAPoll(&polstr, 1, timeout_ms);
#else
            return poll(&polstr, 1, timeout_ms);
#endif
        }

        SOCKET m_socket;
    };

    class HTTPServer : public SocketBase
    {
    public:
        HTTPServer(const std::string &interface_ip, const short int port)
        {
            Listen(interface_ip, port);
        }

        // Создать сокет сервера для прослушивания
        void Listen(const std::string &interface_ip, const short int port)
        {
            if (m_socket != INVALID_SOCKET)
            {
                CloseSocket();
            }

            struct addrinfo hints;
            memset(&hints, 0, sizeof(hints));

            hints.ai_family = AF_INET;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_protocol = IPPROTO_TCP;
            hints.ai_flags = AI_PASSIVE;

            struct addrinfo *addr = NULL;
            int res = getaddrinfo(interface_ip.c_str(), std::to_string(port).c_str(), &hints, &addr);

            if (res != 0)
            {
                std::cerr << "Failed getaddrinfo: " << res << std::endl;
                freeaddrinfo(addr);
                return;
            }

            // Создаем сокет ipv4
            m_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
            if (m_socket == INVALID_SOCKET)
            {
                std::cerr << "Cant open socket: " << ErrorCode() << std::endl;
                freeaddrinfo(addr);
            }

            // Биндим сокет на адрес и порт
            if (bind(m_socket, addr->ai_addr, addr->ai_addrlen) == SOCKET_ERROR)
            {
                std::cerr << "Failed to bind: " << ErrorCode() << std::endl;
                freeaddrinfo(addr);
                CloseSocket();
            }
            freeaddrinfo(addr);

            // Запускаем прослушку на сокете
            if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
            {
                std::cerr << "Failed to start listen: " << ErrorCode() << std::endl;
                CloseSocket();
            }
        }

        // Обрабатывает запрос клиента, если никаких ошибок не произошло
        void ProcessClient()
        {
            if (!IsValid())
            {
                std::cerr << "Server (listening) socket is invalid!" << std::endl;
                return;
            }

            // Ожидает пока не примет запрос
            SOCKET client_socket = accept(m_socket, NULL, NULL);
            if (client_socket == INVALID_SOCKET)
            {
                std::cerr << "Error accepting client: " << ErrorCode() << std::endl;
                CloseSocket(client_socket);
                return;
            }

            if (Poll(client_socket) <= 0)
            {
                CloseSocket(client_socket);
                return;
            }

            std::stringstream recv_str;

            int buf_size = sizeof(m_input_buf) - 1;

            // Читаем поток данных
            int result = -1;
            do
            {
                result = recv(client_socket, m_input_buf, buf_size, 0);
                if (result < 0)
                {
                    break;
                }
                m_input_buf[result] = '\0';
                recv_str << m_input_buf;
            } while (result >= buf_size);

            if (result == SOCKET_ERROR || result < 0)
            {
                std::cerr << "Error on client receive: " << ErrorCode() << std::endl;
                CloseSocket(client_socket);
                return;
            }
            else if (result == 0)
            {
                std::cerr << "Client closed connection before getting any data!" << std::endl;
                CloseSocket(client_socket);
                return;
            }

            auto request = Request(recv_str.str());

            std::cout << request.GetMethod() << " " << request.GetURL() << " " << request.GetFileURL() << std::endl;

            // Пытаемся найти специальный ответ SpecialResponse
            int index_r = -1;
            for (uint64_t i = 0; i < m_sp_responses.size(); ++i)
            {
                if (request.CheckResponse(m_sp_responses[i]))
                {
                    index_r = i;
                    break;
                }
            }

            std::string response;
            // Если нашли, то отправляем
            if (index_r != -1)
            {
                if (m_sp_responses[index_r].IsRaw())
                {
                    response = m_sp_responses[index_r].GetBody();
                }
                else
                {
                    response = m_sp_responses[index_r].GetAnswer();
                }
            }
            else
            {
                // Если нет, то ищем файл в папках рядом с .exe (уже не/нашло при создании request)
                std::string path = request.GetFileURL();
                if (!path.empty())
                {
                    response = Response(request).GetAnswer(utillib::ReadFile(path));
                }
                else
                {
                    // Если у нас лапки, и ничего вообще не нашлось, то отправляем 404
                    response = error_response.GetAnswer();
                }
            }

            std::cout << recv_str.str() << "\n\n";

            // Отправляем ответ клиенту
            result = send(client_socket, response.c_str(), (int)response.length(), 0);
            if (result == SOCKET_ERROR)
            {
                // Произошла ошибка при отправке данных
                std::cerr << "Failed to send responce to client: " << ErrorCode() << std::endl;
            }
            // Закрываем соединение к клиентом
            CloseSocket(client_socket);
            std::cout << "Answered to client!" << std::endl;
        }

        void RegisterResponses(std::vector<SpecialResponse> responses)
        {
            m_sp_responses = responses;
        }

    private:
        char m_input_buf[1024];

        std::vector<SpecialResponse> m_sp_responses;
        ErrorResponse error_response;
    };

}