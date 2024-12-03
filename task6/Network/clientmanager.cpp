#include "clientmanager.h"
#include "Utility/config.h"

void ClientManager::getLog(QPointer<Client> client, const LOG_TYPE log_type)
{
    QString server = QString("http://%1:%2").arg(Config::host_ip).arg(Config::host_port);
    switch (log_type) {
    case SEC:
        server += "/sec/raw";
        break;
    case HOUR:
        server += "/hour/raw";
        break;
    case DAY:
        server += "/day/raw";
        break;
    }

    qDebug() << server;
    client->get(QUrl(server));
}
