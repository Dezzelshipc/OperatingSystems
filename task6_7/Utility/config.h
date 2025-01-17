#ifndef CONFIG_H
#define CONFIG_H

#include <QString>

struct Config
{
    Config() = delete;

    struct Default {
        Default() = delete;

        static inline const bool closable = true;

        static inline const QString host_ip = "127.0.0.1";
        static inline const qint16 host_port = 8080;
    };
    static inline bool closable = Default::closable;

    static inline QString host_ip = Default::host_ip;
    static inline qint16 host_port = Default::host_port;
};

#endif // CONFIG_H
