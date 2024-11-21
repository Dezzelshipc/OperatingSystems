#include <iostream>
#include <string>
#include <sstream>

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
    // Регистрируем сокет на прослушку подключения
    // void Listen(const std::string &interface_ip, short int port)
    // {
    //     if (m_socket != INVALID_SOCKET)
    //     {
    //         CloseSocket();
    //     }
    //     // Создаем сокет ipv4
    //     m_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    //     if (m_socket == INVALID_SOCKET)
    //     {
    //         std::cerr << "Cant open socket: " << ErrorCode() << std::endl;
    //         // return INVALID_SOCKET;
    //     }
    //     // Биндим сокет на адрес и порт
    //     sockaddr_in local_addr;
    //     memset(&local_addr, 0, sizeof(local_addr));
    //     local_addr.sin_family = AF_INET;
    //     local_addr.sin_addr.s_addr = inet_addr(interface_ip.c_str());
    //     local_addr.sin_port = htons(port);
    //     if (bind(m_socket, (struct sockaddr *)&local_addr, sizeof(local_addr)))
    //     {
    //         std::cerr << "Failed to bind: " << ErrorCode() << std::endl;
    //         CloseSocket();
    //         // return INVALID_SOCKET;
    //     }
    //     // Запускаем прослушку на сокете
    //     if (listen(m_socket, SOMAXCONN) == SOCKET_ERROR)
    //     {
    //         std::cerr << "Failed to start listen: " << ErrorCode() << std::endl;
    //         CloseSocket();
    //         // return INVALID_SOCKET;
    //     }
    //     // return m_socket;
    // }

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

    // void ProcessClient(const std::string data_str)
    // {
    //     if (!IsValid())
    //     {
    //         std::cerr << "Server (listening) socket is invalid!" << std::endl;
    //         return;
    //     }

    //     SOCKET client_socket = accept(m_socket, NULL, NULL);
    //     if (client_socket == INVALID_SOCKET)
    //     {
    //         std::cerr << "Error accepting client: " << ErrorCode() << std::endl;
    //         CloseSocket(client_socket);
    //         return;
    //     }

    //     // Хочет ли клиент с нами говорить?
    //     // (современные браузеры могу открыть два подключения сразу)
    //     // Не хочет - закрываем сокет
    //     if (Poll(client_socket) <= 0)
    //     {
    //         CloseSocket(client_socket);
    //         return;
    //     }

    //     // Прочитаем, что клиент нам сказал (блокирующий вызов!!)
    //     int result = recv(client_socket, m_input_buf, sizeof(m_input_buf), 0);
    //     if (result == SOCKET_ERROR)
    //     {
    //         std::cerr << "Error on client receive: " << result << std::endl;
    //         CloseSocket(client_socket);
    //         return;
    //     }
    //     else if (result == 0)
    //     {
    //         std::cerr << "Client closed connection before getting any data!" << std::endl;
    //         CloseSocket(client_socket);
    //         return;
    //     }
    //     m_input_buf[result] = '\0';

    //     // Сюда запишем полный ответ клиенту
    //     std::stringstream response;
    //     // Сюда запишем HTML-страницу с ответом
    //     std::stringstream response_body; // тело ответа
    //     // TODO: хороший сервер должен анализировать заголовки запроса (m_input_buf)
    //     // и на их основе создавать ответ. Но это - плохой сервер )
    //     response_body << "<html>\n<head>"
    //                   << "\t<meta http-equiv=\"Refresh\" content=\"1\" />\n"
    //                   << "\t<title>Temperature Device Server</title>\n"
    //                   << "</head>\n\t<body>\n"
    //                   << "\t\t<h1>Temperature Device Server</h1>\n"
    //                   << "\t\t<p>Current device data: <b>"
    //                   << (data_str.size() ? data_str : "No data provided!")
    //                   << "</b></p>\n"
    //                   << "<pre>" << m_input_buf << "</pre>"
    //                   << "\t</body>\n</html>";
    //     // Формируем весь ответ вместе с заголовками
    //     response << "HTTP/1.0 200 OK\r\n"
    //              << "Version: HTTP/1.1\r\n"
    //              << "Content-Type: text/html; charset=utf-8\r\n"
    //              << "Content-Length: " << response_body.str().length()
    //              << "\r\n\r\n"
    //              << response_body.str();
    //     // Отправляем ответ клиенту
    //     result = send(client_socket, response.str().c_str(), (int)response.str().length(), 0);
    //     if (result == SOCKET_ERROR)
    //     {
    //         // произошла ошибка при отправке данных
    //         std::cerr << "Failed to send responce to client: " << ErrorCode() << std::endl;
    //     }
    //     // Закрываем соединение к клиентом
    //     CloseSocket(client_socket);
    //     std::cout << "Answered to client!" << std::endl;
    // }

    void ProcessClient(const std::string data_str)
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

        // Прочитаем, что клиент нам сказал (блокирующий вызов!!)
        int result = recv(client_socket, m_input_buf, sizeof(m_input_buf), 0);
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
        m_input_buf[result] = '\0';

        // Сюда запишем полный ответ клиенту
        std::stringstream response;
        // Сюда запишем HTML-страницу с ответом
        std::stringstream response_body; // тело ответа

        response_body << "<html>\n<head>"
                      << "\t<meta content=\"1\" />\n"
                      << "\t<title>Temperature Device Server</title>\n"
                      << "</head>\n\t<body>\n"
                      << "\t\t<h1>Temperature Device Server</h1>\n"
                      << "\t\t<p>Current device data: <b>"
                      << (data_str.size() ? data_str : "No data provided!")
                      << "</b></p>\n"
                      << "<pre>" << m_input_buf << "</pre>"
                      << "\t</body>\n</html>";

        // response_body << "<title>Test C++ HTTP Server</title>\n"
        //         << "<h1>Test page</h1>\n"
        //         << "<p>This is body of the test page...</p>\n"
        //         << "<h2>Request headers</h2>\n"
        //         << "<pre>" << m_input_buf << "</pre>\n"
        //         << "<em><small>Test C++ Http Server</small></em>\n";

        // Формируем весь ответ вместе с заголовками
        response << "HTTP/1.0 200 OK\r\n"
                 << "Version: HTTP/1.1\r\n"
                 << "Content-Type: text/html; charset=utf-8\r\n"
                 << "Content-Length: " << response_body.str().length()
                 << "\r\n\r\n"
                 << response_body.str();
        // Отправляем ответ клиенту
        result = send(client_socket, response.str().c_str(), (int)response.str().length(), 0);
        if (result == SOCKET_ERROR)
        {
            // произошла ошибка при отправке данных
            std::cerr << "Failed to send responce to client: " << ErrorCode() << std::endl;
        }
        // Закрываем соединение к клиентом
        CloseSocket(client_socket);
        std::cout << "Answered to client!" << std::endl;
    }

private:
    char m_input_buf[1024];
};