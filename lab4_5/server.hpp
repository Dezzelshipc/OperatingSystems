#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>
#include <fstream>

#if defined(WIN32)

#include <winsock2.h> /* socket */
#include <ws2tcpip.h> /* ipv6 */

#else

#include <sys/socket.h> /* socket */
#include <netinet/in.h> /* socket */
#include <arpa/inet.h>  /* socket */
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1

#endif

#define READ_WAIT_MS 50

namespace fs = std::filesystem;

namespace srvlib
{
    class Response
    {
    public:
        Response(const std::string &action, const std::string &url, std::string (*body_func)(void), bool is_raw = false)
            : m_action(action), m_url(url), m_body_func(body_func), m_is_raw(is_raw) {}

        std::string GetBody()
        {
            if (m_body_func == NULL)
            {
                return "";
            }
            return m_body_func();
        }

        static std::string GetAnswer(std::string body)
        {
            std::stringstream ans;
            ans << "HTTP/1.0 200 OK\r\n"
                << "Version: HTTP/1.1\r\n"
                << "Content-Type: text/html; charset=utf-8\r\n"
                << "Content-Length: " << body.length()
                << "\r\n\r\n"
                << body;
            return ans.str();
        }

        std::string GetAnswer()
        {
            return GetAnswer(GetBody());
        }

        std::string GetAction()
        {
            return m_action;
        }

        std::string GetURL()
        {
            return m_url;
        }

        bool IsRaw()
        {
            return m_is_raw;
        }

        // Searches files near .exe file and sends str_path if it finds or "" if not
        static std::string CheckAssets(const std::string file_path)
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
            else
            {
                try_paths.emplace_back("../assets");
                try_paths.emplace_back("./assets");
            }

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

        static std::string ReadFile(const std::string &file_path)
        {
            std::ifstream file(file_path);
            std::stringstream str;
            str << file.rdbuf() << "\n";
            return str.str();
        }

    protected:
        std::string m_action;
        std::string m_url;
        std::string (*m_body_func)(void);
        bool m_is_raw = false;
    };

    class ErrorResponse : public Response
    {
    public:
        static std::string GetBody()
        {
            return "<html>\n<head>"
                   "\t<meta/>\n"
                   "\t<title>Page Not Exists</title>\n"
                   "</head>\n\t<body>\n"
                   "\t\t<h1>Page Not Exists</h1>\n"
                   "</b></p>\n"
                   "\t</body>\n</html>";
        }

        static std::string GetAnswer()
        {
            auto body = GetBody();
            std::stringstream ans;
            ans << "HTTP/1.0 404 Not Found\r\n"
                << "Version: HTTP/1.1\r\n"
                << "Content-Type: text/html; charset=utf-8\r\n"
                << "Content-Length: " << body.length()
                << "\r\n\r\n"
                << body;
            return ans.str();
        }
    };

    class Parsed
    {
        std::string m_action;
        std::string m_url;

    public:
        Parsed(const std::string &recieved_data)
        {
            std::istringstream recv(recieved_data);
            std::string line;
            std::getline(recv, line);

            std::vector<std::string> split;
            std::istringstream iss(line);
            std::string s;
            while (getline(iss, s, ' '))
            {
                split.emplace_back(s);
            }

            m_action = split[0];
            m_url = split[1];
        }

        bool CheckResponse(Response response)
        {
            return response.GetAction() == m_action && response.GetURL() == m_url;
        }

        std::string GetAction()
        {
            return m_action;
        }

        std::string GetURL()
        {
            return m_url;
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
#if defined(WIN32)
            shutdown(sock, SD_SEND);
            closesocket(sock);
#else
            shutdown(sock, SHUT_WR);
            close(sock);
#endif
        }

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
        void Listen(const std::string &interface_ip, short int port)
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
            std::cout << "Listening to: http://" << interface_ip << ":" << port << std::endl;
        }

        void ProcessClient()
        {
            if (!IsValid())
            {
                std::cerr << "Server (listening) socket is invalid!" << std::endl;
                return;
            }

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

            auto parsed = Parsed(recv_str.str());

            std::cout << parsed.GetAction() << " " << parsed.GetURL() << std::endl;

            int index_r = -1;
            for (int i = 0; i < m_responses.size(); ++i)
            {
                if (parsed.CheckResponse(m_responses[i]))
                {
                    index_r = i;
                    break;
                }
            }

            std::string response;
            if (index_r != -1)
            {
                if (m_responses[index_r].IsRaw())
                {
                    response = m_responses[index_r].GetBody();
                }
                else
                {
                    response = m_responses[index_r].GetAnswer();
                }
            }
            else
            {
                std::string path = Response::CheckAssets(parsed.GetURL());
                if (!path.empty())
                {
                    response = Response::GetAnswer(Response::ReadFile(path));
                }
                else
                {
                    response = ErrorResponse::GetAnswer();
                }
            }

            // response += "<br><pre>" + recv_str.str() + "</pre>";
            std::cout << recv_str.str();

            // Отправляем ответ клиенту
            result = send(client_socket, response.c_str(), (int)response.length(), 0);
            if (result == SOCKET_ERROR)
            {
                // произошла ошибка при отправке данных
                std::cerr << "Failed to send responce to client: " << ErrorCode() << std::endl;
            }
            // Закрываем соединение к клиентом
            CloseSocket(client_socket);
            std::cout << "Answered to client!" << std::endl;
        }

        void RegisterResponses(std::vector<Response> responses)
        {
            m_responses = responses;
        }

    private:
        char m_input_buf[1024];

        std::vector<Response> m_responses;
    };

}