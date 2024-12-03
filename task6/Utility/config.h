#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

class Config
{
public:
    Config() = delete;

    static inline QString host_ip = "127.0.0.1";
    static inline qint16 host_port = 8080;
};

#endif // CONFIG_H
