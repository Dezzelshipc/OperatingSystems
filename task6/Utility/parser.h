#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QtTypes>
#include <QDateTime>
#include <QList>

class Parser
{
public:
    Parser() = delete;

    struct DateTemp
    {
        QDateTime datetime;
        double temp;
    };

    static QList<DateTemp> ParseFromString(const QString& str_recv);

    static QString GetFromList(const QList<DateTemp>& list);
};

#endif // PARSER_H
