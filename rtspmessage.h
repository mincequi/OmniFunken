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

    QByteArray header(const QByteArray &key) const;
    void insert(const QByteArray &key, const QByteArray &value);

private:
    QMap<QByteArray, QByteArray> m_headers;

};

#endif // RTSPMESSAGE_H
