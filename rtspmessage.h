#ifndef RTSPMESSAGE_H
#define RTSPMESSAGE_H

#include <QMap>
#include <QObject>

class RtspMessage : public QObject
{
    Q_OBJECT
public:
    explicit RtspMessage(QObject *parent = 0);

    void parse(const QByteArray &buffer);
    QByteArray data();

    QString header(const QString &key);
    void insert(const QString &key, const QString &value);

private:
    QMap<QString, QString> m_headers;

};

#endif // RTSPMESSAGE_H
