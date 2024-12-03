#ifndef CLIENT_H
#define CLIENT_H

#include <QString>
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QPointer>

class Client : public QObject
{
    Q_OBJECT

private:
    QPointer<QNetworkAccessManager> network_manager;

private slots:
    void managerFinished(QPointer<QNetworkReply> reply);

public:
    Client();

    void get(const QUrl& url) const;


signals:
    void recievedData(QString str);
};

#endif // CLIENT_H
