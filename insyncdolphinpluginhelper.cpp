#include <insyncdolphinpluginhelper.h>

#include <unistd.h>

#include <QDir>
#include <QPointer>
#include <QLocalSocket>
#include <QStringBuilder>
#include <QJsonDocument>
#include <QJsonObject>

bool InsyncDolphinPluginHelper::connectWithInsync(const QPointer<QLocalSocket> &socket,
                                                  SendCommandTimeout timeout) const
{
    QString socketFileName = QLatin1String("insync") % QString::number(getuid()) % QLatin1String(".sock");
    QString insyncSocketPath = QDir::tempPath() % QDir::separator() % socketFileName;
    QString controlSocketPath = QDir::toNativeSeparators(insyncSocketPath);

    if (socket->state() != QLocalSocket::ConnectedState)
    {
        socket->connectToServer(controlSocketPath);

        if (!socket->waitForConnected(timeout == ShortTimeout ? 100 : 500))
        {
            socket->abort();
            return false;
        }
    }

    return true;
}

QVariant InsyncDolphinPluginHelper::sendCommand(const QJsonObject &command,
                                                const QPointer<QLocalSocket> &socket,
                                                SendCommandMode mode,
                                                SendCommandTimeout timeout) const
{
    if (!connectWithInsync(socket, timeout))
    {
        return QVariant();
    }

    const QJsonDocument *request = new QJsonDocument(command);

    socket->readAll();
    socket->write(request->toJson());
    socket->flush();

    if (mode == SendCommandOnly)
    {
        return QVariant();
    }

    QString reply;
    while (socket->waitForReadyRead(timeout == ShortTimeout ? 100 : 500))
    {
        reply.append(QString::fromUtf8(socket->readAll()));
        break;
    }

    QJsonDocument jsonReply = QJsonDocument::fromJson(reply.toUtf8());
    if (jsonReply.toVariant().isNull())
    {
        // Response wasn't JSON serializable

        // Trim quotes
        if (reply.startsWith("\""))
        {
            reply.remove(0, 1);
        }
        if (reply.endsWith("\""))
        {
            reply.remove(reply.size() - 1, 1);
        }

        return QVariant(reply.toUtf8());
    }
    return jsonReply.toVariant();
}

#include "insyncdolphinpluginhelper.moc"
