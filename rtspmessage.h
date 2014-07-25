#ifndef RTSPMESSAGE_H
#define RTSPMESSAGE_H

#include <QHostAddress>
#include <QMap>
#include <QObject>

class RtspMessage : public QObject
{
    Q_OBJECT
public:
    struct Announcement {
        QByteArray fmtp;
        QByteArray rsaAesKey;
        QByteArray aesIv;
        QHostAddress senderAddress;
    };

    explicit RtspMessage(QObject *parent = 0);

    void parse(const QByteArray &buffer);
    QByteArray data();

    QByteArray header(const QByteArray &key) const;
    const QString & body() const;
    void insert(const QByteArray &key, const QByteArray &value);

    void setValid(bool valid);
    bool valid() const;

private:
    QMap<QByteArray, QByteArray> m_headers;
    QString m_body;
    bool m_valid;
};

#endif // RTSPMESSAGE_H
