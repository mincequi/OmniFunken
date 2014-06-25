#include "rtspserver.h"
#include "rtspmessage.h"

#include <QTcpSocket>


RtspServer::RtspServer(QObject *parent)
    : QObject(parent)
{
    m_tcpServer = new QTcpServer(this);
}

bool RtspServer::listen(const QHostAddress &address, quint16 port)
{
    connect(m_tcpServer, SIGNAL(newConnection()), this, SLOT(onNewConnection()));
    return m_tcpServer->listen(address, port);
}

void RtspServer::onNewConnection()
{
    QTcpSocket *tcpSocket = m_tcpServer->nextPendingConnection();
    connect(tcpSocket, SIGNAL(readyRead()),
            this, SLOT(onRequest()));

    connect(tcpSocket, SIGNAL(disconnected()),
            tcpSocket, SLOT(deleteLater()));
}

void RtspServer::onRequest()
{
    QTcpSocket *tcpSocket = qobject_cast<QTcpSocket *>(sender());
    if (!tcpSocket)
    {
        qWarning("onRequest: no valid sender");
        return;
    }

    QByteArray buffer = tcpSocket->readAll();

    RtspMessage request, response;
    request.parse(buffer);

    response.insert("CSeq", request.header("CSeq"));
    response.insert("Audio-Jack-Status", "connected; type=analog");

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
    else
        qWarning("unknown RtspRequest: \n%s", buffer.constData());

    tcpSocket->write(response.data());
}

void RtspServer::handleOptions(const RtspMessage &request, RtspMessage *response)
{
    response->insert("Public", "ANNOUNCE, SETUP, RECORD, PAUSE, FLUSH, TEARDOWN, OPTIONS, SET_PARAMETER");
}

void RtspServer::handleAnnounce(const RtspMessage &request, RtspMessage *response)
{
}

void RtspServer::handleSetup(const RtspMessage &request, RtspMessage *response)
{
}

void RtspServer::handleRecord(const RtspMessage &request, RtspMessage *response)
{
}

void RtspServer::handleFlush(const RtspMessage &request, RtspMessage *response)
{
}

void RtspServer::handleTeardown(const RtspMessage &request, RtspMessage *response)
{
}

void RtspServer::handleAppleChallenge(const RtspMessage &request, RtspMessage *response)
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

    // Write in the decoded Apple-Challenge bytes.
    QByteArray appleChallenge = QByteArray:::fromBase64(request.header("Apple-Challenge").toAscii());
    if (appleChallenge.isEmpty() || (appleChallenge.size() > 16))
    {
        qWarning("Apple-Challenge has illegal size: %d", appleChallenge.size());
        return;
    }

    // Write in the 16-byte IPv6 or 4-byte IPv4 address (network byte order).
    SOCKADDR fdsa;
    socklen_t sa_len = sizeof(fdsa);
    getsockname(fd, (struct sockaddr*)&fdsa, &sa_len);

    int chall_len;
    uint8_t *chall = base64_dec(hdr, &chall_len);
    uint8_t buf[48], *bp = buf;
    int i;
    memset(buf, 0, sizeof(buf));

    if (chall_len > 16) {
        qWarning("oversized Apple-Challenge!");
        free(chall);
        return;
    }
    memcpy(bp, chall, chall_len);
    free(chall);
    bp += chall_len;

    {
        struct sockaddr_in *sa = (struct sockaddr_in*)(&fdsa);
        memcpy(bp, &sa->sin_addr.s_addr, 4);
        bp += 4;
    }

    for (i=0; i<6; i++)
        *bp++ = config.hw_addr[i];

    int buflen, resplen;
    buflen = bp-buf;
    if (buflen < 0x20)
        buflen = 0x20;

    uint8_t *challresp = rsa_apply(buf, buflen, &resplen, RSA_MODE_AUTH);
    char *encoded = base64_enc(challresp, resplen);

    // strip the padding.
    char *padding = strchr(encoded, '=');
    if (padding)
        *padding = 0;

    msg_add_header(resp, "Apple-Response", encoded);
    free(challresp);
    free(encoded);

}
