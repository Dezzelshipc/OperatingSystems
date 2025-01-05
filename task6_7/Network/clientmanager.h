#ifndef CLIENTMANAGER_H
#define CLIENTMANAGER_H

#include <QString>

#include "client.h"

class ClientManager
{
public:
    ClientManager() = delete;

    enum LOG_TYPE
    {
        SEC, HOUR, DAY
    };

    // Calls get method for client for recieveing specific log
    static void getLog(QPointer<Client> client, const LOG_TYPE log_type);
};

#endif // CLIENTMANAGER_H
