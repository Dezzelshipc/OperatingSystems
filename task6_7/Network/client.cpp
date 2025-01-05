#include "client.h"

#include <QDebug>

Client::Client() : network_manager(new QNetworkAccessManager)
{
    QObject::connect(network_manager, &QNetworkAccessManager::finished, this, &Client::managerFinished);
}

void Client::managerFinished(QPointer<QNetworkReply> reply)
{
    if (reply->error()) {
        qDebug() << reply->errorString();
        return;
    }

    QString answer = reply->readAll();

    emit recievedData(answer);
}

void Client::get(const QUrl& url) const
{
    auto request = QNetworkRequest();
    request.setUrl(url);
    request.setRawHeader("User-Agent", "TempQT 1.0");
    network_manager->get(request);
}
