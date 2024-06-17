#ifndef INSYNCDOLPHINPLUGINHELPER_H
#define INSYNCDOLPHINPLUGINHELPER_H

#include "insyncdolphinpluginhelper_export.h"

#include <QObject>
#include <QLocalSocket>

class INSYNCDOLPHINPLUGINHELPER_EXPORT InsyncDolphinPluginHelper : public QObject
{
    Q_OBJECT

private:
    QString controlSocketPath;

public:
    enum SendCommandMode
    {
        WaitForReply,
        SendCommandOnly
    };

    enum SendCommandTimeout
    {
        ShortTimeout,
        LongTimeout
    };

    bool connectWithInsync(const QPointer<QLocalSocket> &socket, SendCommandTimeout timeout = ShortTimeout) const;
    QVariant sendCommand(const QJsonObject &command,
                         const QPointer<QLocalSocket> &socket,
                         SendCommandMode mode = SendCommandOnly,
                         SendCommandTimeout timeout = ShortTimeout) const;
};

#endif // INSYNCDOLPHINPLUGINHELPER_H
