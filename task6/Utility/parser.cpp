#include "parser.h"

QList<Parser::DateTemp> Parser::ParseFromString(const QString& str_recv)
{
    auto split = str_recv.split("\n");

    QList<DateTemp> list;
    for (const auto& str : split)
    {
        auto d_t = str.trimmed().split(" ");
        auto date = QDateTime();
        date.setSecsSinceEpoch(d_t[0].toULongLong());
        list.append({date, d_t[1].toDouble()});
    }

    return list;
}

QString Parser::GetFromList(const QList<Parser::DateTemp>& list)
{
    QString string;
    for (const auto& d_t : list)
    {
        string += QString("%1 %2\n").arg(d_t.datetime.toString()).arg(d_t.temp);
    }
    return string;
}
