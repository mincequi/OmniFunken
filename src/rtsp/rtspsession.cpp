#include "rtspsession.h"
#include "util.h"

#include <openssl/pem.h>
#include <openssl/rsa.h>

#include <QtEndian>
#include <QTcpSocket>

static char airportRsaPrivateKey[] = "-----BEGIN RSA PRIVATE KEY-----\n"
"MIIEpQIBAAKCAQEA59dE8qLieItsH1WgjrcFRKj6eUWqi+bGLOX1HL3U3GhC/j0Qg90u3sG/1CUt\n"
"wC5vOYvfDmFI6oSFXi5ELabWJmT2dKHzBJKa3k9ok+8t9ucRqMd6DZHJ2YCCLlDRKSKv6kDqnw4U\n"
"wPdpOMXziC/AMj3Z/lUVX1G7WSHCAWKf1zNS1eLvqr+boEjXuBOitnZ/bDzPHrTOZz0Dew0uowxf\n"
"/+sG+NCK3eQJVxqcaJ/vEHKIVd2M+5qL71yJQ+87X6oV3eaYvt3zWZYD6z5vYTcrtij2VZ9Zmni/\n"
"UAaHqn9JdsBWLUEpVviYnhimNVvYFZeCXg/IdTQ+x4IRdiXNv5hEewIDAQABAoIBAQDl8Axy9XfW\n"
"BLmkzkEiqoSwF0PsmVrPzH9KsnwLGH+QZlvjWd8SWYGN7u1507HvhF5N3drJoVU3O14nDY4TFQAa\n"
"LlJ9VM35AApXaLyY1ERrN7u9ALKd2LUwYhM7Km539O4yUFYikE2nIPscEsA5ltpxOgUGCY7b7ez5\n"
"NtD6nL1ZKauw7aNXmVAvmJTcuPxWmoktF3gDJKK2wxZuNGcJE0uFQEG4Z3BrWP7yoNuSK3dii2jm\n"
"lpPHr0O/KnPQtzI3eguhe0TwUem/eYSdyzMyVx/YpwkzwtYL3sR5k0o9rKQLtvLzfAqdBxBurciz\n"
"aaA/L0HIgAmOit1GJA2saMxTVPNhAoGBAPfgv1oeZxgxmotiCcMXFEQEWflzhWYTsXrhUIuz5jFu\n"
"a39GLS99ZEErhLdrwj8rDDViRVJ5skOp9zFvlYAHs0xh92ji1E7V/ysnKBfsMrPkk5KSKPrnjndM\n"
"oPdevWnVkgJ5jxFuNgxkOLMuG9i53B4yMvDTCRiIPMQ++N2iLDaRAoGBAO9v//mU8eVkQaoANf0Z\n"
"oMjW8CN4xwWA2cSEIHkd9AfFkftuv8oyLDCG3ZAf0vrhrrtkrfa7ef+AUb69DNggq4mHQAYBp7L+\n"
"k5DKzJrKuO0r+R0YbY9pZD1+/g9dVt91d6LQNepUE/yY2PP5CNoFmjedpLHMOPFdVgqDzDFxU8hL\n"
"AoGBANDrr7xAJbqBjHVwIzQ4To9pb4BNeqDndk5Qe7fT3+/H1njGaC0/rXE0Qb7q5ySgnsCb3DvA\n"
"cJyRM9SJ7OKlGt0FMSdJD5KG0XPIpAVNwgpXXH5MDJg09KHeh0kXo+QA6viFBi21y340NonnEfdf\n"
"54PX4ZGS/Xac1UK+pLkBB+zRAoGAf0AY3H3qKS2lMEI4bzEFoHeK3G895pDaK3TFBVmD7fV0Zhov\n"
"17fegFPMwOII8MisYm9ZfT2Z0s5Ro3s5rkt+nvLAdfC/PYPKzTLalpGSwomSNYJcB9HNMlmhkGzc\n"
"1JnLYT4iyUyx6pcZBmCd8bD0iwY/FzcgNDaUmbX9+XDvRA0CgYEAkE7pIPlE71qvfJQgoA9em0gI\n"
"LAuE4Pu13aKiJnfft7hIjbK+5kyb3TysZvoyDnb3HOKvInK7vXbKuU4ISgxB2bB3HcYzQMGsz1qJ\n"
"2gG0N5hvJpzwwhbhXqFKA4zaaSrw622wDniAK5MlIE0tIAKKP4yxNGjoD2QYjhBGuhvkWKY=\n"
"-----END RSA PRIVATE KEY-----";

RtspSession::RtspSession(int socketDescriptor, QObject *parent)
    : QThread(parent),
      m_socketDescriptor(socketDescriptor)
{
    // Store mac address
    QString macAddress = Util::getMacAddress();
    QStringList stringList = macAddress.split(":");
    for (int i = 0; i < stringList.size(); ++i) {
        bool ok = false;
        m_macAddress[i] = stringList.at(i).toInt(&ok, 16);
    }
}

void RtspSession::run()
{
    // Create TCP socket
    QTcpSocket tcpSocket;
    if (!tcpSocket.setSocketDescriptor(m_socketDescriptor)) {
        qCritical()<<Q_FUNC_INFO<<tcpSocket.errorString();
        return;
    }

    connect(&tcpSocket, &QTcpSocket::readyRead, this, &RtspSession::onRequest);
    connect(&tcpSocket, &QTcpSocket::disconnected, this, &RtspSession::quit);

    // Create RtpReceiver
    //RtpReceiver rtpReceiver;

    exec();
}

void RtspSession::onRequest()
{
    QTcpSocket *tcpSocket = qobject_cast<QTcpSocket*>(sender());
    if (!tcpSocket) {
        qFatal("onRequest: no valid sender");
        return;
    }

    QByteArray buffer = tcpSocket->readAll();

    RtspMessage request, response;
    request.parse(buffer);

    handleAppleChallenge(request, &response, qToBigEndian(tcpSocket->localAddress().toIPv4Address()));
    response.insert("CSeq", request.header("CSeq"));

    if (buffer.startsWith("OPTIONS"))
        handleOptions(request, &response);
    else if (buffer.startsWith("ANNOUNCE"))
        handleAnnounce(request, &response);
    else if (buffer.startsWith("SETUP"))
        handleSetup(request, &response);
    else if (buffer.startsWith("RECORD"))
        handleRecord(request, &response);
    else if (buffer.startsWith("FLUSH"))
        handleFlush(request, &response);
    else if (buffer.startsWith("TEARDOWN"))
        handleTeardown(request, &response);
    else if (buffer.startsWith("SET_PARAMETER"))
        handleSetParameter(request, &response);
    else
        qWarning("RtspWorker::onRequest: unknown RtspRequest: \n%s", buffer.constData());

    if (response.valid()) {
        tcpSocket->write(response.data());
    }
}

void RtspSession::handleOptions(const RtspMessage &request, RtspMessage *response)
{
    Q_UNUSED(request);
    //qDebug()<<Q_FUNC_INFO;

    response->insert("Public", "ANNOUNCE, SETUP, RECORD, PAUSE, FLUSH, TEARDOWN, OPTIONS, SET_PARAMETER");
}

void RtspSession::handleAnnounce(const RtspMessage &request, RtspMessage *response)
{
    Q_UNUSED(response);
    qDebug()<<Q_FUNC_INFO;

    bool ok;
    m_dacpId = request.header("dacp-id").toULong(&ok, 16);

    RtspMessage::Announcement announcement;

    QRegExp rx("a=fmtp:([\\S ]+)"); // match printable characters and space
    rx.indexIn(request.body());
    announcement.fmtp = rx.cap(1).toLatin1();

    rx.setPattern("a=rsaaeskey:(\\S*)\\r\\n");
    rx.indexIn(request.body());
    QByteArray buffer = QByteArray::fromBase64(rx.cap(1).toLatin1());

    // init RSA
    BIO *bio = BIO_new_mem_buf(airportRsaPrivateKey, -1);
    RSA *rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
    //qDebug("RSA Key: %d\n", RSA_check_key(rsa));

    // need memory for signature
    announcement.rsaAesKey.fill(0, RSA_size(rsa));
    int size = RSA_private_decrypt(buffer.size(),
                                   reinterpret_cast<const unsigned char*>(buffer.data()),
                                   reinterpret_cast<unsigned char*>(announcement.rsaAesKey.data()),
                                   rsa, RSA_PKCS1_OAEP_PADDING);
    RSA_free(rsa);
    announcement.rsaAesKey.resize(size);

    rx.setPattern("a=aesiv:(\\S*)\\r\\n");
    rx.indexIn(request.body());
    announcement.aesIv = QByteArray::fromBase64(rx.cap(1).toLatin1());

    rx.setPattern("o=\\S+ \\d+ 0 IN IP4 (\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3})\\r\\n");
    rx.indexIn(request.body());
    announcement.senderAddress = rx.cap(1);

    if (announcement.fmtp.isEmpty() ||
        announcement.rsaAesKey.isEmpty() ||
        announcement.aesIv.isEmpty() ||
        announcement.senderAddress.isNull()) {
        qFatal("RtspWorker::handleAnnounce: obtaining announcement failed!");
        return;
    }

    emit announce(announcement);
}

void RtspSession::handleSetup(const RtspMessage &request, RtspMessage *response)
{
    qDebug()<<Q_FUNC_INFO;

    quint16 senderControlPort = 0;
    quint16 senderTimingPort = 0;
    QString str(request.header("Transport"));
    QStringList transportList = str.split(";");
    foreach(const QString &item, transportList) {
        if (item.contains("control_port")) {
            QRegExp rx("control_port=(\\d+)");
            rx.indexIn(item);
            senderControlPort = rx.cap(1).toUInt();
        } else if (item.contains("timing_port")) {
            QRegExp rx("timing_port=(\\d+)");
            rx.indexIn(item);
            senderTimingPort = rx.cap(1).toUInt();
        }
    }

    if (senderControlPort) {
        emit senderSocketAvailable(airtunes::RetransmitRequest, senderControlPort);
        emit senderSocketAvailable(airtunes::Sync, senderControlPort);
    }
    if (senderTimingPort) {
        emit senderSocketAvailable(airtunes::TimingRequest, senderTimingPort);
    }

    quint16 receiverServerPort  = 0;
    quint16 receiverControlPort = 0;
    quint16 receiverTimingPort  = 0;

    emit receiverSocketRequired(airtunes::AudioData, &receiverServerPort);
    emit receiverSocketRequired(airtunes::RetransmitResponse, &receiverControlPort);
    emit receiverSocketRequired(airtunes::TimingResponse, &receiverTimingPort);

    QByteArray data;
    QTextStream os(&data);
    os << "RTP/AVP/UDP;unicast;mode=record;server_port=" << receiverServerPort;
    os << ";control_port=" << receiverControlPort;
    os << ";timing_port=" << receiverTimingPort;
    os.flush();

    response->insert("Session", "1");
    response->insert("Transport", data);
}

void RtspSession::handleRecord(const RtspMessage &request, RtspMessage *response)
{
    Q_UNUSED(response);

    qint32 seq = -1;
    QString str(request.header("RTP-Info"));
    QStringList rtpInfoList = str.split(";");
    foreach(const QString &item, rtpInfoList) {
        if (item.contains("seq")) {
            QRegExp rx("seq=(\\d+)");
            rx.indexIn(item);
            seq = rx.cap(1).toUInt();
        }
    }

    qDebug()<<Q_FUNC_INFO<< seq;
    if (seq != -1) {
        emit record(seq);
    }
}

void RtspSession::handleFlush(const RtspMessage &request, RtspMessage *response)
{
    Q_UNUSED(response);

    quint16 seq = 0;
    QString str(request.header("RTP-Info"));
    QStringList rtpInfoList = str.split(";");
    foreach(const QString &item, rtpInfoList) {
        if (item.contains("seq")) {
            QRegExp rx("seq=(\\d+)");
            rx.indexIn(item);
            seq = rx.cap(1).toUInt();
        }
    }

    qDebug()<<Q_FUNC_INFO<< seq;
    emit flush(seq);
}

void RtspSession::handleTeardown(const RtspMessage &request, RtspMessage *response)
{
    Q_UNUSED(response);
    qDebug()<<Q_FUNC_INFO;

    // only teardown if it is announced client
    bool ok;
    if (m_dacpId == request.header("dacp-id").toULong(&ok, 16) && ok) {
        emit teardown();
    }
}

void RtspSession::handleSetParameter(const RtspMessage &request, RtspMessage *response)
{
    Q_UNUSED(response);

    bool ok = false;
    float db = request.header("volume").toFloat(&ok);
    if (ok) {
        qDebug()<<Q_FUNC_INFO<< "volume: " << db;
        emit volume(db);
    }
}

void RtspSession::handleAppleChallenge(const RtspMessage &request, RtspMessage *response, quint32 localAddress)
{
    // from https://github.com/joelgibson/go-airplay
    //
    // Allocate a buffer.
    // Write in the decoded Apple-Challenge bytes.
    // Write in the 16-byte IPv6 or 4-byte IPv4 address (network byte order).
    // Write in the 6-byte Hardware address of the network interface (See note).
    // If the buffer has less than 32 bytes written, pad with 0's up to 32 bytes.
    // Encrypt the buffer using the RSA private key extracted in shairport.
    // Base64 encode the ciphertext, trim trailing '=' signs, and send back

    // Obtain apple challenge header
    QByteArray appleChallenge = request.header("Apple-Challenge");
    if (appleChallenge.isEmpty()) {
        return;
    }

    // Write in the decoded Apple-Challenge bytes.
    QByteArray buffer = QByteArray::fromBase64(appleChallenge);
    if (buffer.size() > 16) {
        qCritical("Apple-Challenge has illegal size: %d", buffer.size());
        return;
    }

    // Write in the 16-byte IPv6 or 4-byte IPv4 address (network byte order).
    buffer.append(reinterpret_cast<const char*>(&localAddress), sizeof(localAddress));

    // Write in the 6-byte Hardware address of the network interface (See note).
    //quint8 hwAddress[] = { 1, 2, 3, 4, 5, 6 };
    buffer.append(reinterpret_cast<const char*>(&m_macAddress), sizeof(m_macAddress));

    // If the buffer has less than 32 bytes written, pad with 0's up to 32 bytes.
    while (buffer.size() < 32) {
        buffer.append('\0');
    }

    // Encrypt the buffer using the RSA private key extracted in shairport.
    // https://www.openssl.org/docs/crypto/RSA_private_encrypt.html
    //
    // init RSA
    BIO *bio = BIO_new_mem_buf(airportRsaPrivateKey, -1);
    RSA *rsa = PEM_read_bio_RSAPrivateKey(bio, NULL, NULL, NULL);
    BIO_free(bio);
    //qDebug("RSA Key: %d\n", RSA_check_key(rsa));

    // need memory for signature
    QByteArray to(RSA_size(rsa), 0);
    RSA_private_encrypt(buffer.size(),
                        reinterpret_cast<const unsigned char*>(buffer.data()),
                        reinterpret_cast<unsigned char*>(to.data()),
                        rsa, RSA_PKCS1_PADDING);
    RSA_free(rsa);

    // Base64 encode the ciphertext
    QByteArray appleResponse = to.toBase64();

    // trim trailing '=' signs
    int newSize = appleResponse.indexOf('=');
    if (newSize > 0 && newSize < appleResponse.size())
        appleResponse.resize(newSize);

    // write response
    response->insert("Apple-Response", appleResponse);
}
