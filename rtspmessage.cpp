#include "rtspmessage.h"

#include <QRegExp>
#include <QTextStream>

RtspMessage::RtspMessage(QObject *parent) :
    QObject(parent)
{
}

void RtspMessage::parse(const QByteArray &buffer)
{
    //QRegExp rx("([A-Za-z-]+)\\:\\s([.*])[\\r\\n]");
    QRegExp rx("([A-Za-z-]+)\\:\\s(\\S*)\\r\\n");
    QString str(buffer);
    int pos = 0;
    while ((pos = rx.indexIn(str, pos)) != -1) {
        m_headers.insert(rx.cap(1).toLower().toLatin1(), rx.cap(2).toLatin1());
        pos += rx.matchedLength();
    }
}

QByteArray RtspMessage::header(const QByteArray &key) const
{
    return m_headers.value(key.toLower(), "");
}

void RtspMessage::insert(const QByteArray &key, const QByteArray &value)
{
    m_headers.insert(key, value);
}

QByteArray RtspMessage::data()
{
    QByteArray data;
    QTextStream os(&data);

    os << "RTSP/1.0 200 OK\r\n";
    QMap<QByteArray, QByteArray>::iterator it;
    for (it = m_headers.begin(); it != m_headers.end(); ++it)
        os << it.key() << ": " << it.value() << "\r\n";
    os << "\r\n";   // import!!!
    os.flush();
    return data;
}
